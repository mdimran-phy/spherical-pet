/*
 * DetectorConstruction.cc
 *
 * @date Sep 12, 2011
 * @author Tim Niggemann, III Phys. Inst. A, RWTH Aachen University
 * @copyright GNU General Public License v3.0
 */

#include "DetectorConstruction.hh"
#include "G4Element.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4Orb.hh"
#include "G4PhysicalConstants.hh"
#include "G4Sipm.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "GeometryParameters.hh"
#include "GuideShape.hh"
#include "MaterialFactory.hh"
#include "housing/G4SipmHousing.hh"
#include "housing/impl/HamamatsuCeramicHousing.hh"
#include "housing/impl/HamamatsuSmdHousing.hh"
#include "model/G4SipmModelFactory.hh"
#include <G4Box.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <fstream>
#include <iomanip>
#include <set>
#include <stdexcept>
DetectorConstruction::DetectorConstruction(const Config& settings)
    : G4VUserDetectorConstruction(), config(settings) {
    config.validate();
    const std::string sipmModelName = config.text("sipm-model");
    const std::string housingName = "default";
    const std::set<std::string> supported = {"generic",
                                             "HamamatsuS1036211100",
                                             "HamamatsuS1036233100",
                                             "HamamatsuS10985100",
                                             "HamamatsuS12651050",
                                             "HamamatsuS1036233050",
                                             "HamamatsuS12573100C",
                                             "HamamatsuS12573100X"};
    if (!supported.count(sipmModelName))
        throw std::runtime_error("Unavailable SiPM model: " + sipmModelName);
    if (config.integer("verbose") > 0)
        std::cout << "DetectorConstruction::DetectorConstruction: " << sipmModelName << ", "
                  << housingName << std::endl;
    model.reset(createSipmModel(sipmModelName));
    if (!model)
        throw std::runtime_error("SiPM model factory returned no model: " + sipmModelName);
    mHousings.reserve(GeometryParameters::sensorCount);
    for (int index = 0; index < GeometryParameters::sensorCount; ++index) {
        mHousings.push_back(createHousing(housingName, std::make_unique<G4Sipm>(model.get())));
    }
    if (mHousings.empty())
        throw std::runtime_error("Detector construction requires at least one SiPM housing");
    housing = mHousings.front().get();
    const double pitch = config.number("sipm-pitch-mm") * mm;
    const double outletWidth = 2 * GeometryParameters::guideOutletHalfWidthMm * mm;
    if (housing->getDx() > pitch || housing->getDy() > pitch ||
        (GeometryParameters::sensorsPerSide - 1) * pitch + housing->getDx() > outletWidth ||
        (GeometryParameters::sensorsPerSide - 1) * pitch + housing->getDy() > outletWidth)
        throw std::runtime_error(
            "SiPM package does not fit the requested pitch or 18 mm guide outlet");
    auto* epoxy = MaterialFactory::getInstance()->getEpoxy()->GetMaterialPropertiesTable();
    double energies[] = {1.0 * eV, 6.0 * eV};
    double indices[] = {config.number("epoxy-index"), config.number("epoxy-index")};
    epoxy->RemoveProperty("RINDEX");
    epoxy->AddProperty("RINDEX", energies, indices, 2);
    if (config.number("epoxy-absorption-mm") > 0) {
        double lengths[] = {config.number("epoxy-absorption-mm") * mm,
                            config.number("epoxy-absorption-mm") * mm};
        epoxy->AddProperty("ABSLENGTH", energies, lengths, 2);
    }
}
// The run manager deletes this object before its SD manager. Housing destruction
// deactivates each sensor and detaches its digitizers. Model is declared first so
// it outlives all housings; Geant4 stores retain ownership of solids and volumes.
DetectorConstruction::~DetectorConstruction() {
    // Placeable::setRotation deletes its previous matrix, but Placeable's
    // destructor does not. Keep housing rotations under that single ownership
    // contract, and release the final matrix before deleting each housing.
    for (const auto& sensorHousing : mHousings)
        sensorHousing->setRotation(nullptr);
}
G4SipmModel* DetectorConstruction::createSipmModel(std::string name) const {
    if (name == "generic") {
        return G4SipmModelFactory::getInstance()->createGenericSipmModel();
    }
    if (name == "HamamatsuS1036211100") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS1036211100();
    }
    if (name == "HamamatsuS1036233100") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS1036233100();
    }
    if (name == "HamamatsuS10985100") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS10985100();
    }
    if (name == "HamamatsuS12651050") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS12651050();
    }
    if (name == "HamamatsuS1036233050") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS1036233050();
    }
    if (name == "HamamatsuS12573100C") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS12573100C();
    }
    if (name == "HamamatsuS12573100X") {
        return G4SipmModelFactory::getInstance()->createHamamatsuS12573100X();
    }
    throw std::runtime_error("Unavailable SiPM model: " + name);
}
std::unique_ptr<G4SipmHousing>
DetectorConstruction::createHousing(const std::string& name, std::unique_ptr<G4Sipm> sipm) const {
    if (!sipm)
        throw std::runtime_error("Cannot create housing without a SiPM");
    std::unique_ptr<G4SipmHousing> result;
    if (name == "ceramic")
        result = std::make_unique<HamamatsuCeramicHousing>(sipm.get());
    else if (name == "smd")
        result = std::make_unique<HamamatsuSmdHousing>(sipm.get());
    else if (name == "default")
        result = std::make_unique<G4SipmHousing>(sipm.get());
    else
        throw std::runtime_error("Unknown SiPM housing type: " + name);
    sipm.release(); // Ownership transfers only after successful housing construction.
    return result;
}

