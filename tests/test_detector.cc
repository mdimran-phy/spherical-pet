// SPDX-License-Identifier: GPL-3.0-only
#include "Config.hh"
#include "DetectorConstruction.hh"
#include "G4BooleanSolid.hh"
#include "G4DigiManager.hh"
#include "G4DisplacedSolid.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSensitiveDetector.hh"
#include "GeometryParameters.hh"
#include "digi/G4SipmDigitizer.hh"
#include "digi/G4SipmVoltageTraceDigitizer.hh"
#include "housing/G4SipmHousing.hh"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

namespace {
void require(bool ok, const std::string& message) {
    if (!ok)
        throw std::runtime_error(message);
}
struct OutputDirectory {
    std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("pet_detector_test_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    OutputDirectory() {
        require(std::filesystem::create_directory(path), "Cannot create test output");
    }
    ~OutputDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};
void collectSolids(const G4VSolid* solid, std::set<const G4VSolid*>& seen) {
    if (!seen.insert(solid).second)
        return;
    if (auto* boolean = dynamic_cast<const G4BooleanSolid*>(solid)) {
        collectSolids(boolean->GetConstituentSolid(0), seen);
        collectSolids(boolean->GetConstituentSolid(1), seen);
    } else if (auto* displaced = dynamic_cast<const G4DisplacedSolid*>(solid)) {
        collectSolids(displaced->GetConstituentMovedSolid(), seen);
    }
}
void collectVolumes(G4LogicalVolume* volume, std::set<G4LogicalVolume*>& seen,
                    std::set<const G4VSolid*>& solids) {
    if (!seen.insert(volume).second)
        return;
    collectSolids(volume->GetSolid(), solids);
    for (size_t i = 0; i < volume->GetNoDaughters(); ++i)
        collectVolumes(volume->GetDaughter(i)->GetLogicalVolume(), seen, solids);
}
bool contains(const G4VPhysicalVolume* volume, const G4ThreeVector& point) {
    const auto local =
        volume->GetObjectRotationValue().inverse() * (point - volume->GetObjectTranslation());
    return volume->GetLogicalVolume()->GetSolid()->Inside(local) == kInside;
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "Expected guide shape");
        OutputDirectory output;
        std::vector<std::string> args = {"test-detector", "--output", output.path.string(),
                                         "--guide-shape", argv[1]};
        std::vector<char*> raw;
        for (auto& arg : args)
            raw.push_back(arg.data());
        Config config;
        config.parse(raw.size(), raw.data());
        // Keep managers alive through detector teardown, matching application shutdown order.
        G4RunManager runManager;
        auto detector = std::make_unique<DetectorConstruction>(config);
        auto* world = detector->Construct();
        std::set<G4LogicalVolume*> volumes;
        std::set<const G4VSolid*> solids;
        collectVolumes(world->GetLogicalVolume(), volumes, solids);
        for (auto* volume : *G4LogicalVolumeStore::GetInstance())
            require(volumes.count(volume), "Unplaced logical volume: " + volume->GetName());
        for (auto* solid : *G4SolidStore::GetInstance())
            require(solids.count(solid), "Unused solid: " + solid->GetName());

        std::map<std::string, G4VPhysicalVolume*> shell;
        int guides = 0, sensors = 0;
        std::vector<G4VSensitiveDetector*> detectors;
        std::vector<G4SipmDigitizer*> digitizers;
        std::vector<G4SipmVoltageTraceDigitizer*> traces;
        for (auto* volume : *G4PhysicalVolumeStore::GetInstance()) {
            const auto& name = volume->GetName();
            if (name == "chapeauPoofMinusHomeOrb_PV")
                ++guides;
            if (name == "hemisphereNorthScint_PV" || name == "hemisphereNorthGasket_PV" ||
                name == "hemisphereSouthScint_PV" || name == "hemisphereSouthGasket_PV")
                shell.emplace(name, volume);
            if (auto* sd = volume->GetLogicalVolume()->GetSensitiveDetector()) {
                ++sensors;
                detectors.push_back(sd);
                const auto id = sd->GetName().substr(std::string("g4sipmSd-").size());
                auto* digitizer = dynamic_cast<G4SipmDigitizer*>(
                    G4DigiManager::GetDMpointer()->FindDigitizerModule("g4sipmDigitizer-" + id));
                auto* trace = dynamic_cast<G4SipmVoltageTraceDigitizer*>(
                    G4DigiManager::GetDMpointer()->FindDigitizerModule(
                        "g4sipmVoltageTraceDigitizer-" + id));
                require(digitizer && trace && digitizer->getSipm() && trace->getSipm(),
                        "Missing sensor digitizers");
                digitizers.push_back(digitizer);
                traces.push_back(trace);
            }
        }
        require(guides == 1, "Exactly one main light guide must be placed");
        require(sensors == GeometryParameters::sensorCount,
                "Incorrect number of sensitive sensors");
        require(std::set<G4VSensitiveDetector*>(detectors.begin(), detectors.end()).size() ==
                    GeometryParameters::sensorCount,
                "Sensitive detectors must be distinct");
        require(shell.size() == 4, "Missing hemisphere or gasket");
        // Probe either side of each seam, away from surfaces: exactly one material
        // must contain the point. These checks supplement the sampled overlap audit.
        const double halfGasket = GeometryParameters::halfGasketThicknessMm * mm;
        for (int sign : {-1, 1}) {
            const std::string side = sign > 0 ? "North" : "South";
            for (double z : {halfGasket / 2, 2 * halfGasket, 10 * mm}) {
                const G4ThreeVector point(30 * mm, 0, sign * z);
                const std::string expected =
                    "hemisphere" + side + (z < halfGasket ? "Gasket_PV" : "Scint_PV");
                for (const auto& [name, volume] : shell)
                    require(contains(volume, point) == (name == expected),
                            "Incorrect hemisphere/gasket partition at " + expected + ": " + name);
            }
        }
        // Exercise upstream rotation replacement before shutdown: setRotation
        // deletes the previous pointer, so no second owner may retain it.
        auto* housing = detector->getSipmHousing();
        require(housing->getRotation(), "Missing sensor placement rotation");
        housing->setRotation(new G4RotationMatrix(*housing->getRotation()));
        detector.reset();
        for (auto* sd : detectors)
            require(!sd->isActive(), "Sensor still active after detector destruction");
        for (auto* digitizer : digitizers)
            require(!digitizer->getSipm(), "Dangling SiPM in digitizer");
        for (auto* trace : traces)
            require(!trace->getSipm(), "Dangling SiPM in trace digitizer");
        std::cout << "PASS: all solids and volumes used, main guide placed, seam partition, 512 "
                     "sensors, safe teardown\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
