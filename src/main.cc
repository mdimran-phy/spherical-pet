// SPDX-License-Identifier: GPL-3.0-only
// Sequential transport application; detector-response assumptions are documented in
// docs/physics.md.
#include "Config.hh"
#include "DetectorConstruction.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4Version.hh"
#include "G4VisAttributes.hh"
#include <cmath>
#include <cstdlib>
#include <set>
#include <sstream>
#include <stdexcept>
#ifdef PET_WITH_UIVIS
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#endif
#include "G4DecayPhysics.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4HCofThisEvent.hh"
#include "G4LogicalVolume.hh"
#include "G4Navigator.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpticalPhysics.hh"
#include "G4Polyhedron.hh"
#include "G4ProcessManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SipmUiMessenger.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4TransportationManager.hh"
#include "G4UImanager.hh"
#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserTrackingAction.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VProcess.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VTrajectory.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"
#include "hit/G4SipmHit.hh"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <string>

namespace fs = std::filesystem;
struct State {
    fs::path out;
    const Config& config;
    bool started = false;
    std::set<int> savedOpticalTracks;
    std::ofstream events, hits, tracks, opticalTracks;
    int event = 0, optical = 0, scint = 0, cherenkov = 0, killed = 0;
    double edep = 0;
    long totalOptical = 0, totalHits = 0, totalKilled = 0;
    double totalEdep = 0;
    std::map<std::string, long> endings;
    explicit State(const Config& c) : out(c.text("output")), config(c) {
        if (fs::exists(out))
            throw std::runtime_error("Output already exists: " + out.string() +
                                     ". Choose a new --output directory.");
        if (!out.parent_path().empty())
            fs::create_directories(out.parent_path());
        if (!fs::create_directory(out))
            throw std::runtime_error("Cannot create output: " + out.string());
    }
};
class Physics : public G4VModularPhysicsList {
  public:
    Physics(const Config& c) {
        SetVerboseLevel(0);
        RegisterPhysics(new G4EmLivermorePhysics(0));
        RegisterPhysics(new G4DecayPhysics(0));
        RegisterPhysics(new G4OpticalPhysics(0));
        defaultCutValue = c.number("range-cut-mm") * mm;
    }
    void SetCuts() override { SetCutsWithDefault(); }
};
class Primary : public G4VUserPrimaryGeneratorAction {
    G4GeneralParticleSource gps;

  public:
    void GeneratePrimaries(G4Event* e) override { gps.GeneratePrimaryVertex(e); }
};
class RunAction : public G4UserRunAction {
    State& s;

  public:
    explicit RunAction(State& x) : s(x) {}
    void BeginOfRunAction(const G4Run*) override {
        if (s.started)
            throw std::runtime_error(
                "One beamOn is supported per invocation; start a new process for another run.");
        s.started = true;
        for (auto* f : {&s.events, &s.hits, &s.tracks, &s.opticalTracks}) {
            f->exceptions(std::ios::failbit | std::ios::badbit);
            f->precision(12);
        }
        s.opticalTracks.open(s.out / "optical_steps.csv");
        s.opticalTracks << "event,track_id,parent_id,step,x0_mm,y0_mm,z0_mm,x1_mm,y1_mm,z1_mm,t0_"
                           "ns,t1_ns,process\n";
        s.events.open(s.out / "events.csv");
        s.hits.open(s.out / "sipm_hits.csv");
        s.tracks.open(s.out / "gamma_steps.csv");
        s.events << "event,edep_lyso_keV,optical_created,scintillation_created,cherenkov_created,"
                    "sipm_accepted_hits,guard_killed_tracks\n";
        s.hits << "event,sipm_id,track_id,time_ns,energy_eV,x_mm,y_mm,z_mm\n";
        s.tracks
            << "event,track_id,parent_id,x0_mm,y0_mm,z0_mm,x1_mm,y1_mm,z1_mm,edep_keV,process\n";
    }
    void EndOfRunAction(const G4Run* r) override {
        s.events.close();
        s.hits.close();
        s.tracks.close();
        s.opticalTracks.close();
        std::ofstream f(s.out / "summary.json");
        f.exceptions(std::ios::failbit | std::ios::badbit);
        f.precision(12);
        f << "{\n  \"events\": " << r->GetNumberOfEvent()
          << ",\n  \"edep_lyso_keV\": " << s.totalEdep
          << ",\n  \"optical_created\": " << s.totalOptical
          << ",\n  \"sipm_accepted_hits\": " << s.totalHits
          << ",\n  \"guard_killed_tracks\": " << s.totalKilled << ",\n  \"optical_endings\": {";
        bool first = true;
        for (auto& kv : s.endings) {
            if (!first)
                f << ",";
            f << "\n    \"" << kv.first << "\": " << kv.second;
            first = false;
        }
        f << "\n  }\n}\n";
        G4cout << "PET_RESULT events=" << r->GetNumberOfEvent() << " Edep_keV=" << s.totalEdep
               << " optical=" << s.totalOptical << " SiPM_hits=" << s.totalHits
               << " guard_kills=" << s.totalKilled << G4endl;
    }
};
class EventAction : public G4UserEventAction {
    State& s;