G4RotationMatrix* DetectorConstruction::keepRotation(const G4RotationMatrix& rotation) {
    rotations.push_back(std::make_unique<G4RotationMatrix>(rotation));
    return rotations.back().get();
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    G4TriangularFacet* chapeauPoof_A2_SquareHexagonFacets[2 * 20] = {};
    G4TriangularFacet* chapeauPoof_A2_SquarePentagonFacets[2 * 12] = {};
    G4TriangularFacet* chapeauPoof_A2_SquareHexagonSkirtFacets[10 * 20] = {};
    G4TriangularFacet* chapeauPoof_A2_SquarePentagonSkirtFacets[9 * 12] = {};
    G4TriangularFacet* chapeauCasting_A2_SquareHexagonSkirtFacets[10 * 20] = {};
    G4TriangularFacet* chapeauCasting_A2_SquarePentagonSkirtFacets[9 * 12] = {};
    G4TriangularFacet* chapeauCasting_A2_SquareHexagonExtensionFacets[10 * 20] = {};
    G4TriangularFacet* chapeauCasting_A2_SquarePentagonExtensionFacets[10 * 12] = {};

    G4ThreeVector worldDimension = G4ThreeVector(GeometryParameters::worldHalfLengthMm * mm,
                                                 GeometryParameters::worldHalfLengthMm * mm,
                                                 GeometryParameters::worldHalfLengthMm * mm);
    G4Box* world = new G4Box("world", worldDimension.x(), worldDimension.y(), worldDimension.z());
    G4LogicalVolume* world_LV = new G4LogicalVolume(world, MaterialFactory::getInstance()->getAir(),
                                                    "world_LV", NULL, NULL, NULL);
    world_LV->SetVisAttributes(new G4VisAttributes(G4Color(255, 255, 255, 0.1)));
    G4VPhysicalVolume* world_PV =
        new G4PVPlacement(NULL, G4ThreeVector(), world_LV, "world_PV", NULL, false, 0);
    auto* fLu = new G4Element("Lutetium", "Lu", 71., 174.97 * g / mole);
    auto* fY = new G4Element("Yttrium", "Y", 39., 88.91 * g / mole);
    auto* fSi = new G4Element("Silicon", "Si", 14., 28.09 * g / mole);
    auto* fO = new G4Element("O", "O", 8., 16.00 * g / mole);
    auto* fLYSO = new G4Material("LYSO", 7.36 * g / cm3, 4);
    G4double fractionLu_LYSO = 0.714467891;
    fLYSO->AddElement(fLu, fractionLu_LYSO);
    G4double fractionY_LYSO = 0.04033805;
    fLYSO->AddElement(fY, fractionY_LYSO);
    G4double fractionSi_LYSO = 0.063714272;
    fLYSO->AddElement(fSi, fractionSi_LYSO);
    G4double fractionO_LYSO = 0.181479788;
    fLYSO->AddElement(fO, fractionO_LYSO);
    auto spectrum = config.spectrum();
    std::vector<G4double> energies, intensities, indices, lengths;
    for (const auto& point : spectrum) {
        energies.push_back(point.first * eV);
        intensities.push_back(point.second);
        indices.push_back(config.number("lyso-index"));
        lengths.push_back(config.number("lyso-absorption-mm") * mm);
    }
    auto* fLYSO_mt = new G4MaterialPropertiesTable();
    fLYSO_mt->AddProperty("SCINTILLATIONCOMPONENT1", energies.data(), intensities.data(),
                          energies.size());
    fLYSO_mt->AddProperty("RINDEX", energies.data(), indices.data(), energies.size());
    fLYSO_mt->AddProperty("ABSLENGTH", energies.data(), lengths.data(), energies.size());
    fLYSO_mt->AddConstProperty("SCINTILLATIONYIELD", config.number("lyso-yield") / keV);
    fLYSO_mt->AddConstProperty("RESOLUTIONSCALE", 1.0);
    fLYSO_mt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", config.number("lyso-decay-ns") * ns);
    fLYSO_mt->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
    fLYSO->SetMaterialPropertiesTable(fLYSO_mt);
    G4double hemisphereThickness = config.number("shell-thickness-mm") * mm;
    G4double hemisphereRadiusOuterScaled = GeometryParameters::shellOuterRadiusMm * mm;
    G4double hemisphereRadiusInner = hemisphereRadiusOuterScaled - hemisphereThickness;
    auto* scint_hemisphere_virgin =
        new G4Sphere("scint_hemisphere_virgin", hemisphereRadiusInner, hemisphereRadiusOuterScaled,
                     0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI);
    const G4double gasketThickness = GeometryParameters::halfGasketThicknessMm * mm;
    G4ThreeVector scintTrans(0 * mm, 0 * mm, (hemisphereRadiusOuterScaled / 2.0) + gasketThickness);
    G4ThreeVector gasketTrans(0 * mm, 0 * mm,
                              -1.0 * (hemisphereRadiusOuterScaled / 2.0) + gasketThickness);
    G4RotationMatrix* gasketIdentityMatrix = nullptr;
    auto* gasketBox = new G4Box("gasketBox", hemisphereRadiusOuterScaled,
                                hemisphereRadiusOuterScaled, hemisphereRadiusOuterScaled / 2.0);
    auto* hemisphereNorthScint =
        new G4SubtractionSolid("hemisphereNorthScint", scint_hemisphere_virgin, gasketBox,
                               gasketIdentityMatrix, (gasketTrans * -1.0));
    auto* hemisphereNorthGasket =
        new G4SubtractionSolid("hemisphereNorthGasket", scint_hemisphere_virgin, gasketBox,
                               gasketIdentityMatrix, (scintTrans * -1.0));
    auto* hemisphereSouthScint =
        new G4SubtractionSolid("hemisphereSouthScint", scint_hemisphere_virgin, gasketBox,
                               gasketIdentityMatrix, (gasketTrans * -1.0));
    auto* hemisphereSouthGasket =
        new G4SubtractionSolid("hemisphereSouthGasket", scint_hemisphere_virgin, gasketBox,
                               gasketIdentityMatrix, (scintTrans * -1.0));
    auto* hemisphereNorthScint_LV = new G4LogicalVolume(
        hemisphereNorthScint, G4Material::GetMaterial("LYSO"), "hemisphereNorthScint_LV", 0, 0, 0);
    auto* hemisphereNorthGasket_LV =
        new G4LogicalVolume(hemisphereNorthGasket, MaterialFactory::getInstance()->getEpoxy(),
                            "hemisphereNorthGasket_LV", 0, 0, 0);
    auto* hemisphereSouthScint_LV = new G4LogicalVolume(
        hemisphereSouthScint, G4Material::GetMaterial("LYSO"), "hemisphereSouthScint_LV", 0, 0, 0);
    auto* hemisphereSouthGasket_LV =
        new G4LogicalVolume(hemisphereSouthGasket, MaterialFactory::getInstance()->getEpoxy(),
                            "hemisphereSouthGasket_LV", 0, 0, 0);
    G4Colour white = G4Color::White();
    G4Colour yellow = G4Color::Yellow();
    hemisphereNorthScint_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 1.0)));
    hemisphereNorthGasket_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 1.0)));
    hemisphereSouthScint_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 1.0)));
    hemisphereSouthGasket_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 1.0)));
    G4RotationMatrix* rm_x1 = keepRotation(G4RotationMatrix());
    rm_x1->rotateX(180.0 * deg);
    // Boolean cuts separate each gasket from its scintillator. The north rotation
    // puts it in +z; the south occupies -z. Shared origins do not imply overlap.
    auto* hemisphereNorthScint_PV =
        new G4PVPlacement(rm_x1, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm),
                          hemisphereNorthScint_LV, "hemisphereNorthScint_PV", world_LV, false, 0);
    auto* hemisphereNorthGasket_PV =
        new G4PVPlacement(rm_x1, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm),
                          hemisphereNorthGasket_LV, "hemisphereNorthGasket_PV", world_LV, false, 0);
    auto* hemisphereSouthScint_PV =
        new G4PVPlacement(0, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm), hemisphereSouthScint_LV,
                          "hemisphereSouthScint_PV", world_LV, false, 0);
    auto* hemisphereSouthGasket_PV =
        new G4PVPlacement(0, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm), hemisphereSouthGasket_LV,
                          "hemisphereSouthGasket_PV", world_LV, false, 0);
    auto* innermost_hemisphere =
        new G4Sphere("innermost_hemisphere",
                     hemisphereRadiusInner - GeometryParameters::innerLinerThicknessMm * mm,
                     hemisphereRadiusInner, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI);
    auto* innermost_hemisphere_LV =
        new G4LogicalVolume(innermost_hemisphere, MaterialFactory::getInstance()->getEpoxy(),
                            "innermost_hemisphere_LV", 0, 0, 0);
    innermost_hemisphere_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
    auto* innermost_hemisphere_PV =
        new G4PVPlacement(rm_x1, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm),
                          innermost_hemisphere_LV, "innermost_hemisphere_PV", world_LV, false, 0);
    auto* lg_orb_void = new G4Orb("lg_orb_void", hemisphereRadiusOuterScaled);
    G4Colour brown = G4Color::Brown();

    // Vertex coordinates and face connectivity of the supplied faceted guide.
    G4double chapeau_a = (3.0 + sqrt(5.0)) / 6.0;
    G4double chapeau_b = (sqrt(5.0) - 1.0) / 3.0;
    G4double chapeau_c = sqrt(5.0) / 3.0;
    G4double chapeau_bDiv2 = chapeau_b / 2.0;
    G4double chapeau_zero = 0.0;
    G4double chapeau_one = 1.0;
    G4double chapeau_oneThird = 1.0 / 3.0;
    G4double chapeau_twoThirds = 2.0 / 3.0;
    G4ThreeVector chapeau_Vertices[72] = {
        G4ThreeVector(chapeau_oneThird, chapeau_a, chapeau_b),
        G4ThreeVector(chapeau_b, chapeau_oneThird, chapeau_a),
        G4ThreeVector(chapeau_a, chapeau_b, chapeau_oneThird),
        G4ThreeVector(chapeau_twoThirds, chapeau_c, chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, chapeau_twoThirds, chapeau_c),
        G4ThreeVector(chapeau_c, chapeau_bDiv2, chapeau_twoThirds),
        G4ThreeVector(chapeau_one, chapeau_bDiv2, chapeau_zero),
        G4ThreeVector(chapeau_zero, chapeau_one, chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, chapeau_zero, chapeau_one),
        G4ThreeVector(-1.0 * chapeau_oneThird, chapeau_a, chapeau_b),
        G4ThreeVector(-1.0 * chapeau_b, chapeau_oneThird, chapeau_a),
        G4ThreeVector(-1.0 * chapeau_a, chapeau_b, chapeau_oneThird),
        G4ThreeVector(-1.0 * chapeau_twoThirds, chapeau_c, chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, chapeau_twoThirds, chapeau_c),
        G4ThreeVector(-1.0 * chapeau_c, chapeau_bDiv2, chapeau_twoThirds),
        G4ThreeVector(-1.0 * chapeau_one, chapeau_bDiv2, chapeau_zero),
        G4ThreeVector(-1.0 * chapeau_zero, chapeau_one, chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, chapeau_zero, chapeau_one),
        G4ThreeVector(chapeau_oneThird, -1.0 * chapeau_a, chapeau_b),
        G4ThreeVector(chapeau_b, -1.0 * chapeau_oneThird, chapeau_a),
        G4ThreeVector(chapeau_a, -1.0 * chapeau_b, chapeau_oneThird),
        G4ThreeVector(chapeau_twoThirds, -1.0 * chapeau_c, chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, -1.0 * chapeau_twoThirds, chapeau_c),
        G4ThreeVector(chapeau_c, -1.0 * chapeau_bDiv2, chapeau_twoThirds),
        G4ThreeVector(chapeau_one, -1.0 * chapeau_bDiv2, chapeau_zero),
        G4ThreeVector(chapeau_zero, -1.0 * chapeau_one, chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, -1.0 * chapeau_zero, chapeau_one),
        G4ThreeVector(-1.0 * chapeau_oneThird, -1.0 * chapeau_a, chapeau_b),
        G4ThreeVector(-1.0 * chapeau_b, -1.0 * chapeau_oneThird, chapeau_a),
        G4ThreeVector(-1.0 * chapeau_a, -1.0 * chapeau_b, chapeau_oneThird),
        G4ThreeVector(-1.0 * chapeau_twoThirds, -1.0 * chapeau_c, chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds, chapeau_c),
        G4ThreeVector(-1.0 * chapeau_c, -1.0 * chapeau_bDiv2, chapeau_twoThirds),
        G4ThreeVector(-1.0 * chapeau_one, -1.0 * chapeau_bDiv2, chapeau_zero),
        G4ThreeVector(-1.0 * chapeau_zero, -1.0 * chapeau_one, chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, -1.0 * chapeau_zero, chapeau_one),
        G4ThreeVector(chapeau_oneThird, chapeau_a, -1.0 * chapeau_b),
        G4ThreeVector(chapeau_b, chapeau_oneThird, -1.0 * chapeau_a),
        G4ThreeVector(chapeau_a, chapeau_b, -1.0 * chapeau_oneThird),
        G4ThreeVector(chapeau_twoThirds, chapeau_c, -1.0 * chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, chapeau_twoThirds, -1.0 * chapeau_c),
        G4ThreeVector(chapeau_c, chapeau_bDiv2, -1.0 * chapeau_twoThirds),
        G4ThreeVector(chapeau_one, chapeau_bDiv2, -1.0 * chapeau_zero),
        G4ThreeVector(chapeau_zero, chapeau_one, -1.0 * chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, chapeau_zero, -1.0 * chapeau_one),
        G4ThreeVector(-1.0 * chapeau_oneThird, chapeau_a, -1.0 * chapeau_b),
        G4ThreeVector(-1.0 * chapeau_b, chapeau_oneThird, -1.0 * chapeau_a),
        G4ThreeVector(-1.0 * chapeau_a, chapeau_b, -1.0 * chapeau_oneThird),
        G4ThreeVector(-1.0 * chapeau_twoThirds, chapeau_c, -1.0 * chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, chapeau_twoThirds, -1.0 * chapeau_c),
        G4ThreeVector(-1.0 * chapeau_c, chapeau_bDiv2, -1.0 * chapeau_twoThirds),
        G4ThreeVector(-1.0 * chapeau_one, chapeau_bDiv2, -1.0 * chapeau_zero),
        G4ThreeVector(-1.0 * chapeau_zero, chapeau_one, -1.0 * chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, chapeau_zero, -1.0 * chapeau_one),
        G4ThreeVector(chapeau_oneThird, -1.0 * chapeau_a, -1.0 * chapeau_b),
        G4ThreeVector(chapeau_b, -1.0 * chapeau_oneThird, -1.0 * chapeau_a),
        G4ThreeVector(chapeau_a, -1.0 * chapeau_b, -1.0 * chapeau_oneThird),
        G4ThreeVector(chapeau_twoThirds, -1.0 * chapeau_c, -1.0 * chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, -1.0 * chapeau_twoThirds, -1.0 * chapeau_c),
        G4ThreeVector(chapeau_c, -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds),
        G4ThreeVector(chapeau_one, -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero),
        G4ThreeVector(chapeau_zero, -1.0 * chapeau_one, -1.0 * chapeau_bDiv2),
        G4ThreeVector(chapeau_bDiv2, -1.0 * chapeau_zero, -1.0 * chapeau_one),
        G4ThreeVector(-1.0 * chapeau_oneThird, -1.0 * chapeau_a, -1.0 * chapeau_b),
        G4ThreeVector(-1.0 * chapeau_b, -1.0 * chapeau_oneThird, -1.0 * chapeau_a),
        G4ThreeVector(-1.0 * chapeau_a, -1.0 * chapeau_b, -1.0 * chapeau_oneThird),
        G4ThreeVector(-1.0 * chapeau_twoThirds, -1.0 * chapeau_c, -1.0 * chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds, -1.0 * chapeau_c),
        G4ThreeVector(-1.0 * chapeau_c, -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds),
        G4ThreeVector(-1.0 * chapeau_one, -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero),
        G4ThreeVector(-1.0 * chapeau_zero, -1.0 * chapeau_one, -1.0 * chapeau_bDiv2),
        G4ThreeVector(-1.0 * chapeau_bDiv2, -1.0 * chapeau_zero, -1.0 * chapeau_one),
    };
    const G4ThreeVector zero_Vertices[1] = {G4ThreeVector(0.0, 0.0, 0.0)};
    int xVertexOffset = 9;
    int yVertexOffset = 18;
    int xyVertexOffset = 27;
    int zVertexOffset = 36;
    int xzVertexOffset = 45;
    int yzVertexOffset = 54;
    int xyzVertexOffset = 63;
    const int pentagonVertexIndices[12][5] = {
        {5, 1, 8, 1 + yVertexOffset, 5 + yVertexOffset},
        {1 + xyzVertexOffset, 5 + xyzVertexOffset, 5 + xzVertexOffset, 1 + xzVertexOffset,
         8 + xzVertexOffset},
        {0 + xyVertexOffset, 7 + yVertexOffset, 0 + yVertexOffset, 4 + yVertexOffset,
         4 + xyVertexOffset},
        {7 + zVertexOffset, 0 + zVertexOffset, 4 + zVertexOffset, 4 + xzVertexOffset,
         0 + xzVertexOffset},
        {3 + yzVertexOffset, 2 + yzVertexOffset, 6 + yVertexOffset, 2 + yVertexOffset,
         3 + yVertexOffset},
        {5 + zVertexOffset, 5 + yzVertexOffset, 1 + yzVertexOffset, 8 + yzVertexOffset,
         1 + zVertexOffset},
        {2 + zVertexOffset, 3 + zVertexOffset, 3, 2, 6},
        {0 + yzVertexOffset, 7 + yzVertexOffset, 0 + xyzVertexOffset, 4 + xyzVertexOffset,
         4 + yzVertexOffset},
        {7, 0 + xVertexOffset, 4 + xVertexOffset, 4, 0},
        {3 + xyVertexOffset, 2 + xyVertexOffset, 6 + xyVertexOffset, 2 + xyzVertexOffset,
         3 + xyzVertexOffset},
        {5 + xVertexOffset, 5 + xyVertexOffset, 1 + xyVertexOffset, 8 + xVertexOffset,
         1 + xVertexOffset},
        {2 + xVertexOffset, 3 + xVertexOffset, 3 + xzVertexOffset, 2 + xzVertexOffset,
         6 + xVertexOffset},
    };
    const int hexagonVertexIndices[20][6] = {
        {3, 0, 4, 1, 5, 2},
        {0 + xyzVertexOffset, 3 + xyzVertexOffset, 2 + xyzVertexOffset, 5 + xyzVertexOffset,
         1 + xyzVertexOffset, 4 + xyzVertexOffset},
        {4 + xVertexOffset, 1 + xVertexOffset, 8 + xVertexOffset, 8, 1, 4},
        {6 + xyVertexOffset, 6 + xVertexOffset, 2 + xzVertexOffset, 5 + xzVertexOffset,
         5 + xyzVertexOffset, 2 + xyzVertexOffset},
        {1 + xyVertexOffset, 4 + xyVertexOffset, 4 + yVertexOffset, 1 + yVertexOffset,
         8 + yVertexOffset, 8 + xVertexOffset},
        {3 + xzVertexOffset, 0 + xzVertexOffset, 4 + xzVertexOffset, 1 + xzVertexOffset,
         5 + xzVertexOffset, 2 + xzVertexOffset},
        {0 + yVertexOffset, 3 + yVertexOffset, 2 + yVertexOffset, 5 + yVertexOffset,
         1 + yVertexOffset, 4 + yVertexOffset},
        {4 + zVertexOffset, 1 + zVertexOffset, 8 + zVertexOffset, 8 + xzVertexOffset,
         1 + xzVertexOffset, 4 + xzVertexOffset},
        {6 + yVertexOffset, 6, 2, 5, 5 + yVertexOffset, 2 + yVertexOffset},
        {1 + yzVertexOffset, 4 + yzVertexOffset, 4 + xyzVertexOffset, 1 + xyzVertexOffset,
         8 + xzVertexOffset, 8 + zVertexOffset},
        {0 + zVertexOffset, 7 + zVertexOffset, 7, 0, 3, 3 + zVertexOffset},
        {7 + yVertexOffset, 0 + xyVertexOffset, 3 + xyVertexOffset, 3 + xyzVertexOffset,
         0 + xyzVertexOffset, 7 + yzVertexOffset},
        {3 + xVertexOffset, 2 + xVertexOffset, 5 + xVertexOffset, 1 + xVertexOffset,
         4 + xVertexOffset, 0 + xVertexOffset},
        {5 + xyVertexOffset, 5 + xVertexOffset, 2 + xVertexOffset, 6 + xVertexOffset,
         6 + xyVertexOffset, 2 + xyVertexOffset},
        {2 + xyVertexOffset, 3 + xyVertexOffset, 0 + xyVertexOffset, 4 + xyVertexOffset,
         1 + xyVertexOffset, 5 + xyVertexOffset},
        {0 + xVertexOffset, 7, 7 + zVertexOffset, 0 + xzVertexOffset, 3 + xzVertexOffset,
         3 + xVertexOffset},
        {7 + yzVertexOffset, 0 + yzVertexOffset, 3 + yzVertexOffset, 3 + yVertexOffset,
         0 + yVertexOffset, 7 + yVertexOffset},
        {3 + zVertexOffset, 2 + zVertexOffset, 5 + zVertexOffset, 1 + zVertexOffset,
         4 + zVertexOffset, 0 + zVertexOffset},
        {5 + yzVertexOffset, 5 + zVertexOffset, 2 + zVertexOffset, 6, 6 + yVertexOffset,
         2 + yzVertexOffset},
        {2 + yzVertexOffset, 3 + yzVertexOffset, 0 + yzVertexOffset, 4 + yzVertexOffset,
         1 + yzVertexOffset, 5 + yzVertexOffset},
    };
    G4ThreeVector P1_CoG =
        (chapeau_Vertices[1] + chapeau_Vertices[5] + chapeau_Vertices[5 + yVertexOffset] +
         chapeau_Vertices[1 + yVertexOffset] + chapeau_Vertices[8]) /
        5.0;
    G4ThreeVector zPosAxis = G4ThreeVector(0.0 * mm, 0.0 * mm, 1.0 * mm);
    G4ThreeVector myCross1 = zPosAxis.cross(P1_CoG);
    G4double angleDelta1 = zPosAxis.theta(P1_CoG);
    int vertIndex = 0;
    while (vertIndex < 72) {
        chapeau_Vertices[vertIndex] =
            chapeau_Vertices[vertIndex].rotate(myCross1, -1.0 * angleDelta1);
        vertIndex = vertIndex + 1;
    }
    P1_CoG = (chapeau_Vertices[1] + chapeau_Vertices[5] + chapeau_Vertices[5 + yVertexOffset] +
              chapeau_Vertices[1 + yVertexOffset] + chapeau_Vertices[8]) /
             5.0;
    G4ThreeVector P1_axis1 = (chapeau_Vertices[1 + yVertexOffset] - P1_CoG) * -1.0;
    G4ThreeVector yPosAxis = G4ThreeVector(0.0 * mm, 1.0 * mm, 0.0 * mm);
    G4ThreeVector myCross2 = yPosAxis.cross(P1_axis1);
    G4double angleDelta2 = yPosAxis.theta(P1_axis1);
    vertIndex = 0;
    while (vertIndex < 72) {
        chapeau_Vertices[vertIndex] =
            chapeau_Vertices[vertIndex].rotate(myCross2, -1.0 * (angleDelta2 + (90.0 * deg)));
        vertIndex = vertIndex + 1;
    }
    G4double bigR = sqrt((21.0 - sqrt(5.0)) / 18.0);
    G4double littleR = (sqrt(3.0) * (sqrt(5.0) + 1.0)) / 6.0;
    G4double ratioRs = bigR / littleR;
    const G4double knownR = GeometryParameters::shellOuterRadiusMm * mm;
    G4double targetR = knownR * ratioRs;
    G4double chapeau_scale = targetR / bigR;
    const G4double sipmR = GeometryParameters::guideOutletRadiusMm * mm;
    // Housing front touches the guide outlet; placement is by housing center.
    const G4double sensorCenterR = sipmR + housing->getDz() / 2.0;
    const G4double chapeau_casting_scale = GeometryParameters::maskExtensionScale;
    G4ThreeVector pentagonQuad_Vertices[12][4 * 2];
    G4ThreeVector hexagonQuad_Vertices[20][4 * 2];
    int pentagonIndex = 0;
    const int pentagonCount = GeometryParameters::pentagonCount;
    while (pentagonIndex < pentagonCount) {
        G4ThreeVector PB_CoG =
            ((chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale) +
             (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale) +
             (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale) +
             (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale) +
             (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale)) /
            5.0;
        G4ThreeVector PB_axis1 =
            (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale) -
            (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale);
        G4ThreeVector PB_axis2 =
            (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale) - PB_CoG;
        PB_CoG = PB_CoG / PB_CoG.mag();
        PB_axis1 = PB_axis1 / PB_axis1.mag();
        PB_axis2 = PB_axis2 / PB_axis2.mag();
        G4double pbHalf = GeometryParameters::guideOutletHalfWidthMm * mm;
        pentagonQuad_Vertices[pentagonIndex][0] =
            G4ThreeVector(((zero_Vertices[0] - PB_axis1 - PB_axis2) * pbHalf) + (PB_CoG * sipmR));
        pentagonQuad_Vertices[pentagonIndex][1] =
            G4ThreeVector(((zero_Vertices[0] + PB_axis1 - PB_axis2) * pbHalf) + (PB_CoG * sipmR));
        pentagonQuad_Vertices[pentagonIndex][2] =
            G4ThreeVector(((zero_Vertices[0] + PB_axis1 + PB_axis2) * pbHalf) + (PB_CoG * sipmR));
        pentagonQuad_Vertices[pentagonIndex][3] =
            G4ThreeVector(((zero_Vertices[0] - PB_axis1 + PB_axis2) * pbHalf) + (PB_CoG * sipmR));
        pentagonQuad_Vertices[pentagonIndex][4] =
            G4ThreeVector(((zero_Vertices[0] - PB_axis1 - PB_axis2) * pbHalf) +
                          (PB_CoG * sipmR * chapeau_casting_scale));
        pentagonQuad_Vertices[pentagonIndex][5] =
            G4ThreeVector(((zero_Vertices[0] + PB_axis1 - PB_axis2) * pbHalf) +
                          (PB_CoG * sipmR * chapeau_casting_scale));
        pentagonQuad_Vertices[pentagonIndex][6] =
            G4ThreeVector(((zero_Vertices[0] + PB_axis1 + PB_axis2) * pbHalf) +
                          (PB_CoG * sipmR * chapeau_casting_scale));
        pentagonQuad_Vertices[pentagonIndex][7] =
            G4ThreeVector(((zero_Vertices[0] - PB_axis1 + PB_axis2) * pbHalf) +
                          (PB_CoG * sipmR * chapeau_casting_scale));
        pentagonIndex = pentagonIndex + 1;
    }
    int hexagonIndex = 0;
    const int hexagonCount = GeometryParameters::hexagonCount;
    while (hexagonIndex < hexagonCount) {
        G4ThreeVector HB_CoG =
            ((chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale) +
             (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale)) /
            2.0;
        G4ThreeVector HB_axis1 =
            (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale) - HB_CoG;
        G4ThreeVector HB_axis2 =
            (((chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale) +
              (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale)) /
             2.0) -
            HB_CoG;
        HB_CoG = HB_CoG / HB_CoG.mag();
        HB_axis1 = HB_axis1 / HB_axis1.mag();
        HB_axis2 = HB_axis2 / HB_axis2.mag();
        G4double hbHalf = GeometryParameters::guideOutletHalfWidthMm * mm;
        hexagonQuad_Vertices[hexagonIndex][0] =
            G4ThreeVector(((zero_Vertices[0] - HB_axis1 - HB_axis2) * hbHalf) + (HB_CoG * sipmR));
        hexagonQuad_Vertices[hexagonIndex][1] =
            G4ThreeVector(((zero_Vertices[0] + HB_axis1 - HB_axis2) * hbHalf) + (HB_CoG * sipmR));
        hexagonQuad_Vertices[hexagonIndex][2] =
            G4ThreeVector(((zero_Vertices[0] + HB_axis1 + HB_axis2) * hbHalf) + (HB_CoG * sipmR));
        hexagonQuad_Vertices[hexagonIndex][3] =
            G4ThreeVector(((zero_Vertices[0] - HB_axis1 + HB_axis2) * hbHalf) + (HB_CoG * sipmR));
        hexagonQuad_Vertices[hexagonIndex][4] =
            G4ThreeVector(((zero_Vertices[0] - HB_axis1 - HB_axis2) * hbHalf) +
                          (HB_CoG * sipmR * chapeau_casting_scale));
        hexagonQuad_Vertices[hexagonIndex][5] =
            G4ThreeVector(((zero_Vertices[0] + HB_axis1 - HB_axis2) * hbHalf) +
                          (HB_CoG * sipmR * chapeau_casting_scale));
        hexagonQuad_Vertices[hexagonIndex][6] =
            G4ThreeVector(((zero_Vertices[0] + HB_axis1 + HB_axis2) * hbHalf) +
                          (HB_CoG * sipmR * chapeau_casting_scale));
        hexagonQuad_Vertices[hexagonIndex][7] =
            G4ThreeVector(((zero_Vertices[0] - HB_axis1 + HB_axis2) * hbHalf) +
                          (HB_CoG * sipmR * chapeau_casting_scale));
        hexagonIndex = hexagonIndex + 1;
    }
    int squareIndex = 0;
    int squareCount = 2 * 20;
    hexagonIndex = 0;
    while (squareIndex < squareCount) {
        chapeauPoof_A2_SquareHexagonFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][0], hexagonQuad_Vertices[hexagonIndex][1],
            hexagonQuad_Vertices[hexagonIndex][2], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauPoof_A2_SquareHexagonFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][2], hexagonQuad_Vertices[hexagonIndex][3],
            hexagonQuad_Vertices[hexagonIndex][0], ABSOLUTE);
        squareIndex = squareIndex + 1;
        hexagonIndex = hexagonIndex + 1;
    }
    squareIndex = 0;
    squareCount = 2 * 12;
    pentagonIndex = 0;
    while (squareIndex < squareCount) {
        chapeauPoof_A2_SquarePentagonFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][0], pentagonQuad_Vertices[pentagonIndex][1],
            pentagonQuad_Vertices[pentagonIndex][2], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauPoof_A2_SquarePentagonFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][2], pentagonQuad_Vertices[pentagonIndex][3],
            pentagonQuad_Vertices[pentagonIndex][0], ABSOLUTE);
        squareIndex = squareIndex + 1;
        pentagonIndex = pentagonIndex + 1;
    }
    int pentagonSkirtIndex = 0;
    pentagonIndex = 0;
    while (pentagonIndex < pentagonCount) {
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0], pentagonQuad_Vertices[pentagonIndex][3],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][3],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][3], pentagonQuad_Vertices[pentagonIndex][2],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][2],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][2], pentagonQuad_Vertices[pentagonIndex][1],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][1],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][1], pentagonQuad_Vertices[pentagonIndex][0],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauPoof_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        pentagonIndex = pentagonIndex + 1;
    }
    int hexagonSkirtIndex = 0;
    hexagonIndex = 0;
    while (hexagonIndex < hexagonCount) {
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0], hexagonQuad_Vertices[hexagonIndex][3], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][3],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][3], hexagonQuad_Vertices[hexagonIndex][2], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2], hexagonQuad_Vertices[hexagonIndex][1], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][1],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][1], hexagonQuad_Vertices[hexagonIndex][0], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauPoof_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        hexagonIndex = hexagonIndex + 1;
    }
    squareIndex = 0;
    squareCount = 10 * 20;
    hexagonIndex = 0;
    while (squareIndex < squareCount) {
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][4], hexagonQuad_Vertices[hexagonIndex][5],
            hexagonQuad_Vertices[hexagonIndex][6], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][6], hexagonQuad_Vertices[hexagonIndex][7],
            hexagonQuad_Vertices[hexagonIndex][4], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][0], hexagonQuad_Vertices[hexagonIndex][4],
            hexagonQuad_Vertices[hexagonIndex][3], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][1], hexagonQuad_Vertices[hexagonIndex][5],
            hexagonQuad_Vertices[hexagonIndex][0], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][2], hexagonQuad_Vertices[hexagonIndex][6],
            hexagonQuad_Vertices[hexagonIndex][1], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][3], hexagonQuad_Vertices[hexagonIndex][7],
            hexagonQuad_Vertices[hexagonIndex][2], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][4], hexagonQuad_Vertices[hexagonIndex][0],
            hexagonQuad_Vertices[hexagonIndex][5], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][5], hexagonQuad_Vertices[hexagonIndex][1],
            hexagonQuad_Vertices[hexagonIndex][6], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][6], hexagonQuad_Vertices[hexagonIndex][2],
            hexagonQuad_Vertices[hexagonIndex][7], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquareHexagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            hexagonQuad_Vertices[hexagonIndex][7], hexagonQuad_Vertices[hexagonIndex][3],
            hexagonQuad_Vertices[hexagonIndex][4], ABSOLUTE);
        squareIndex = squareIndex + 1;
        hexagonIndex = hexagonIndex + 1;
    }
    squareIndex = 0;
    squareCount = 10 * 12;
    pentagonIndex = 0;
    while (squareIndex < squareCount) {
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][4], pentagonQuad_Vertices[pentagonIndex][5],
            pentagonQuad_Vertices[pentagonIndex][6], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][6], pentagonQuad_Vertices[pentagonIndex][7],
            pentagonQuad_Vertices[pentagonIndex][4], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][0], pentagonQuad_Vertices[pentagonIndex][4],
            pentagonQuad_Vertices[pentagonIndex][3], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][1], pentagonQuad_Vertices[pentagonIndex][5],
            pentagonQuad_Vertices[pentagonIndex][0], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][2], pentagonQuad_Vertices[pentagonIndex][6],
            pentagonQuad_Vertices[pentagonIndex][1], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][3], pentagonQuad_Vertices[pentagonIndex][7],
            pentagonQuad_Vertices[pentagonIndex][2], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][4], pentagonQuad_Vertices[pentagonIndex][0],
            pentagonQuad_Vertices[pentagonIndex][5], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][5], pentagonQuad_Vertices[pentagonIndex][1],
            pentagonQuad_Vertices[pentagonIndex][6], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][6], pentagonQuad_Vertices[pentagonIndex][2],
            pentagonQuad_Vertices[pentagonIndex][7], ABSOLUTE);
        squareIndex = squareIndex + 1;
        chapeauCasting_A2_SquarePentagonExtensionFacets[squareIndex] = new G4TriangularFacet(
            pentagonQuad_Vertices[pentagonIndex][7], pentagonQuad_Vertices[pentagonIndex][3],
            pentagonQuad_Vertices[pentagonIndex][4], ABSOLUTE);
        squareIndex = squareIndex + 1;
        pentagonIndex = pentagonIndex + 1;
    }
    pentagonSkirtIndex = 0;
    pentagonIndex = 0;
    while (pentagonIndex < pentagonCount) {
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0], pentagonQuad_Vertices[pentagonIndex][3],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][3],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][3], pentagonQuad_Vertices[pentagonIndex][2],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][2],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][2], pentagonQuad_Vertices[pentagonIndex][1],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][1],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][1], pentagonQuad_Vertices[pentagonIndex][0],
            ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        chapeauCasting_A2_SquarePentagonSkirtFacets[pentagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale,
            pentagonQuad_Vertices[pentagonIndex][0],
            chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale, ABSOLUTE);
        pentagonSkirtIndex = pentagonSkirtIndex + 1;
        pentagonIndex = pentagonIndex + 1;
    }
    hexagonSkirtIndex = 0;
    hexagonIndex = 0;
    while (hexagonIndex < hexagonCount) {
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0], hexagonQuad_Vertices[hexagonIndex][3], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][5]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][3],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][3], hexagonQuad_Vertices[hexagonIndex][2], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][2], hexagonQuad_Vertices[hexagonIndex][1], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][1],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][1], hexagonQuad_Vertices[hexagonIndex][0], ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        chapeauCasting_A2_SquareHexagonSkirtFacets[hexagonSkirtIndex] = new G4TriangularFacet(
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][1]] * chapeau_scale,
            hexagonQuad_Vertices[hexagonIndex][0],
            chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale, ABSOLUTE);
        hexagonSkirtIndex = hexagonSkirtIndex + 1;
        hexagonIndex = hexagonIndex + 1;
    }
    // Tessellated solids own the facets passed to AddFacet().
    auto* chapeauPoof_A2 = new G4TessellatedSolid("chapeauPoof_A2");
    auto* chapeauCasting_A2 = new G4TessellatedSolid("chapeauCasting_A2");
    for (int face = 0; face < GeometryParameters::pentagonCount; ++face) {
        for (int facet = 0; facet < 9; ++facet)
            chapeauPoof_A2->AddFacet(chapeauPoof_A2_SquarePentagonSkirtFacets[face * 9 + facet]);
        for (int facet = 0; facet < 2; ++facet)
            chapeauPoof_A2->AddFacet(chapeauPoof_A2_SquarePentagonFacets[face * 2 + facet]);
        for (int facet = 0; facet < 9; ++facet)
            chapeauCasting_A2->AddFacet(
                chapeauCasting_A2_SquarePentagonSkirtFacets[face * 9 + facet]);
        for (int facet = 0; facet < 10; ++facet)
            chapeauCasting_A2->AddFacet(
                chapeauCasting_A2_SquarePentagonExtensionFacets[face * 10 + facet]);
    }
    for (int face = 0; face < GeometryParameters::hexagonCount; ++face) {
        for (int facet = 0; facet < 10; ++facet)
            chapeauPoof_A2->AddFacet(chapeauPoof_A2_SquareHexagonSkirtFacets[face * 10 + facet]);
        for (int facet = 0; facet < 2; ++facet)
            chapeauPoof_A2->AddFacet(chapeauPoof_A2_SquareHexagonFacets[face * 2 + facet]);
        for (int facet = 0; facet < 10; ++facet)
            chapeauCasting_A2->AddFacet(
                chapeauCasting_A2_SquareHexagonSkirtFacets[face * 10 + facet]);
        for (int facet = 0; facet < 10; ++facet)
            chapeauCasting_A2->AddFacet(
                chapeauCasting_A2_SquareHexagonExtensionFacets[face * 10 + facet]);
    }
    chapeauPoof_A2->SetSolidClosed(true);
    chapeauCasting_A2->SetSolidClosed(true);

    // Preserve the supplied outer-mask radius using its original reference corner.
    const G4ThreeVector maskCenter =
        ((chapeau_Vertices[2] * chapeau_scale) + (chapeau_Vertices[4] * chapeau_scale)) / 2.0;
    G4ThreeVector maskAxis1 =
        ((chapeau_Vertices[1] * chapeau_scale + chapeau_Vertices[5] * chapeau_scale) / 2.0) -
        maskCenter;
    G4ThreeVector maskAxis2 = chapeau_Vertices[2] * chapeau_scale - maskCenter;
    maskAxis1 = maskAxis1 / maskAxis1.mag();
    maskAxis2 = maskAxis2 / maskAxis2.mag();
    const G4ThreeVector maskCorner = (zero_Vertices[0] + maskAxis1 + maskAxis2) *
                                         (GeometryParameters::guideOutletHalfWidthMm * mm) +
                                     maskCenter * (sipmR / knownR);

    auto* chapeauPoofMinusHomeOrb =
        new G4SubtractionSolid("chapeauPoofMinusHomeOrb", chapeauPoof_A2, lg_orb_void);
    // Optional bevel removes corners; the default guide is unchanged.
    G4VSolid* selectedGuide = chapeauPoofMinusHomeOrb;
    const bool octagons = config.text("guide-shape") == "pentagon-octagon";
    std::ofstream sections(config.text("output") + "/guide_sections.csv");
    if (!sections)
        throw std::runtime_error("Cannot write guide_sections.csv");
    sections << "array_id,shape,vertex,x_mm,y_mm,z_mm\n" << std::setprecision(12);
    for (int array = 0; array < 32; ++array) {
        const bool pent = array < 12;
        const int index = pent ? array : array - 12;
        std::vector<G4ThreeVector> ring;
        for (int v = 0; v < (pent ? 5 : 6); ++v)
            ring.push_back(chapeau_Vertices[pent ? pentagonVertexIndices[index][v]
                                                 : hexagonVertexIndices[index][v]] *
                           chapeau_scale);
        if (octagons && !pent) {
            for (int corner : {0, 3}) {
                auto* cut = guideCornerCut(
                    "guideCornerCut" + std::to_string(array) + "_" + std::to_string(corner),
                    ring[corner], ring[(corner + 5) % 6], ring[(corner + 1) % 6],
                    hexagonQuad_Vertices[index], config.number("guide-bevel-fraction"));
                selectedGuide = new G4SubtractionSolid(
                    "pentagonOctagonGuide" + std::to_string(array) + "_" + std::to_string(corner),
                    selectedGuide, cut);
            }
            ring = bevelGuideHexagon(ring, config.number("guide-bevel-fraction"));
        }
        for (size_t v = 0; v < ring.size(); ++v)
            sections << array << ',' << (pent ? "pentagon" : (octagons ? "octagon" : "hexagon"))
                     << ',' << v << ',' << ring[v].x() / mm << ',' << ring[v].y() / mm << ','
                     << ring[v].z() / mm << '\n';
    }
    sections.close();
    if (!sections)
        throw std::runtime_error("Cannot finish guide_sections.csv");
    auto* chapeauPoofMinusHomeOrb_LV =
        new G4LogicalVolume(selectedGuide, MaterialFactory::getInstance()->getEpoxy(),
                            "chapeauPoofMinusHomeOrb", 0, 0, 0);
    chapeauPoofMinusHomeOrb_LV->SetVisAttributes(new G4VisAttributes(
        G4Color(brown.GetRed() + 0.2, brown.GetGreen() + 0.2, brown.GetBlue() + 0.2, 1.0)));
    G4RotationMatrix* chapeauPoofMinusHomeOrbMatrix = keepRotation(G4RotationMatrix());

    auto* chapeauPoofMinusHomeOrb_PV = new G4PVPlacement(
        chapeauPoofMinusHomeOrbMatrix, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm),
        chapeauPoofMinusHomeOrb_LV, "chapeauPoofMinusHomeOrb_PV", world_LV, false, 0);
    auto* homeSphere =
        new G4Sphere("homeSphere", (chapeau_Vertices[0] * chapeau_scale).mag(), maskCorner.mag(),
                     0.0 * deg, 360.0 * deg, 0.0 * deg, 180.0 * deg);
    auto* homeSphereMinusChapeauCasting =
        new G4SubtractionSolid("homeSphereMinusChapeauCasting", homeSphere, chapeauCasting_A2);
    auto* homeSphereMinusChapeauCasting_LV = new G4LogicalVolume(
        homeSphereMinusChapeauCasting, MaterialFactory::getInstance()->getEpoxy(),
        "homeSphereMinusChapeauCasting_A2", 0, 0, 0);
    homeSphereMinusChapeauCasting_LV->SetVisAttributes(
        new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
    auto* homeSphereMinusChapeauCasting_PV = new G4PVPlacement(
        chapeauPoofMinusHomeOrbMatrix, G4ThreeVector(0.0 * mm, 0.0 * mm, 0.0 * mm),
        homeSphereMinusChapeauCasting_LV, "homeSphereMinusChapeauCasting_PV", world_LV, false, 0);
    G4String absorptiveOpticalSurfaceChapeauPoofString =
        G4String("absorptiveOpticalSurfaceChapeauPoof");
    G4OpticalSurface* absorptiveOpticalSurfaceChapeauPoof =
        new G4OpticalSurface(absorptiveOpticalSurfaceChapeauPoofString);
    new G4LogicalBorderSurface(absorptiveOpticalSurfaceChapeauPoofString,
                               chapeauPoofMinusHomeOrb_PV, homeSphereMinusChapeauCasting_PV,
                               absorptiveOpticalSurfaceChapeauPoof);
    absorptiveOpticalSurfaceChapeauPoof->SetType(dielectric_metal);
    absorptiveOpticalSurfaceChapeauPoof->SetFinish(polished);
    absorptiveOpticalSurfaceChapeauPoof->SetModel(glisur);
    G4double ppChapeauPoof[] = {2.0 * eV, 3.5 * eV};
    const G4int numChapeauPoof = sizeof(ppChapeauPoof) / sizeof(G4double);
    G4double reflectivityChapeauPoof[] = {0.0, 0.0};
    assert(sizeof(reflectivityChapeauPoof) == sizeof(ppChapeauPoof));
    G4double efficiencyChapeauPoof[] = {1.0, 1.0};
    assert(sizeof(efficiencyChapeauPoof) == sizeof(ppChapeauPoof));
    G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyChapeauPoof =
        new G4MaterialPropertiesTable();
    absorptiveOpticalSurfacePropertyChapeauPoof->AddProperty(
        "REFLECTIVITY", ppChapeauPoof, reflectivityChapeauPoof, numChapeauPoof);
    absorptiveOpticalSurfacePropertyChapeauPoof->AddProperty("EFFICIENCY", ppChapeauPoof,
                                                             efficiencyChapeauPoof, numChapeauPoof);
    absorptiveOpticalSurfaceChapeauPoof->SetMaterialPropertiesTable(
        absorptiveOpticalSurfacePropertyChapeauPoof);
    G4String absorptiveOpticalSurfacehemisphereNorthScintString =
        G4String("absorptiveOpticalSurfacehemisphereNorthScint");
    G4OpticalSurface* absorptiveOpticalSurfacehemisphereNorthScint =
        new G4OpticalSurface(absorptiveOpticalSurfacehemisphereNorthScintString);
    new G4LogicalBorderSurface(absorptiveOpticalSurfacehemisphereNorthScintString,
                               hemisphereNorthScint_PV, innermost_hemisphere_PV,
                               absorptiveOpticalSurfacehemisphereNorthScint);
    absorptiveOpticalSurfacehemisphereNorthScint->SetType(dielectric_metal);
    absorptiveOpticalSurfacehemisphereNorthScint->SetFinish(ground);
    absorptiveOpticalSurfacehemisphereNorthScint->SetModel(glisur);
    G4double pphemisphereNorthScint[] = {2.0 * eV, 3.5 * eV};
    const G4int numhemisphereNorthScint = sizeof(pphemisphereNorthScint) / sizeof(G4double);
    G4double reflectivityhemisphereNorthScint[] = {1.0, 1.0};
    assert(sizeof(reflectivityhemisphereNorthScint) == sizeof(pphemisphereNorthScint));
    G4double efficiencyhemisphereNorthScint[] = {0.0, 0.0};
    assert(sizeof(efficiencyhemisphereNorthScint) == sizeof(pphemisphereNorthScint));
    G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereNorthScint =
        new G4MaterialPropertiesTable();
    absorptiveOpticalSurfacePropertyhemisphereNorthScint->AddProperty(
        "REFLECTIVITY", pphemisphereNorthScint, reflectivityhemisphereNorthScint,
        numhemisphereNorthScint);
    absorptiveOpticalSurfacePropertyhemisphereNorthScint->AddProperty(
        "EFFICIENCY", pphemisphereNorthScint, efficiencyhemisphereNorthScint,
        numhemisphereNorthScint);
    absorptiveOpticalSurfacehemisphereNorthScint->SetMaterialPropertiesTable(
        absorptiveOpticalSurfacePropertyhemisphereNorthScint);
    G4String absorptiveOpticalSurfacehemisphereNorthGasketString =
        G4String("absorptiveOpticalSurfacehemisphereNorthGasket");
    G4OpticalSurface* absorptiveOpticalSurfacehemisphereNorthGasket =
        new G4OpticalSurface(absorptiveOpticalSurfacehemisphereNorthGasketString);
    new G4LogicalBorderSurface(absorptiveOpticalSurfacehemisphereNorthGasketString,
                               hemisphereNorthGasket_PV, innermost_hemisphere_PV,
                               absorptiveOpticalSurfacehemisphereNorthGasket);
    absorptiveOpticalSurfacehemisphereNorthGasket->SetType(dielectric_metal);
    absorptiveOpticalSurfacehemisphereNorthGasket->SetFinish(ground);
    absorptiveOpticalSurfacehemisphereNorthGasket->SetModel(glisur);
    G4double pphemisphereNorthGasket[] = {2.0 * eV, 3.5 * eV};
    const G4int numhemisphereNorthGasket = sizeof(pphemisphereNorthGasket) / sizeof(G4double);
    G4double reflectivityhemisphereNorthGasket[] = {1.0, 1.0};
    assert(sizeof(reflectivityhemisphereNorthGasket) == sizeof(pphemisphereNorthGasket));
    G4double efficiencyhemisphereNorthGasket[] = {0.0, 0.0};
    assert(sizeof(efficiencyhemisphereNorthGasket) == sizeof(pphemisphereNorthGasket));
    G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereNorthGasket =
        new G4MaterialPropertiesTable();
    absorptiveOpticalSurfacePropertyhemisphereNorthGasket->AddProperty(
        "REFLECTIVITY", pphemisphereNorthGasket, reflectivityhemisphereNorthGasket,
        numhemisphereNorthGasket);
    absorptiveOpticalSurfacePropertyhemisphereNorthGasket->AddProperty(
        "EFFICIENCY", pphemisphereNorthGasket, efficiencyhemisphereNorthGasket,
        numhemisphereNorthGasket);
    absorptiveOpticalSurfacehemisphereNorthGasket->SetMaterialPropertiesTable(
        absorptiveOpticalSurfacePropertyhemisphereNorthGasket);
    G4String absorptiveOpticalSurfacehemisphereSouthScintString =
        G4String("absorptiveOpticalSurfacehemisphereSouthScint");
    G4OpticalSurface* absorptiveOpticalSurfacehemisphereSouthScint =
        new G4OpticalSurface(absorptiveOpticalSurfacehemisphereSouthScintString);
    new G4LogicalBorderSurface(absorptiveOpticalSurfacehemisphereSouthScintString,
                               hemisphereSouthScint_PV, innermost_hemisphere_PV,
                               absorptiveOpticalSurfacehemisphereSouthScint);
    absorptiveOpticalSurfacehemisphereSouthScint->SetType(dielectric_metal);
    absorptiveOpticalSurfacehemisphereSouthScint->SetFinish(ground);
    absorptiveOpticalSurfacehemisphereSouthScint->SetModel(glisur);
    G4double pphemisphereSouthScint[] = {2.0 * eV, 3.5 * eV};
    const G4int numhemisphereSouthScint = sizeof(pphemisphereSouthScint) / sizeof(G4double);
    G4double reflectivityhemisphereSouthScint[] = {1.0, 1.0};
    assert(sizeof(reflectivityhemisphereSouthScint) == sizeof(pphemisphereSouthScint));
    G4double efficiencyhemisphereSouthScint[] = {0.0, 0.0};
    assert(sizeof(efficiencyhemisphereSouthScint) == sizeof(pphemisphereSouthScint));
    G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereSouthScint =
        new G4MaterialPropertiesTable();
    absorptiveOpticalSurfacePropertyhemisphereSouthScint->AddProperty(
        "REFLECTIVITY", pphemisphereSouthScint, reflectivityhemisphereSouthScint,
        numhemisphereSouthScint);
    absorptiveOpticalSurfacePropertyhemisphereSouthScint->AddProperty(
        "EFFICIENCY", pphemisphereSouthScint, efficiencyhemisphereSouthScint,
        numhemisphereSouthScint);
    absorptiveOpticalSurfacehemisphereSouthScint->SetMaterialPropertiesTable(
        absorptiveOpticalSurfacePropertyhemisphereSouthScint);
    G4String absorptiveOpticalSurfacehemisphereSouthGasketString =
        G4String("absorptiveOpticalSurfacehemisphereSouthGasket");
    G4OpticalSurface* absorptiveOpticalSurfacehemisphereSouthGasket =
        new G4OpticalSurface(absorptiveOpticalSurfacehemisphereSouthGasketString);
    new G4LogicalBorderSurface(absorptiveOpticalSurfacehemisphereSouthGasketString,
                               hemisphereSouthGasket_PV, innermost_hemisphere_PV,
                               absorptiveOpticalSurfacehemisphereSouthGasket);
    absorptiveOpticalSurfacehemisphereSouthGasket->SetType(dielectric_metal);
    absorptiveOpticalSurfacehemisphereSouthGasket->SetFinish(ground);
    absorptiveOpticalSurfacehemisphereSouthGasket->SetModel(glisur);
    G4double pphemisphereSouthGasket[] = {2.0 * eV, 3.5 * eV};
    const G4int numhemisphereSouthGasket = sizeof(pphemisphereSouthGasket) / sizeof(G4double);
    G4double reflectivityhemisphereSouthGasket[] = {1.0, 1.0};
    assert(sizeof(reflectivityhemisphereSouthGasket) == sizeof(pphemisphereSouthGasket));
    G4double efficiencyhemisphereSouthGasket[] = {0.0, 0.0};
    assert(sizeof(efficiencyhemisphereSouthGasket) == sizeof(pphemisphereSouthGasket));
    G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereSouthGasket =
        new G4MaterialPropertiesTable();
    absorptiveOpticalSurfacePropertyhemisphereSouthGasket->AddProperty(
        "REFLECTIVITY", pphemisphereSouthGasket, reflectivityhemisphereSouthGasket,
        numhemisphereSouthGasket);
    absorptiveOpticalSurfacePropertyhemisphereSouthGasket->AddProperty(
        "EFFICIENCY", pphemisphereSouthGasket, efficiencyhemisphereSouthGasket,
        numhemisphereSouthGasket);
    absorptiveOpticalSurfacehemisphereSouthGasket->SetMaterialPropertiesTable(
        absorptiveOpticalSurfacePropertyhemisphereSouthGasket);
    // Place 4 x 4 sensors on every pentagonal and hexagonal outlet.
    pentagonIndex = 0;
    G4RotationMatrix* myRotMatrixPent[12][16];
    G4ThreeVector PB_CoG[12][16];
    G4ThreeVector PB_axis1[12][16];
    G4ThreeVector PB_axis2[12][16];
    G4ThreeVector zPosAxisOut[12][16];
    G4ThreeVector myCross1P[12][16];
    G4double angleDelta1P[12][16];
    G4RotationMatrix facetMatrix1[12][16];
    G4ThreeVector zPosAxis1[12][16];
    G4ThreeVector zPosAxis1New[12][16];
    G4double angleDelta2P[12][16];
    G4double dot2P[12][16];
    G4RotationMatrix facetMatrix2[12][16];
    G4float mySipmPitch = config.number("sipm-pitch-mm") * mm;
    G4ThreeVector home_Vertex[12][16];
    int axis1Index;
    int axis2Index;
    int triggerMan;
    G4double rotationTwister;
    while (pentagonIndex < pentagonCount) {
        triggerMan = 0;
        axis1Index = 0;
        axis2Index = 0;
        while (axis2Index < 4) {
            while (axis1Index < 4) {
                PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    ((chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale) +
                     (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale) +
                     (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][2]] * chapeau_scale) +
                     (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale) +
                     (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][4]] * chapeau_scale)) /
                    5.0;
                PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][1]] * chapeau_scale) -
                    (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][0]] * chapeau_scale);
                PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    (chapeau_Vertices[pentagonVertexIndices[pentagonIndex][3]] * chapeau_scale) -
                    PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)];
                PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)] /
                    PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)] /
                    PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)] /
                    PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    G4ThreeVector(0.0 * mm, 0.0 * mm, -1.0 * mm);
                zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)] /
                    zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                if (PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)].dot(
                        zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)]) < -0.9999) {
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)].setX(0.0);
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)].setY(1.0);
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)].setZ(0.0);
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)] =
                        myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)] /
                        myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                    angleDelta1P[pentagonIndex][axis1Index + (axis2Index * 4)] = pi;
                } else {
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)] =
                        PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)].cross(
                            zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)]);
                    angleDelta1P[pentagonIndex][axis1Index + (axis2Index * 4)] =
                        PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)].theta(
                            zPosAxisOut[pentagonIndex][axis1Index + (axis2Index * 4)]);
                }
                facetMatrix1[pentagonIndex][axis1Index + (axis2Index * 4)] = G4RotationMatrix(
                    myCross1P[pentagonIndex][axis1Index + (axis2Index * 4)],
                    1.0 * angleDelta1P[pentagonIndex][axis1Index + (axis2Index * 4)]);
                zPosAxis1[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    G4ThreeVector(0.0 * mm, -1.0 * mm, 0.0 * mm);
                zPosAxis1[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    zPosAxis1[pentagonIndex][axis1Index + (axis2Index * 4)] /
                    zPosAxis1[pentagonIndex][axis1Index + (axis2Index * 4)].mag();
                zPosAxis1New[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    facetMatrix1[pentagonIndex][axis1Index + (axis2Index * 4)] *
                    zPosAxis1[pentagonIndex][axis1Index + (axis2Index * 4)];
                angleDelta2P[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)].theta(
                        zPosAxis1New[pentagonIndex][axis1Index + (axis2Index * 4)]);
                G4double orientationSign = PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)].dot(
                    PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)].cross(
                        zPosAxis1New[pentagonIndex][axis1Index + (axis2Index * 4)]));
                if (orientationSign < 0.0) {
                    rotationTwister = 1.0;
                } else {
                    rotationTwister = -1.0;
                }
                dot2P[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)].dot(
                        zPosAxis1New[pentagonIndex][axis1Index + (axis2Index * 4)]);
                if (triggerMan == 0) {
                    if (config.integer("verbose") > 0)
                        std::cout << "DetCon::DetCon: pentagonIndex, angleDelta1P (rad), "
                                     "angleDelta2P (rad), dot2P, orientationSign: "
                                  << pentagonIndex << ", "
                                  << angleDelta1P[pentagonIndex][axis1Index + (axis2Index * 4)]
                                  << ", "
                                  << angleDelta2P[pentagonIndex][axis1Index + (axis2Index * 4)]
                                  << ", " << dot2P[pentagonIndex][axis1Index + (axis2Index * 4)]
                                  << ", " << orientationSign << std::endl;
                    triggerMan = 1;
                }
                facetMatrix2[pentagonIndex][axis1Index + (axis2Index * 4)] = G4RotationMatrix(
                    PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)],
                    rotationTwister * angleDelta2P[pentagonIndex][axis1Index + (axis2Index * 4)]);
                home_Vertex[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    (zero_Vertices[0] -
                     (PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)] * 1.5 * mySipmPitch) -
                     (PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)] * 1.5 * mySipmPitch));
                myRotMatrixPent[pentagonIndex][axis1Index + (axis2Index * 4)] =
                    new G4RotationMatrix(
                        facetMatrix1[pentagonIndex][axis1Index + (axis2Index * 4)] *
                        facetMatrix2[pentagonIndex][axis1Index + (axis2Index * 4)]);
                mHousings.at((pentagonIndex * 16) + (axis1Index + (axis2Index * 4)))
                    ->buildAndPlace(
                        world_PV,
                        G4ThreeVector(
                            ((home_Vertex[pentagonIndex][axis1Index + (axis2Index * 4)] +
                              (PB_axis1[pentagonIndex][axis1Index + (axis2Index * 4)] *
                               mySipmPitch * axis1Index) +
                              (PB_axis2[pentagonIndex][axis1Index + (axis2Index * 4)] *
                               mySipmPitch * axis2Index))) +
                            (PB_CoG[pentagonIndex][axis1Index + (axis2Index * 4)] * sensorCenterR)),
                        myRotMatrixPent[pentagonIndex][axis1Index + (axis2Index * 4)]);
                axis1Index = axis1Index + 1;
            }
            axis1Index = 0;
            axis2Index = axis2Index + 1;
        }
        pentagonIndex = pentagonIndex + 1;
    }
    hexagonIndex = 0;
    G4RotationMatrix* myRotMatrixHex[20][16];
    G4ThreeVector HB_CoG[20][16];
    G4ThreeVector HB_axis1[20][16];
    G4ThreeVector HB_axis2[20][16];
    G4ThreeVector HB_zPosAxisOut[20][16];
    G4ThreeVector myCross1H[20][16];
    G4double angleDelta1H[20][16];
    G4RotationMatrix HB_facetMatrix1[20][16];
    G4ThreeVector HB_zPosAxis1[20][16];
    G4ThreeVector HB_zPosAxis1New[20][16];
    G4double angleDelta2H[20][16];
    G4double dot2H[20][16];
    G4RotationMatrix HB_facetMatrix2[20][16];
    G4ThreeVector HB_home_Vertex[20][16];
    while (hexagonIndex < hexagonCount) {
        triggerMan = 0;
        axis1Index = 0;
        axis2Index = 0;
        while (axis2Index < 4) {
            while (axis1Index < 4) {
                HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    ((chapeau_Vertices[hexagonVertexIndices[hexagonIndex][0]] * chapeau_scale) +
                     (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale)) /
                    2.0;
                HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][2]] * chapeau_scale) -
                    HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)];
                HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    (((chapeau_Vertices[hexagonVertexIndices[hexagonIndex][3]] * chapeau_scale) +
                      (chapeau_Vertices[hexagonVertexIndices[hexagonIndex][4]] * chapeau_scale)) /
                     2.0) -
                    HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)];
                HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)] /
                    HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)] /
                    HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)] /
                    HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    G4ThreeVector(0.0 * mm, 0.0 * mm, -1.0 * mm);
                HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)] /
                    HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                if (HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)].dot(
                        HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)]) < -0.9999) {
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)].setX(0.0);
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)].setY(1.0);
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)].setZ(0.0);
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)] =
                        myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)] /
                        myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                    angleDelta1H[hexagonIndex][axis1Index + (axis2Index * 4)] = pi;
                } else {
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)] =
                        HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)].cross(
                            HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)]);
                    angleDelta1H[hexagonIndex][axis1Index + (axis2Index * 4)] =
                        HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)].theta(
                            HB_zPosAxisOut[hexagonIndex][axis1Index + (axis2Index * 4)]);
                }
                HB_facetMatrix1[hexagonIndex][axis1Index + (axis2Index * 4)] = G4RotationMatrix(
                    myCross1H[hexagonIndex][axis1Index + (axis2Index * 4)],
                    1.0 * angleDelta1H[hexagonIndex][axis1Index + (axis2Index * 4)]);
                HB_zPosAxis1[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    G4ThreeVector(0.0 * mm, -1.0 * mm, 0.0 * mm);
                HB_zPosAxis1[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_zPosAxis1[hexagonIndex][axis1Index + (axis2Index * 4)] /
                    HB_zPosAxis1[hexagonIndex][axis1Index + (axis2Index * 4)].mag();
                HB_zPosAxis1New[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_facetMatrix1[hexagonIndex][axis1Index + (axis2Index * 4)] *
                    HB_zPosAxis1[hexagonIndex][axis1Index + (axis2Index * 4)];
                angleDelta2H[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)].theta(
                        HB_zPosAxis1New[hexagonIndex][axis1Index + (axis2Index * 4)]);
                G4double orientationSign = HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)].dot(
                    HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)].cross(
                        HB_zPosAxis1New[hexagonIndex][axis1Index + (axis2Index * 4)]));
                if (hexagonIndex < 10) {
                    if (orientationSign < 0.0) {
                        rotationTwister = -1.0;
                    } else {
                        rotationTwister = 1.0;
                    }
                } else {
                    if (orientationSign < 0.0) {
                        rotationTwister = 1.0;
                    } else {
                        rotationTwister = -1.0;
                    }
                }
                dot2H[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)].dot(
                        HB_zPosAxis1New[hexagonIndex][axis1Index + (axis2Index * 4)]);
                if (triggerMan == 0) {
                    if (config.integer("verbose") > 0)
                        std::cout
                            << "DetCon::DetCon: hexagonIndex, angleDelta1H (rad), angleDelta2H "
                               "(rad), dot2H, orientationSign: "
                            << hexagonIndex << ", "
                            << angleDelta1H[hexagonIndex][axis1Index + (axis2Index * 4)] << ", "
                            << angleDelta2H[hexagonIndex][axis1Index + (axis2Index * 4)] << ", "
                            << dot2H[hexagonIndex][axis1Index + (axis2Index * 4)] << ", "
                            << orientationSign << std::endl;
                    triggerMan = 1;
                }
                HB_facetMatrix2[hexagonIndex][axis1Index + (axis2Index * 4)] = G4RotationMatrix(
                    HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)],
                    rotationTwister * angleDelta2H[hexagonIndex][axis1Index + (axis2Index * 4)]);
                HB_home_Vertex[hexagonIndex][axis1Index + (axis2Index * 4)] =
                    (zero_Vertices[0] -
                     (HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)] * 1.5 * mySipmPitch) -
                     (HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)] * 1.5 * mySipmPitch));
                myRotMatrixHex[hexagonIndex][axis1Index + (axis2Index * 4)] = new G4RotationMatrix(
                    HB_facetMatrix1[hexagonIndex][axis1Index + (axis2Index * 4)] *
                    HB_facetMatrix2[hexagonIndex][axis1Index + (axis2Index * 4)]);
                mHousings.at((12 * 16) + (hexagonIndex * 16) + (axis1Index + (axis2Index * 4)))
                    ->buildAndPlace(
                        world_PV,
                        G4ThreeVector(
                            ((HB_home_Vertex[hexagonIndex][axis1Index + (axis2Index * 4)] +
                              (HB_axis1[hexagonIndex][axis1Index + (axis2Index * 4)] * mySipmPitch *
                               axis1Index) +
                              (HB_axis2[hexagonIndex][axis1Index + (axis2Index * 4)] * mySipmPitch *
                               axis2Index))) +
                            (HB_CoG[hexagonIndex][axis1Index + (axis2Index * 4)] * sensorCenterR)),
                        myRotMatrixHex[hexagonIndex][axis1Index + (axis2Index * 4)]);
                axis1Index = axis1Index + 1;
            }
            axis1Index = 0;
            axis2Index = axis2Index + 1;
        }
        hexagonIndex = hexagonIndex + 1;
    }
    return world_PV;
}
G4SipmModel* DetectorConstruction::getSipmModel() const { return housing->getSipm()->getModel(); }
G4SipmHousing* DetectorConstruction::getSipmHousing() const { return housing; }