  public:
    explicit EventAction(State& x) : s(x) {}
    void BeginOfEventAction(const G4Event* e) override {
        s.event = e->GetEventID();
        s.savedOpticalTracks.clear();
        s.edep = 0;
        s.optical = s.scint = s.cherenkov = s.killed = 0;
    }
    void EndOfEventAction(const G4Event* e) override {
        long n = 0;
        auto* hc = e->GetHCofThisEvent();
        if (hc)
            for (int j = 0; j < hc->GetCapacity(); ++j) {
                auto* col = dynamic_cast<G4SipmHitsCollection*>(hc->GetHC(j));
                if (!col)
                    continue;
                for (size_t i = 0; i < col->GetSize(); ++i) {
                    auto* h = (*col)[i];
                    auto p = h->getWorldPosition();
                    s.hits << s.event << "," << h->getSipmId() << "," << h->getTrackId() << ","
                           << h->getTime() / ns << "," << h->getEKin() / eV << "," << p.x() / mm
                           << "," << p.y() / mm << "," << p.z() / mm << "\n";
                    ++n;
                }
            }
        s.events << s.event << "," << s.edep / keV << "," << s.optical << "," << s.scint << ","
                 << s.cherenkov << "," << n << "," << s.killed << "\n";
        s.totalOptical += s.optical;
        s.totalHits += n;
        s.totalEdep += s.edep / keV;
        s.totalKilled += s.killed;
        if (!s.config.flag("batch")) {
            auto* trajectories = e->GetTrajectoryContainer();
            int opticalPaths = 0;
            if (trajectories)
                for (int i = 0; i < trajectories->entries(); ++i)
                    if ((*trajectories)[i]->GetParticleName() == "opticalphoton")
                        ++opticalPaths;
            G4cout << "PET_DISPLAY_EVENT event=" << s.event
                   << " trajectories=" << (trajectories ? trajectories->entries() : 0)
                   << " optical_paths=" << opticalPaths << " SiPM_hits=" << n << G4endl;
        }
        if (s.config.integer("verbose") > 0)
            G4cout << "PET_EVENT " << s.event << " Edep_keV=" << s.edep / keV
                   << " optical=" << s.optical << " hits=" << n << G4endl;
    }
};
class Tracking : public G4UserTrackingAction {
    State& s;

  public:
    explicit Tracking(State& x) : s(x) {}
    void PreUserTrackingAction(const G4Track* t) override {
        const bool optical = t->GetDefinition() == G4OpticalPhoton::Definition();
        // Native trajectories are independent of CSV recording. Keep all charged
        // and gamma tracks, but only the first N optical tracks of each event.
        const bool draw =
            !s.config.flag("batch") && (!optical || s.optical < s.config.integer("vis-photons"));
        fpTrackingManager->SetStoreTrajectory(draw ? 1 : 0);
        if (!optical)
            return;
        ++s.optical;
        if (s.optical <= s.config.integer("track-photons"))
            s.savedOpticalTracks.insert(t->GetTrackID());
        auto* p = t->GetCreatorProcess();
        if (p && p->GetProcessName() == "Scintillation")
            ++s.scint;
        if (p && p->GetProcessName() == "Cerenkov")
            ++s.cherenkov;
    }
};
class Stepping : public G4UserSteppingAction {
    State& s;
    G4OpBoundaryProcess* boundary = nullptr;

  public:
    explicit Stepping(State& x) : s(x) {}
    void UserSteppingAction(const G4Step* st) override {
        auto* t = st->GetTrack();
        bool opt = t->GetDefinition() == G4OpticalPhoton::Definition();
        if (!opt && st->GetPreStepPoint()->GetMaterial()->GetName() == "LYSO")
            s.edep += st->GetTotalEnergyDeposit();
        if (t->GetDefinition()->GetParticleName() == "gamma") {
            auto a = st->GetPreStepPoint()->GetPosition(),
                 b = st->GetPostStepPoint()->GetPosition();
            auto* pr = st->GetPostStepPoint()->GetProcessDefinedStep();
            s.tracks << s.event << "," << t->GetTrackID() << "," << t->GetParentID() << ","
                     << a.x() / mm << "," << a.y() / mm << "," << a.z() / mm << "," << b.x() / mm
                     << "," << b.y() / mm << "," << b.z() / mm << ","
                     << st->GetTotalEnergyDeposit() / keV << ","
                     << (pr ? pr->GetProcessName() : "none") << "\n";
        }
        if (opt && s.savedOpticalTracks.count(t->GetTrackID())) {
            auto a = st->GetPreStepPoint()->GetPosition(),
                 b = st->GetPostStepPoint()->GetPosition();
            auto* process = st->GetPostStepPoint()->GetProcessDefinedStep();
            s.opticalTracks << s.event << "," << t->GetTrackID() << "," << t->GetParentID() << ","
                            << t->GetCurrentStepNumber() << "," << a.x() / mm << "," << a.y() / mm
                            << "," << a.z() / mm << "," << b.x() / mm << "," << b.y() / mm << ","
                            << b.z() / mm << "," << st->GetPreStepPoint()->GetGlobalTime() / ns
                            << "," << st->GetPostStepPoint()->GetGlobalTime() / ns << ","
                            << (process ? process->GetProcessName() : "none") << "\n";
        }
        if (t->GetCurrentStepNumber() > s.config.integer("max-steps")) {
            t->SetTrackStatus(fStopAndKill);
            ++s.killed;
        }
        if (opt && t->GetTrackStatus() == fStopAndKill) {
            if (!boundary) {
                auto* ps = t->GetDefinition()->GetProcessManager()->GetProcessList();
                for (size_t i = 0; i < ps->size(); ++i)
                    if (auto* b = dynamic_cast<G4OpBoundaryProcess*>((*ps)[i]))
                        boundary = b;
            }
            auto* pr = st->GetPostStepPoint()->GetProcessDefinedStep();
            std::string name = pr ? pr->GetProcessName() : "none";
            auto* lv = st->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume();
            if (lv->GetSensitiveDetector())
                name = "sensor_sensitive_volume";
            else if (st->GetPostStepPoint()->GetStepStatus() == fGeomBoundary && boundary)
                name = "boundary_status_" + std::to_string(boundary->GetStatus());
            else if (st->GetPostStepPoint()->GetStepStatus() == fWorldBoundary)
                name = "world_escape";
            ++s.endings[name];
        }
    }
};
// Export the actual placed geometry (not an independently reconstructed drawing).
void exportGeometry(G4VPhysicalVolume* world, const fs::path& out, const Config& config) {
    // Fine visualization mesh only; tracking solids and physics are unchanged.
    G4Polyhedron::SetNumberOfRotationSteps(config.integer("mesh-steps"));
    std::ofstream inv(out / "geometry_inventory.csv"), mesh(out / "geometry.obj"),
        sensors(out / "sensor_positions.csv");
    for (auto* f : {&inv, &mesh, &sensors}) {
        f->exceptions(std::ios::failbit | std::ios::badbit);
        f->precision(12);
    }
    inv << "depth,name,material,solid,x_mm,y_mm,z_mm,sensitive\n";
    sensors << "sd_name,x_mm,y_mm,z_mm\n";
    long vertex = 1;
    std::function<void(G4VPhysicalVolume*, G4RotationMatrix, G4ThreeVector, int)> visit;
    visit = [&](G4VPhysicalVolume* pv, G4RotationMatrix r, G4ThreeVector t, int depth) {
        auto rr = r * pv->GetObjectRotationValue();
        auto tt = r * pv->GetObjectTranslation() + t;
        auto* lv = pv->GetLogicalVolume();
        inv << depth << "," << pv->GetName() << "," << lv->GetMaterial()->GetName() << ","
            << lv->GetSolid()->GetEntityType() << "," << tt.x() / mm << "," << tt.y() / mm << ","
            << tt.z() / mm << "," << (lv->GetSensitiveDetector() ? 1 : 0) << "\n";
        if (lv->GetSensitiveDetector())
            sensors << lv->GetSensitiveDetector()->GetName() << "," << tt.x() / mm << ","
                    << tt.y() / mm << "," << tt.z() / mm << "\n";
        // Core placed optics and sensor packages only, to keep the mesh readable.
        if (depth == 1) {
            auto* poly = lv->GetSolid()->GetPolyhedron();
            if (!poly)
                throw std::runtime_error("Cannot export geometry mesh for " +
                                         std::string(pv->GetName()));
            if (poly) {
                mesh << "o " << pv->GetName() << "\n";
                for (int f = 1; f <= poly->GetNoFacets(); ++f) {
                    int n = 0;
                    G4Point3D nodes[4];
                    poly->GetFacet(f, n, nodes);
                    for (int j = 0; j < n; ++j) {
                        auto v = rr * G4ThreeVector(nodes[j].x(), nodes[j].y(), nodes[j].z()) + tt;
                        mesh << "v " << v.x() / mm << " " << v.y() / mm << " " << v.z() / mm
                             << "\n";
                    }
                    mesh << "f";
                    for (int j = 0; j < n; ++j)
                        mesh << " " << vertex++;
                    mesh << "\n";
                }
            }
        }
        for (int i = 0; i < lv->GetNoDaughters(); ++i)
            visit(lv->GetDaughter(i), rr, tt, depth + 1);
    };
    visit(world, G4RotationMatrix(), G4ThreeVector(), 0);
}

namespace {
void command(const std::string& text) {
    int result = G4UImanager::GetUIpointer()->ApplyCommand(text);
    if (result != 0)
        throw std::runtime_error("Geant4 command failed (" + std::to_string(result) + "): " + text);
}
std::string real(double value) {
    std::ostringstream out;
    out << std::setprecision(17) << value;
    return out.str();
}
std::vector<std::string> readMacro(const Config& c) {
    std::vector<std::string> result;
    if (c.text("macro").empty())
        return result;
    std::ifstream file(c.text("macro"));
    std::string line;
    while (std::getline(file, line)) {
        auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == '#')
            continue;
        line = line.substr(first);
        auto last = line.find_last_not_of(" \t\r");
        line = line.substr(0, last + 1);
        if (line.rfind("/gps/", 0) != 0)
            throw std::runtime_error("Source macros accept /gps/* commands only: " + line);
        result.push_back(line);
    }
    return result;
}
int auditGeometry(G4VPhysicalVolume* world, const Config& c) {
    int overlaps = 0, checked = 0;
    std::function<void(G4VPhysicalVolume*)> visit = [&](G4VPhysicalVolume* pv) {
        auto* lv = pv->GetLogicalVolume();
        for (int i = 0; i < lv->GetNoDaughters(); ++i) {
            auto* child = lv->GetDaughter(i);
            ++checked;
            if (child->CheckOverlaps(c.integer("overlap-samples"),
                                     c.number("overlap-tolerance-mm") * mm, false, 1))
                ++overlaps;
            visit(child);
        }
    };
    visit(world);
    std::ofstream f(fs::path(c.text("output")) / "geometry_check.json");
    f.exceptions(std::ios::failbit | std::ios::badbit);
    f << "{\"volumes_checked\":" << checked << ",\"volumes_reporting_overlaps\":" << overlaps
      << ",\"samples_per_volume\":" << c.integer("overlap-samples")
      << ",\"tolerance_mm\":" << c.number("overlap-tolerance-mm") << "}\n";
    G4cout << "GEOMETRY_CHECK volumes=" << checked << " overlap_reports=" << overlaps << G4endl;
    return overlaps;
}
void displayColors(bool nativeDisplay) {
    for (auto* lv : *G4LogicalVolumeStore::GetInstance()) {
        const auto& name = lv->GetName();
        if (name.find("Scint_LV") != std::string::npos) {
            auto* a = new G4VisAttributes(G4Colour(.55, .1, .75, .75));
            if (nativeDisplay)
                a->SetForceWireframe(true);
            else
                a->SetForceSolid(true);
            lv->SetVisAttributes(a);
        } else if (name.find("chapeauPoofMinusHomeOrb") != std::string::npos) {
            auto* a = new G4VisAttributes(G4Colour(.8, .95, 1., .2));
            if (nativeDisplay)
                a->SetForceWireframe(true);
            else
                a->SetForceSolid(true);
            lv->SetVisAttributes(a);
        } else if (name == "world_LV" || name.find("Gasket") != std::string::npos ||
                   name.find("homeSphereMinusChapeauCasting") != std::string::npos ||
                   name.find("innermost_hemisphere") != std::string::npos)
            lv->SetVisAttributes(G4VisAttributes::GetInvisible());
    }
}
} // namespace
int main(int argc, char** argv) {
    try {
        Config c;
        c.parse(argc, argv);
        if (c.helpRequested) {
            c.help(std::cout);
            return 0;
        }
        if (c.versionRequested) {
            std::cout << "spherical-pet 0.1.0\n" << G4Version << "\n";
#ifdef PET_WITH_UIVIS
            std::cout << "Native event display: enabled (use -b 0)\n";
#else
            std::cout << "Native event display: disabled\n";
#endif
            return 0;
        }
        if (c.printRequested) {
            c.write(std::cout);
            return 0;
        }
#ifndef PET_WITH_UIVIS
        if (!c.flag("batch"))
            throw std::runtime_error(
                "Interactive display requires a build with -DWITH_GEANT4_UIVIS=ON");
#endif
#if defined(PET_WITH_UIVIS) && defined(__linux__)
        if (!c.flag("batch") && !std::getenv("DISPLAY") && !std::getenv("WAYLAND_DISPLAY"))
            throw std::runtime_error(
                "Native event display needs a graphical display. "
                "For a remote host, reconnect with ssh -Y; use -b 1 for batch.");
#endif
        auto extra = readMacro(c);
        State state(c);
        {
            std::ofstream f(state.out / "resolved.conf");
            f.exceptions(std::ios::failbit | std::ios::badbit);
            c.write(f);
        }
        {
            std::ofstream f(state.out / "spectrum.csv");
            f << "energy_eV,intensity\n";
            for (auto [e, i] : c.spectrum())
                f << std::setprecision(17) << e << "," << i << "\n";
        }
        {
            std::ofstream f(state.out / "build.txt");
            f << "spherical-pet 0.1.0\n"
              << G4Version << "\nCompiler: " << __VERSION__ << "\nSequential Geant4 run manager\n";
        }
        CLHEP::HepRandom::setTheSeed(c.integer("seed"));
        auto rm = std::make_unique<G4RunManager>();
        rm->SetUserInitialization(new DetectorConstruction(c));
        rm->SetUserInitialization(new Physics(c));
        rm->SetUserAction(new Primary);
        rm->SetUserAction(new RunAction(state));
        rm->SetUserAction(new EventAction(state));
        rm->SetUserAction(new Tracking(state));
        rm->SetUserAction(new Stepping(state));
        rm->Initialize();
        displayColors(!c.flag("batch"));
        G4SipmUiMessenger::getInstance(); // Register SiPM commands before applying them.
        command("/run/verbose 0");
        command("/event/verbose 0");
        command("/tracking/verbose 0");
        command("/g4sipm/verbose " + c.text("verbose"));
        command("/g4sipm/filter/pde " + std::string(c.flag("pde-filter") ? "true" : "false"));
        command("/g4sipm/filter/geometry " + std::string(c.flag("cell-filter") ? "true" : "false"));
        // Seed after G4SiPM's engine initialization. Reset after geometry sampling too.
        const std::string seedCommand =
            "/random/setSeeds " + c.text("seed") + " " +
            std::to_string(c.integer("seed") == 2147483646L ? 1 : c.integer("seed") + 1);
        command(seedCommand);
        auto* world = G4TransportationManager::GetTransportationManager()
                          ->GetNavigatorForTracking()
                          ->GetWorldVolume();
        if (c.flag("export-geometry"))
            exportGeometry(world, state.out, c);
        if (c.text("mode") == "geometry" || c.flag("check-overlaps")) {
            if (auditGeometry(world, c))
                throw std::runtime_error("Geometry overlaps detected; simulation was not started. "
                                         "See geometry_check.json.");
        }
        if (c.text("mode") == "geometry" && c.flag("batch"))
            return 0;
        std::vector<std::string> source = {
            "/gps/particle " + c.text("particle"), "/gps/energy " + c.text("energy-kev") + " keV",
            "/gps/pos/type Point",
            "/gps/pos/centre " + c.text("source-x-mm") + " " + c.text("source-y-mm") + " " +
                c.text("source-z-mm") + " mm"};
        if (c.text("angular") == "isotropic")
            source.push_back("/gps/ang/type iso");
        else {
            double theta = c.number("theta") * deg, phi = c.number("phi") * deg;
            source.push_back("/gps/direction " + real(std::sin(theta) * std::cos(phi)) + " " +
                             real(std::sin(theta) * std::sin(phi)) + " " + real(std::cos(theta)));
        }
        source.insert(source.end(), extra.begin(), extra.end());
        {
            std::ofstream f(state.out / "source_commands.mac");
            f.exceptions(std::ios::failbit | std::ios::badbit);
            for (const auto& line : source) {
                command(line);
                f << line << "\n";
            }
        }
#ifdef PET_WITH_UIVIS
        // Qt requires argc/argv storage to outlive the UI session.
        int uiArgc = 1;
        char* uiArgv[] = {argv[0], nullptr};
        std::unique_ptr<G4VisExecutive> vis;
        std::unique_ptr<G4UIExecutive> ui;
        if (!c.flag("batch")) {
            ui = std::make_unique<G4UIExecutive>(uiArgc, uiArgv);
            vis = std::make_unique<G4VisExecutive>("quiet");
            vis->Initialize();
            command("/vis/open OGL 1000x800-0+0");
            if (!vis->GetCurrentViewer())
                throw std::runtime_error(
                    "No OpenGL viewer was created; check the graphical display and X forwarding.");
            command("/vis/viewer/set/autoRefresh false");
            command("/vis/drawVolume");
            command("/vis/viewer/set/viewpointThetaPhi 65 30");
            command("/vis/viewer/set/style surface");
            command("/vis/viewer/set/background white");
            command("/vis/viewer/zoom 1.4");
            command("/vis/scene/add/axes 0 0 0 20 mm");
            command("/vis/modeling/trajectories/create/drawByParticleID");
            command("/vis/modeling/trajectories/drawByParticleID-0/set gamma red");
            command("/vis/modeling/trajectories/drawByParticleID-0/set e- blue");
            command("/vis/modeling/trajectories/drawByParticleID-0/set e+ magenta");
            command("/vis/modeling/trajectories/drawByParticleID-0/set opticalphoton cyan");
            command("/vis/scene/add/trajectories");
            command("/vis/scene/add/hits");
            command("/vis/scene/endOfEventAction accumulate " + c.text("events"));
            command("/vis/viewer/set/autoRefresh true");
            if (ui->IsGUI()) {
                command("/gui/addMenu pet PET");
                command("/gui/addButton pet \"Review events\" \"/vis/reviewKeptEvents\"");
                command("/gui/addButton pet \"Reset angle\" \"/vis/viewer/set/viewpointThetaPhi 65 "
                        "30\"");
            }
            G4cout << "PET_DISPLAY red=gamma blue=electron magenta=positron cyan=optical "
                      "green=SiPM_hit\n"
                   << "Native view keeps the run's events for /vis/reviewKeptEvents. "
                   << "Enter continue between events.\n"
                   << "Drawing up to " << c.integer("vis-photons")
                   << " optical paths per event; this does not limit optical transport." << G4endl;
        }
#endif
        command(seedCommand);
        if (c.text("mode") == "simulation")
            rm->BeamOn(c.integer("events"));
#ifdef PET_WITH_UIVIS
        if (ui) {
            command("/vis/viewer/flush");
            ui->SessionStart();
        }
#endif
        if (state.totalKilled)
            throw std::runtime_error(
                "Track step guard was reached; inspect summary.json before interpreting results.");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "spherical-pet: " << e.what() << "\n";
        return 1;
    }
}
