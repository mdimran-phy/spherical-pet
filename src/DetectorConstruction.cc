/*
 * DetectorConstruction.cc
 *
 * @date Sep 12, 2011
 * @author Tim Niggemann, III Phys. Inst. A, RWTH Aachen University
 * @copyright GNU General Public License v3.0
 */

#include "DetectorConstruction.hh"
#include <stdexcept>
#include <set>

#include <G4Box.hh>
#include <G4VisAttributes.hh>
#include <G4PVPlacement.hh>

#include "G4Sipm.hh"
#include "MaterialFactory.hh"
#include "model/G4SipmModelFactory.hh"
#include "housing/G4SipmHousing.hh"
#include "housing/impl/HamamatsuCeramicHousing.hh"
#include "housing/impl/HamamatsuSmdHousing.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

DetectorConstruction::DetectorConstruction(const Config& settings) :
  G4VUserDetectorConstruction(), config(settings) {
  const std::string sipmModelName=config.text("sipm-model");
  const std::string housingName="default";
  const std::set<std::string> supported={"generic", "HamamatsuS1036211100", "HamamatsuS1036233100", "HamamatsuS10985100", "HamamatsuS12651050", "HamamatsuS1036233050", "HamamatsuS12573100C", "HamamatsuS12573100X"};
  if (!supported.count(sipmModelName)) throw std::runtime_error("Unavailable SiPM model: "+sipmModelName);


  if (config.integer("verbose") > 0) std::cout << "DetectorConstruction::DetectorConstruction: " << sipmModelName << ", " << housingName << std::endl;

  // Create SiPM and housing.
  G4SipmModel* model = createSipmModel(sipmModelName);
	
  /*
    housing  = createHousing(housingName, new G4Sipm(model));
    housing2 = createHousing(housingName, new G4Sipm(model));
    housing3 = createHousing(housingName, new G4Sipm(model));
    housing4 = createHousing(housingName, new G4Sipm(model));

    housing5  = createHousing(housingName, new G4Sipm(model));
    housing6 = createHousing(housingName, new G4Sipm(model));
    housing7 = createHousing(housingName, new G4Sipm(model));
    housing8 = createHousing(housingName, new G4Sipm(model));

    housing9 = createHousing(housingName, new G4Sipm(model));
    housing10 = createHousing(housingName, new G4Sipm(model));
    housing11 = createHousing(housingName, new G4Sipm(model));
    housing12 = createHousing(housingName, new G4Sipm(model));

    housing13 = createHousing(housingName, new G4Sipm(model));
    housing14 = createHousing(housingName, new G4Sipm(model));
    housing15 = createHousing(housingName, new G4Sipm(model));
    housing16 = createHousing(housingName, new G4Sipm(model));
  */
  //  Dynamic version
  int housingIndex = 0;
  //int facetCount = 14;

  int housingCount = ( 4 * 4 * 12 ) + ( 4 * 4 * 20 );  //  HAMA
  //int housingCount = 4 * 4 * 6;  //  HAMA

  //int housingCount = 8 * 8 * 6;  //  KETEK
  //int housingCount = 9 * 9 * 3;
  //int housingCount = 9 * 9;
  //int housingCount = 4 * 4;
  //int facetCount = 14;
  //int housingCount = 16;


  while ( housingIndex < housingCount ) {

    G4SipmHousing* housingScratch = createHousing(housingName, new G4Sipm(model));

    mHousings.push_back(housingScratch); 		

    housingIndex = housingIndex + 1;

  }

  housing = mHousings.at( 0 );
  const double pitch=config.number("sipm-pitch-mm")*mm;
  if (housing->getDx()>pitch || housing->getDy()>pitch ||
      3*pitch+housing->getDx()>18*mm || 3*pitch+housing->getDy()>18*mm)
    throw std::runtime_error("SiPM package does not fit the requested pitch or 18 mm guide outlet");
  auto* epoxy=MaterialFactory::getInstance()->getEpoxy()->GetMaterialPropertiesTable();
  double energies[]={1.0*eV, 6.0*eV};
  double indices[]={config.number("epoxy-index"),config.number("epoxy-index")};
  epoxy->RemoveProperty("RINDEX");
  epoxy->AddProperty("RINDEX",energies,indices,2);
  if(config.number("epoxy-absorption-mm")>0){
    double lengths[]={config.number("epoxy-absorption-mm")*mm,config.number("epoxy-absorption-mm")*mm};
    epoxy->AddProperty("ABSLENGTH",energies,lengths,2);
  }


}

DetectorConstruction::~DetectorConstruction() {

  /*
    delete housing;
    delete housing2;
    delete housing3;
    delete housing4;

    delete housing5;
    delete housing6;
    delete housing7;
    delete housing8;

    delete housing9;
    delete housing10;
    delete housing11;
    delete housing12;

    delete housing13;
    delete housing14;
    delete housing15;
    delete housing16;
  */
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
  if (name == "HamamatsuS141604050HS") {
    G4Exception("createSipmModel", "PET_MODEL_MISSING", FatalException, "The custom S141604050HS model was not supplied. Select an available model explicitly.");
    return nullptr;
  }
  return G4SipmModelFactory::getInstance()->createConfigFileModel(name);
}

G4SipmHousing* DetectorConstruction::createHousing(std::string name, G4Sipm* sipm) const {

  if (name == "ceramic") {
    return new HamamatsuCeramicHousing(sipm);
  }
  if (name == "smd") {
    return new HamamatsuSmdHousing(sipm);
  }
  if (name == "default") {
    return new G4SipmHousing(sipm);
  }
  if (config.integer("verbose") > 0) std::cout << "G4SipmDetectorConstruction::createHousingForName(name = \"" << name
	    << "\"): housing type does not exist." << std::endl;
  throw 1;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  G4ThreeVector worldDimension = G4ThreeVector( 0.1 * CLHEP::m, 0.1 * CLHEP::m, 0.1 * CLHEP::m);
  //G4ThreeVector worldDimension = G4ThreeVector(1. * CLHEP::m, 1. * CLHEP::m, 1. * CLHEP::m);
  // Create world volume.
  G4Box* world = new G4Box("world", worldDimension.x(), worldDimension.y(), worldDimension.z());
  //G4LogicalVolume* world_LV = new G4LogicalVolume(world, MaterialFactory::getInstance()->getWater(), "world_LV", NULL,
  //NULL, NULL);
  //G4LogicalVolume* world_LV = new G4LogicalVolume(world, MaterialFactory::getInstance()->getEpoxy(), "world_LV", NULL,
  G4LogicalVolume* world_LV = new G4LogicalVolume(world, MaterialFactory::getInstance()->getAir(), "world_LV", NULL,
						 NULL, NULL);
  world_LV->SetVisAttributes(new G4VisAttributes(G4Color(255, 255, 255, 0.1)));
  //world_LV->SetVisAttributes(G4VisAttributes::Invisible);
  G4VPhysicalVolume* world_PV = new G4PVPlacement(NULL, G4ThreeVector(), world_LV, "world_PV", NULL, false, 0);

  //  PFM-NOTE: Where the SiPM array construction used to be...



  //  Create the chunk of LYSO and couple it to the SiPM array
  //  First we need the elements...
  G4double a;  // atomic mass
  G4double z;  // atomic number
  G4double density;

  //G4double scint_outer_radius = 34.5 * mm;
  //G4double scint_outer_radius = 50.0 * mm;
  //G4double pentagon_B_spacing = 0.0 * mm;
  //G4double pentagon_B_spacing = 5.0 * mm;

  G4double toleranceTwiddle = 0.01;

  fLu = new G4Element("Lutetium", "Lu", z=71., a=174.97*g/mole);
  fY = new G4Element("Yttrium", "Y", z=39., a=88.91*g/mole);
  fSi = new G4Element("Silicon", "Si", z=14., a=28.09*g/mole);
  fO = new G4Element("O"  , "O", z=8., a= 16.00*g/mole);

  //fLYSO = new G4Material( "LYSO", density=10000.0*g/cm3, 4 );
  //fLYSO = new G4Material( "LYSO", density=28.0*g/cm3, 4 );
  fLYSO = new G4Material( "LYSO", density=7.36*g/cm3, 4 );
  G4double fractionLu_LYSO = 0.714467891;
  fLYSO->AddElement( fLu, fractionLu_LYSO );
  G4double fractionY_LYSO = 0.04033805;
  fLYSO->AddElement( fY, fractionY_LYSO );
  G4double fractionSi_LYSO = 0.063714272;
  fLYSO->AddElement( fSi, fractionSi_LYSO );
  G4double fractionO_LYSO = 0.181479788;
  fLYSO->AddElement( fO, fractionO_LYSO );

  //  Add the LYSO material properties.
  auto spectrum=config.spectrum();
  std::vector<G4double> energies,intensities,indices,lengths;
  for(const auto& point:spectrum){
    energies.push_back(point.first*eV);intensities.push_back(point.second);
    indices.push_back(config.number("lyso-index"));
    lengths.push_back(config.number("lyso-absorption-mm")*mm);
  }
  fLYSO_mt = new G4MaterialPropertiesTable();
  fLYSO_mt->AddProperty("SCINTILLATIONCOMPONENT1",energies.data(),intensities.data(),energies.size());
  fLYSO_mt->AddProperty("RINDEX",energies.data(),indices.data(),energies.size());
  fLYSO_mt->AddProperty("ABSLENGTH",energies.data(),lengths.data(),energies.size());
  fLYSO_mt->AddConstProperty("SCINTILLATIONYIELD",config.number("lyso-yield")/keV);
  fLYSO_mt->AddConstProperty("RESOLUTIONSCALE",1.0);
  fLYSO_mt->AddConstProperty("SCINTILLATIONTIMECONSTANT1",config.number("lyso-decay-ns")*ns);
  fLYSO_mt->AddConstProperty("SCINTILLATIONYIELD1",1.0);
  fLYSO->SetMaterialPropertiesTable(fLYSO_mt);

  // Set the Birks Constant for the LYSO scintillator
  // PFM-NOTE: Commenting this out after discussion w/Sasha, is probably harmless for what we're doing
  //fLYSO->GetIonisation()->SetBirksConstant(0.126*mm/MeV);

  //double flooper = 15.91352;
  //double flooper = 80.0;

  //  Now that Geant4 knows what "LYSO" is, create a logical volume to tell it what we want
  //  Make the geometry, make the logical volume, then place it 

  /*
    fTestScint_box = new G4Box("testScint_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
    fTestScint_logicalVolume = new G4LogicalVolume( fTestScint_box, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );
    fTestScint_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , fTestScint_logicalVolume, "scintillator", world_LV, false, 0 );
    //fTestScint_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, (( flooper * 0.5 ) + 0.25 )*mm) , fTestScint_logicalVolume, "scintillator", world_LV, false, 0 );
    //new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, (( flooper * 0.5 ) + 1.0 )*mm) , fTestScint_logicalVolume, "scintillator", world_LV, false, 0 );
    */
  //  The annulus geometry
  //  PFM-NOTE: This may be overly-complicated, it may be simpler to do CSG subtraction than addition but this works so leave it as-is for now.
  //  PFM-NOTE: Use the X when you need the measurement of the maximum external radius of the annulus.
  //  Use the Y when you need the minimum.  It sounds weird but they both have applications, usually to accommodate what Geant wants/assumes.
  //  PFM-P2: I need to rename these to radiusOuterMax and radiusOuterMin to reduce ambiguity, the current variable names are misleading. 
  //G4double scintillatorX = 8.2*cm;  //  "fScint_x"
  G4double scintillatorY = 8.0*cm;  //  "fScint_y"
  //G4double scintillatorY = 7.994*cm;  //  "fScint_y"
  G4double scintillatorZ = 7.2*cm;  //  "fScint_z"
  G4double scintillatorHoleDiameter = 6.0*cm;

  G4double radiusInner = scintillatorHoleDiameter * 0.5;
  G4double radiusOuter = scintillatorY * 0.5; 
  G4double radiusOverlap =  ( radiusOuter - radiusInner ) * 0.25;	
  G4double halfZ = scintillatorZ / 2.0;

  const G4double m_zPlane[ 2 ] = { -1.0 * halfZ, 1.0 * halfZ };
  const G4double m_radiusOuter[ 2 ] = { radiusOuter, radiusOuter };
  const G4double m_radiusInner[ 2 ] = { radiusInner + radiusOverlap, radiusInner + radiusOverlap };

  scint_poly = new G4Polyhedra( "scint_outer", 0.0*M_PI, 2.0*M_PI, 14, 2, m_zPlane, m_radiusInner, m_radiusOuter );

  scint_tube = new G4Tubs( "scint_inner", radiusInner, radiusOuter - radiusOverlap,
			   halfZ, 0.0 * deg, 360.0 * deg);
        
  union_solid_scint = new G4UnionSolid( "scint_box", scint_tube, scint_poly );

  G4double shellScale = 0.69;
  G4double hemisphereThickness = config.number("shell-thickness-mm")*mm;
  G4double hemisphereRadiusOuter = 5.0*cm;
  //G4double hemisphereRadiusInner = 4.0*cm;
  G4double hemisphereRadiusOuterScaled = hemisphereRadiusOuter * shellScale;
  G4double hemisphereRadiusInner = hemisphereRadiusOuterScaled - hemisphereThickness;

  scint_hemisphere_virgin = new G4Sphere( "scint_hemisphere_virgin", hemisphereRadiusInner, hemisphereRadiusOuterScaled, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI );  
 
  //  PFM-NOTE: This is for a hemisphere so to get the total gasket thickness for the sphere it needs to be doubled
  //  Per the 2024-Mar-27 group meeting notes the new target thickness for the entire gasket is 0.1mm, which means each half is 0.05mm 
  G4double gasketThickness = 0.05 * mm;
  //G4double gasketThickness = 0.5 * mm;
  //G4double gasketThickness = 5.0 * mm;

  G4ThreeVector scintTrans( 0 * mm, 0 * mm, ( hemisphereRadiusOuterScaled / 2.0) + gasketThickness );
  G4ThreeVector gasketTrans( 0 * mm, 0 * mm, -1.0 * ( hemisphereRadiusOuterScaled / 2.0) + gasketThickness );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, ( hemisphereRadiusOuterScaled / 2.0) + gasketThickness );
  //G4ThreeVector scintTrans( 0 * mm, 0 * mm, -1.0 * ( hemisphereRadiusOuterScaled / 2.0) + gasketThickness );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, ( hemisphereRadiusOuterScaled / 2.0) + ( 0.5 * mm ));
  //G4ThreeVector scintTrans( 0 * mm, 0 * mm, -1.0 * ( hemisphereRadiusOuterScaled / 2.0) - ( 0.5 * mm ));
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, ( hemisphereRadiusOuterScaled / 2.0) + ( 5.0 * mm ));
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, -5.5 * mm );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, -9.5 * mm );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, 19.5 * mm );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, 19.0 * mm );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, 15.0 * mm );
  //G4ThreeVector gasketTrans( 0 * mm, 0 * mm, 0.0 * mm );
  G4RotationMatrix *gasketIdentityMatrix = new G4RotationMatrix;
  gasketBox = new G4Box("gasketBox", hemisphereRadiusOuterScaled, hemisphereRadiusOuterScaled, hemisphereRadiusOuterScaled / 2.0 );
  gasketBox_LV = new G4LogicalVolume( gasketBox, MaterialFactory::getInstance()->getEpoxy(), "gasketBox_LV", 0, 0, 0 );
  //gasketBox_PV = new G4PVPlacement( gasketIdentityMatrix, gasketTrans, gasketBox_LV, "gasketBox_PV", world_LV, false, 0 );

  gasketBox2 = new G4Box("gasketBox2", hemisphereRadiusOuterScaled, hemisphereRadiusOuterScaled, hemisphereRadiusOuterScaled / 2.0 );
  gasketBox2_LV = new G4LogicalVolume( gasketBox2, MaterialFactory::getInstance()->getEpoxy(), "gasketBox2_LV", 0, 0, 0 );
  //gasketBox2_PV = new G4PVPlacement( gasketIdentityMatrix, scintTrans, gasketBox2_LV, "gasketBox2_PV", world_LV, false, 0 );

  //gasketBox = new G4Box("gasketBox", 40.0 * mm, 40.0 * mm, 20.0 * mm );
  //gasketBox = new G4Box("gasketBox", 20.0 * mm, 40.0 * mm, 20.0 * mm );
  //gasketBox = new G4Box("gasketBox", 20.0 * mm, 20.0 * mm, 20.0 * mm );
  //gasketBox = new G4Box("gasketBox", 40.0 * mm, 40.0 * mm, 40.0 * mm );

  //  gasketSubtraction 

  scint_hemisphere = new G4Sphere( "scint_et_al", hemisphereRadiusInner, hemisphereRadiusOuterScaled, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI );

  hemisphereNorthScint = new G4SubtractionSolid( "hemisphereNorthScint", scint_hemisphere_virgin, gasketBox, gasketIdentityMatrix, ( gasketTrans * -1.0 )); 
  hemisphereNorthGasket = new G4SubtractionSolid( "hemisphereNorthGasket", scint_hemisphere_virgin, gasketBox, gasketIdentityMatrix, ( scintTrans * -1.0 )); 
  hemisphereSouthScint = new G4SubtractionSolid( "hemisphereSouthScint", scint_hemisphere_virgin, gasketBox, gasketIdentityMatrix, ( gasketTrans * -1.0 )); 
  hemisphereSouthGasket = new G4SubtractionSolid( "hemisphereSouthGasket", scint_hemisphere_virgin, gasketBox, gasketIdentityMatrix, ( scintTrans * -1.0 )); 

  //hemisphereNorthScint = new G4SubtractionSolid( "hemisphereNorthScint", scint_hemisphere_virgin, gasketBox ); 
  //hemisphereNorthScint = new G4SubtractionSolid( "hemisphereNorthScint", scint_hemisphere_virgin, scint_hemisphere ); 

  //scint_hemisphere = new G4Sphere( "scint_et_al", hemisphereRadiusInner, hemisphereRadiusOuterScaled, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI );  //  This is the last true hemisphere, the one used for A2.  B1 needs the gasket so the hemisphere has to be "ground down" to be flush with the gasket and mate up to the lightguide 
  //scint_hemisphere = new G4Sphere( "scint_et_al", hemisphereRadiusInner, hemisphereRadiusOuter, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI );
  //scint_hemisphere = new G4Sphere( "scint_et_al", hemisphereRadiusInner, hemisphereRadiusOuter, 0.0 * M_PI, 1.0 * M_PI, 0.0 * M_PI, 2.0 * M_PI );
  
  //fTestScint_logicalVolume = new G4LogicalVolume( scint_poly, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );
  //fTestScint_logicalVolume = new G4LogicalVolume( union_solid_scint, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );
  
  //  PFM-NOTE: Box tinkering to get the CSG right
/*
  tinkerBox1 = new G4Box("tinker1_box", 20.0 * mm, 20.0 * mm, 20.0 * mm );
  tinkerBox2 = new G4Box("tinker2_box", 10.0 * mm, 20.0 * mm, 10.0 * mm );
  tinkerBox3 = new G4Box("tinker3_box", 20.0 * mm, 10.0 * mm, 10.0 * mm );

  tinkerBox1MinusTinkerBox2 = new G4SubtractionSolid( "Box1-Box2", tinkerBox1, tinkerBox2 ); 
  tinkerBox1MinusTinkerBox2MinusTinkerBox3 = new G4SubtractionSolid( "Box1-Box2-Box3", tinkerBox1MinusTinkerBox2, tinkerBox3 ); 

  tinkerBox1MinusTinkerBox2_logicalVolume = new G4LogicalVolume( tinkerBox1MinusTinkerBox2MinusTinkerBox3, MaterialFactory::getInstance()->getEpoxy(), "tinkerBox1MinusTinkerBox2_logicalVolume", 0, 0, 0 );
  tinkerBox1MinusTinkerBox2_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , tinkerBox1MinusTinkerBox2_logicalVolume, "tinkerBox1MinusTinkerBox2_physicalVolume", world_LV, false, 0 );

  //tinkerBox1MinusTinkerBox2_logicalVolume = new G4LogicalVolume( tinkerBox1MinusTinkerBox2, MaterialFactory::getInstance()->getEpoxy(), "tinkerBox1MinusTinkerBox2_logicalVolume", 0, 0, 0 );
  //tinkerBox1MinusTinkerBox2_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , tinkerBox1MinusTinkerBox2_logicalVolume, "tinkerBox1MinusTinkerBox2_physicalVolume", world_LV, false, 0 );
*/
  //  Okay, now for real...
  //hackGeo1 = new G4Box( "hackGeo1", 0.5 * mm, 0.5 * mm, 30.0 * mm );

  //  PFM-NOTE: This is the real one
  
  //hackGeo1 = new G4Box( "hackGeo1", 0.5 * mm, 0.5 * mm, 2.0 * mm );
  waterTubs = new G4Tubs( "waterTubs", 0.0 * mm, 0.75 * mm, 2.5 * mm, 0.0 * deg, 360.0 * deg);

  //tinkerBox1MinusTinkerBox2_logicalVolume = new G4LogicalVolume( hackGeo1, MaterialFactory::getInstance()->getEpoxy(), "tinkerBox1MinusTinkerBox2_logicalVolume", 0, 0, 0 );
  //tinkerBox1MinusTinkerBox2_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , tinkerBox1MinusTinkerBox2_logicalVolume, "tinkerBox1MinusTinkerBox2_physicalVolume", world_LV, false, 0 );

  G4ThreeVector zTrans( 0 * mm, 0 * mm, ( -29.5 + 2.5 ) * mm );
  //G4ThreeVector zTrans( 0 * mm, 0 * mm, ( -29.5 + 2.5 - 0.5 ) * mm );
  G4RotationMatrix *identityMatrix = new G4RotationMatrix;

  hemisphereMinusHack = new G4SubtractionSolid( "hemisphereMinusHack", scint_hemisphere, waterTubs, identityMatrix, zTrans ); 
  //hemisphereMinusHack = new G4SubtractionSolid( "hemisphereMinusHack", scint_hemisphere, hackGeo1, identityMatrix, zTrans ); 
  //hemisphereMinusHack = new G4SubtractionSolid( "hemisphereMinusHack", scint_hemisphere, hackGeo1 ); 

  //fTestScint_LV = new G4LogicalVolume( hemisphereMinusHack, MaterialFactory::getInstance()->getEpoxy(), "fTestScint_LV", 0, 0, 0 );
  //fTestScint_logicalVolume = new G4LogicalVolume( hemisphereMinusHack, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );
  //fTestScint_LV = new G4LogicalVolume( hemisphereMinusHack, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );
  fTestScint_LV = new G4LogicalVolume( scint_hemisphere, G4Material::GetMaterial("LYSO"), "testScint_logicalVolume", 0, 0, 0 );

  hemisphereNorthScint_LV = new G4LogicalVolume( hemisphereNorthScint, G4Material::GetMaterial("LYSO"), "hemisphereNorthScint_LV", 0, 0, 0 );
  hemisphereNorthGasket_LV = new G4LogicalVolume( hemisphereNorthGasket, MaterialFactory::getInstance()->getEpoxy(), "hemisphereNorthGasket_LV", 0, 0, 0 );
  hemisphereSouthScint_LV = new G4LogicalVolume( hemisphereSouthScint, G4Material::GetMaterial("LYSO"), "hemisphereSouthScint_LV", 0, 0, 0 );
  hemisphereSouthGasket_LV = new G4LogicalVolume( hemisphereSouthGasket, MaterialFactory::getInstance()->getEpoxy(), "hemisphereSouthGasket_LV", 0, 0, 0 );

  //fTestScint_logicalVolume->SetVisAttributes(G4VisAttributes::Invisible);
 
  G4Colour white = G4Color::White();
  fTestScint_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
  G4Colour yellow = G4Color::Yellow();

  hemisphereNorthScint_LV->SetVisAttributes(new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 1.0 )));
  hemisphereNorthGasket_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 1.0 )));
  hemisphereSouthScint_LV->SetVisAttributes(new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 1.0 )));
  hemisphereSouthGasket_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 1.0 )));

  //hemisphereNorthScint_LV->SetVisAttributes(new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 0.5)));
  //hemisphereNorthGasket_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));

  G4RotationMatrix* rm_x1 = new G4RotationMatrix();

  //rm_x1->rotateZ( 0 * deg );	
  //rm_x1->rotateZ( 16.5 * deg );	
  //rm_x1->rotateZ( 37.4 * deg );	
  //rm_x1->rotateZ( 8.0 * deg );	

  rm_x1->rotateX( 180.0 * deg );	
  //rm_x1->rotateX( 90.0 * deg );	
  //rm_x1->rotateX( 90.0 * deg );	
  //rm_x1->rotateX( 45.0 * deg );	

  //  PFM-NOTE: Bring this line back to bring the scintillator back
  //fTestScint_PV = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , fTestScint_LV, "fTestScint_PV", world_LV, false, 0 );

  //  Debugging re-enable me please
  hemisphereNorthScint_PV = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hemisphereNorthScint_LV, "hemisphereNorthScint_PV", world_LV, false, 0 );
  hemisphereNorthGasket_PV = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hemisphereNorthGasket_LV, "hemisphereNorthGasket_PV", world_LV, false, 0 );

  hemisphereSouthScint_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hemisphereSouthScint_LV, "hemisphereSouthScint_PV", world_LV, false, 0 );
  hemisphereSouthGasket_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hemisphereSouthGasket_LV, "hemisphereSouthGasket_PV", world_LV, false, 0 );

  //fTestScint_PV = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , fTestScint_LV, "fTestScint_PV", world_LV, false, 0 );

  hackScint1 = new G4Box( "hackScint1", 0.5 * mm, 0.5 * mm, 2.0 * mm );
  //hackScint1 = new G4Box( "hackScint1", 0.5 * mm, 2.0 * mm, 0.5 * mm );
  //hackScint1_logicalVolume = new G4LogicalVolume( hackScint1, MaterialFactory::getInstance()->getEpoxy(), "hackScint1_logicalVolume", 0, 0, 0 );
  hackScint1of4 = new G4Box( "hackScint1of4", 0.5 * mm, 0.5 * mm, 0.5 * mm );
  hackScint2of4 = new G4Box( "hackScint2of4", 0.5 * mm, 0.5 * mm, 0.5 * mm );
  hackScint3of4 = new G4Box( "hackScint3of4", 0.5 * mm, 0.5 * mm, 0.5 * mm );
  hackScint4of4 = new G4Box( "hackScint4of4", 0.5 * mm, 0.5 * mm, 0.5 * mm );


  //  New water column LV/PV
  G4RotationMatrix *waterTubsIdentityMatrix = new G4RotationMatrix;
  //waterTubsIdentityMatrix->rotateZ( 45.0 * deg );	
  G4ThreeVector waterTubsTrans( 0.0 * mm, 0.0 * mm, 0.5 * mm );
  waterTubsMinusScintBox = new G4SubtractionSolid( "waterTubsMinusScintBox", waterTubs, hackScint1, waterTubsIdentityMatrix, waterTubsTrans ); 
  //waterTubsMinusScintBox_LV = new G4LogicalVolume( waterTubsMinusScintBox, MaterialFactory::getInstance()->getEpoxy(), "waterTubsMinusScintBox_LV", 0, 0, 0 );
  waterTubsMinusScintBox_LV = new G4LogicalVolume( waterTubsMinusScintBox, MaterialFactory::getInstance()->getWater(), "waterTubsMinusScintBox_LV", 0, 0, 0 );
  G4RotationMatrix *waterTubsMinusScintBoxRotationMatrix = new G4RotationMatrix;
  G4ThreeVector tubHack = G4ThreeVector( 0.0 * mm, ( -29.5 + 2.5 ) * mm, 0.0 * mm ); 
  //tubHack.rotate( 37.4 * deg, G4ThreeVector( 0, 0, -1 ));
  //waterTubsMinusScintBoxRotationMatrix->rotateZ( 37.4 * deg );	
  waterTubsMinusScintBoxRotationMatrix->rotateX( -90.0 * deg );	

  //  PFM-CHIP 
  //waterTubsMinusScintBox_PV = new G4PVPlacement( waterTubsMinusScintBoxRotationMatrix, tubHack, waterTubsMinusScintBox_LV, "waterTubsMinusScintBox_PV", world_LV, false, 0 );
  //waterTubsMinusScintBox_PV = new G4PVPlacement( waterTubsMinusScintBoxRotationMatrix, G4ThreeVector( 0.0*mm, ( -29.5 + 2.5 ) * mm, 0.0*mm) , waterTubsMinusScintBox_LV, "waterTubsMinusScintBox_PV", world_LV, false, 0 );
  //waterTubsMinusScintBox_PV = new G4PVPlacement( waterTubsMinusScintBoxRotationMatrix, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , waterTubsMinusScintBox_LV, "waterTubsMinusScintBox_PV", world_LV, false, 0 );
  //waterTubsMinusScintBox_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , waterTubsMinusScintBox_LV, "waterTubsMinusScintBox_PV", world_LV, false, 0 );


  hackScint1_LV = new G4LogicalVolume( hackScint1, G4Material::GetMaterial("LYSO"), "hackScint1_LV", 0, 0, 0 );
  //hackScint1_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0 * mm, ( 29.5 + 2.0 ) * mm, 0.0 * mm ), hackScint1_logicalVolume, "hackScint1_physicalVolume", world_LV, false, 0 ); 
  hackScint1of4_LV = new G4LogicalVolume( hackScint1of4, G4Material::GetMaterial("LYSO"), "hackScint1of4_LV", 0, 0, 0 );
  hackScint2of4_LV = new G4LogicalVolume( hackScint2of4, G4Material::GetMaterial("LYSO"), "hackScint2of4_LV", 0, 0, 0 );
  hackScint3of4_LV = new G4LogicalVolume( hackScint3of4, G4Material::GetMaterial("LYSO"), "hackScint3of4_LV", 0, 0, 0 );
  hackScint4of4_LV = new G4LogicalVolume( hackScint4of4, G4Material::GetMaterial("LYSO"), "hackScint4of4_LV", 0, 0, 0 );

  G4RotationMatrix* rm_boink1 = new G4RotationMatrix();
  //rm_boink1->rotateZ( 37.4 * deg );	

  rm_boink1->rotateX( 90.0 * deg );	
  //rm_boink1->rotateZ( 45.0 * deg );	

  //rm_boink1->rotateZ( 0 * deg );	
  //rm_boink1->rotateZ( 16.5 * deg );	

  G4ThreeVector myVecHack = G4ThreeVector( 0.0 * mm, ( -29.5 + 2.0 ) * mm, 0.0 * mm ); 

  G4ThreeVector myVecHack1of4 = G4ThreeVector( 0.0 * mm, ( -29.5 + 0.5 ) * mm, 0.0 * mm ); 
  G4ThreeVector myVecHack2of4 = G4ThreeVector( 0.0 * mm, ( -29.5 + 1.5 ) * mm, 0.0 * mm ); 
  G4ThreeVector myVecHack3of4 = G4ThreeVector( 0.0 * mm, ( -29.5 + 2.5 ) * mm, 0.0 * mm ); 
  G4ThreeVector myVecHack4of4 = G4ThreeVector( 0.0 * mm, ( -29.5 + 3.5 ) * mm, 0.0 * mm ); 

  myVecHack.rotate( 0.0 * deg, G4ThreeVector( 0, 0, -1 ));
  //myVecHack.rotate( 37.4 * deg, G4ThreeVector( 0, 0, -1 ));
  //myVecHack.rotate( 16.5 * deg, G4ThreeVector( 0, 0, -1 ));

  //  PFM-NOTE: re-enable the real scint with this line 
  //hackScint1of4_PV = new G4PVPlacement( rm_boink1, myVecHack1of4, hackScint1of4_LV, "hackScint1of4_PV", world_LV, false, 0 ); 
  //hackScint2of4_PV = new G4PVPlacement( rm_boink1, myVecHack2of4, hackScint2of4_LV, "hackScint2of4_PV", world_LV, false, 0 ); 
  //hackScint3of4_PV = new G4PVPlacement( rm_boink1, myVecHack3of4, hackScint3of4_LV, "hackScint3of4_PV", world_LV, false, 0 ); 
  //hackScint4of4_PV = new G4PVPlacement( rm_boink1, myVecHack4of4, hackScint4of4_LV, "hackScint4of4_PV", world_LV, false, 0 ); 

  //hackScint1_PV = new G4PVPlacement( rm_boink1, myVecHack, hackScint1_LV, "hackScint1_PV", world_LV, false, 0 ); 
  //hackScint1_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0 * mm, ( -29.5 + 2.0 ) * mm, 0.0 * mm ), hackScint1_LV, "hackScint1_PV", world_LV, false, 0 ); 

  innermost_hemisphere = new G4Sphere( "innermost_hemisphere", hemisphereRadiusInner - ( 1.0 * mm ), hemisphereRadiusInner, 0.0 * M_PI, 2.0 * M_PI, 0.5 * M_PI, 0.5 * M_PI );
  innermost_hemisphere_LV = new G4LogicalVolume( innermost_hemisphere, MaterialFactory::getInstance()->getEpoxy(), "innermost_hemisphere_LV", 0, 0, 0 );
  innermost_hemisphere_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
  innermost_hemisphere_PV = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , innermost_hemisphere_LV, "innermost_hemisphere_PV", world_LV, false, 0 );



  //  PFM-NOTE: Building the new lightguide
  G4double hexagon_A_Radius = (( 3.82 ) / 2.0 ) * cm;
  G4double hexagon_A_RadiusScaled = hexagon_A_Radius * shellScale;
  const G4double hexagon_A_ZPlane[ 2 ] = { 0.0 * cm, -1.0 * hemisphereRadiusOuterScaled };
  //const G4double hexagon_A_ZPlane[ 2 ] = { 0.0 * cm, -5.0 * cm };
  const G4double hexagon_A_RadiiInner[ 2 ] = { 0.0, 0.0 };
  const G4double hexagon_A_RadiiOuter[ 2 ] = { 0.0, hexagon_A_RadiusScaled };
  //const G4double hexagon_A_RadiiOuter[ 2 ] = { 0.0, hexagon_A_Radius };
  G4double hexAatThinnest = 0.0 * mm;
  G4double hexAatThinnestScaled = hexAatThinnest * shellScale;

  hexagonPyramid = new G4Polyhedra( "hexagonPyramid", 0.0*M_PI, 2.0*M_PI, 6, 2, hexagon_A_ZPlane, hexagon_A_RadiiInner, hexagon_A_RadiiOuter );
  //hexagonPyramid_LV = new G4LogicalVolume( hexagonPyramid, MaterialFactory::getInstance()->getEpoxy(), "hexagonPyramid_LV", 0, 0, 0 );
  //hexagonPyramid_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagonPyramid_LV, "hexagonPyramid_PV", world_LV, false, 0 );


  hexagon_A = new G4Polyhedra( "lgA", 0.0*M_PI, 2.0*M_PI, 6, 2, hexagon_A_ZPlane, hexagon_A_RadiiInner, hexagon_A_RadiiOuter );

  //  Create the sphere to cut the section out of the pyramid to finish lgA 
  lg_orb_void = new G4Orb( "lg_orb_void", hemisphereRadiusOuterScaled );
  //lg_orb_void = new G4Orb( "lg_orb_void", 5.0 * cm );
  hexagon_A_MinusOrb = new G4SubtractionSolid( "Pyramid-Orb", hexagon_A, lg_orb_void ); 

  hexagon_A_LV = new G4LogicalVolume( hexagon_A_MinusOrb, MaterialFactory::getInstance()->getEpoxy(), "hexagon_A_logicalVolume", 0, 0, 0 );
  hexagon_A_Fake_LV = new G4LogicalVolume( hexagon_A_MinusOrb, MaterialFactory::getInstance()->getEpoxy(), "hexagon_A_Fake_LV", 0, 0, 0 );
  //G4Colour green = G4Color::Green();
  G4Colour brown = G4Color::Brown();
  hexagon_A_Fake_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 0.3)));
  //hexagon_A_logicalVolumeFake->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed(), brown.GetGreen(), brown.GetBlue(), 0.3)));
  //hexagon_A_logicalVolumeFake->SetVisAttributes(new G4VisAttributes(G4Color(green.GetRed(), green.GetGreen(), green.GetBlue(), 0.3)));
  //hexagon_A_logicalVolumeFake->SetVisAttributes(G4VisAttributes::Invisible);

  //  END - Hexagon logical volume creation 


  //hexagon_A_physicalVolume = new G4PVPlacement( rm_x1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolume, "lightguide", world_LV, false, 0 );


  G4ThreeVector axisOfRotation1 = G4ThreeVector( 1.0, 0.0, 0.0 ).unit();
  G4ThreeVector axisOfRotation2 = G4ThreeVector( 0.0, 1.0, 0.0 ).unit();
  G4ThreeVector axisOfRotation3 = G4ThreeVector( 0.0, 0.0, 1.0 ).unit();

  G4RotationMatrix *rotMatrix1 = new G4RotationMatrix();
  rotMatrix1->rotate((( 90.0 - 36.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexA = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexA", world_LV, false, 0 );
  //hexagon_A_physicalVolumeDummyHexA = new G4PVPlacement( rotMatrix1, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexA", world_LV, false, 0 );

  G4double HexRing1Angle = 37.4;
  G4double HexRing1Step = 72.0;

  //  PFM-TWEAK3 

  G4RotationMatrix *rotMatrix2 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix2a = new G4RotationMatrix();
  rotMatrix2->rotate( -1.0 * (( 90.0 + ( HexRing1Step * 0.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix2a->rotate( -1.0 * (( 90.0 + ( HexRing1Step * 0.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix2->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix2a->rotate((( -1.0 * HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //rotMatrix2a->rotate((( 360.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //rotMatrix2->rotate((( 90.0 - 36.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexB = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexB", world_LV, false, 0 );
  //hexagon_A_DummyHexB_PV = new G4PVPlacement( rotMatrix2, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_Fake_LV, "hexagon_A_DummyHexB_PV", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix3 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix3a = new G4RotationMatrix();
  rotMatrix3->rotate( -1.0 * (( 90 + ( HexRing1Step * 1.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix3a->rotate( -1.0 * (( 90 + ( HexRing1Step * 1.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix3->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix3a->rotate((( -1.0 * HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_DummyHexC_PV = new G4PVPlacement( rotMatrix3, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_Fake_LV, "hexagon_A_DummyHexC_PV", world_LV, false, 0 );

//  surgical, remove 
/*
  G4RotationMatrix *rotMatrix3 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix3a = new G4RotationMatrix();
  rotMatrix3->rotate( -1.0 * (( 90 + ( HexRing1Step * 1.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix3a->rotate( -1.0 * (( 90 + 72 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix3->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix3a->rotate((( 360.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  hexagon_A_physicalVolumeDummyHexC = new G4PVPlacement( rotMatrix3, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexC", world_LV, false, 0 );
*/

  G4RotationMatrix *rotMatrix4 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix4a = new G4RotationMatrix();
  rotMatrix4->rotate( -1.0 * (( 90 + ( HexRing1Step * 2.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix4a->rotate( -1.0 * (( 90 + ( HexRing1Step * 2.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix4->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix4a->rotate((( -1.0 * HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_DummyHexD_PV = new G4PVPlacement( rotMatrix4, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_Fake_LV, "hexagon_A_DummyHexD_PV", world_LV, false, 0 );

/*
  G4RotationMatrix *rotMatrix4 = new G4RotationMatrix();
  rotMatrix4->rotate( -1.0 * (( 90 + ( HexRing1Step * 2.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix4->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexD = new G4PVPlacement( rotMatrix4, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexD", world_LV, false, 0 );
*/

  G4RotationMatrix *rotMatrix5 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix5a = new G4RotationMatrix();
  rotMatrix5->rotate( -1.0 * (( 90 + ( HexRing1Step * 3.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix5a->rotate( -1.0 * (( 90 + ( HexRing1Step * 3.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix5->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix5a->rotate((( -1.0 * HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_DummyHexE_PV = new G4PVPlacement( rotMatrix5, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_Fake_LV, "hexagon_A_DummyHexE_PV", world_LV, false, 0 );

/*
  G4RotationMatrix *rotMatrix5 = new G4RotationMatrix();
  rotMatrix5->rotate( -1.0 * (( 90 + ( HexRing1Step * 3.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix5->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexE = new G4PVPlacement( rotMatrix5, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexE", world_LV, false, 0 );
*/

  G4RotationMatrix *rotMatrix6 = new G4RotationMatrix();
  G4RotationMatrix *rotMatrix6a = new G4RotationMatrix();
  rotMatrix6->rotate( -1.0 * (( 90 + ( HexRing1Step * 4.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix6a->rotate( -1.0 * (( 90 + ( HexRing1Step * 4.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix6->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix6a->rotate((( -1.0 * HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_DummyHexF_PV = new G4PVPlacement( rotMatrix6, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_Fake_LV, "hexagon_A_DummyHexF_PV", world_LV, false, 0 );

/*
  G4RotationMatrix *rotMatrix6 = new G4RotationMatrix();
  rotMatrix6->rotate( -1.0 * (( 90 + ( HexRing1Step * 4.0 )) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix6->rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexF = new G4PVPlacement( rotMatrix6, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexF", world_LV, false, 0 );
*/
  //  Second group of hexagons
  G4double HexRing2Angle = 41.8;

  G4RotationMatrix *rotMatrix7 = new G4RotationMatrix();
  rotMatrix7->rotate( -1.0 * ( 90.0 / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix7->rotate((( 90.0 - HexRing1Angle - HexRing2Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexG = new G4PVPlacement( rotMatrix7, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexG", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix8 = new G4RotationMatrix();
  rotMatrix8->rotate( -1.0 * (( 90 + 72 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix8->rotate((( 90.0 - HexRing1Angle - HexRing2Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexH = new G4PVPlacement( rotMatrix8, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexH", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix9 = new G4RotationMatrix();
  rotMatrix9->rotate( -1.0 * (( 90 + 144 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix9->rotate((( 90.0 - HexRing1Angle - HexRing2Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexI = new G4PVPlacement( rotMatrix9, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexI", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix10 = new G4RotationMatrix();
  rotMatrix10->rotate( -1.0 * (( 90 + 216 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix10->rotate((( 90.0 - HexRing1Angle - HexRing2Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexJ = new G4PVPlacement( rotMatrix10, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexJ", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix11 = new G4RotationMatrix();
  rotMatrix11->rotate( -1.0 * (( 90 + 288 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix11->rotate((( 90.0 - HexRing1Angle - HexRing2Angle ) / 180.0 ) * M_PI, axisOfRotation1 );
  //hexagon_A_physicalVolumeDummyHexK = new G4PVPlacement( rotMatrix11, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_A_logicalVolumeFake, "lightguideDummyHexK", world_LV, false, 0 );

  //  Create the pentagonal versions of the facets; not as lightguides but for their geometry to manage surfaces
  G4double pentagon_A_Radius = ( 1.5177 ) * cm;  //  inscribed inside target pentagon geometry; confirmed by drawing
  G4double pentagon_A_RadiusScaled = pentagon_A_Radius * shellScale;  

  G4double pentagon_A_RadiusTangent = 5.1327 * cm;
  G4double pentagon_A_RadiusTangentScaled = pentagon_A_RadiusTangent * shellScale;  


  const G4double pentagon_A_ZPlane[ 2 ] = { 0.0 * cm, -1 * pentagon_A_RadiusTangentScaled };  //  Confirmed by drawing
  //const G4double pentagon_A_ZPlane[ 2 ] = { 0.0 * cm, -5.1327 * cm };  //  Confirmed by drawing
  //const G4double pentagon_A_ZPlane[ 2 ] = { 0.0 * cm, -5.1283 * cm };
  const G4double pentagon_A_RadiiInner[ 2 ] = { 0.0, 0.0 };
  const G4double pentagon_A_RadiiOuter[ 2 ] = { 0.0, pentagon_A_RadiusScaled };
  //const G4double pentagon_A_RadiiOuter[ 2 ] = { 0.0, pentagon_A_Radius };
  G4double pentAatThinnest = 1.327 * mm;
  G4double pentAatThinnestScaled = pentAatThinnest * shellScale;

  G4double lightguideThicknessAB = 5.0 * mm;
  //G4double lightguideThicknessAB = 1.5 * mm;
  //G4double lightguideThicknessAB = 10.0 * mm;
  G4double lightguideThicknessHexB = lightguideThicknessAB - hexAatThinnestScaled;
  G4double lightguideThicknessPentB = lightguideThicknessAB - pentAatThinnestScaled;
  //G4double lightguideThicknessPentB = lightguideThicknessAB - pentAatThinnest;
  pentagonPyramid = new G4Polyhedra( "pentagonPyramid", 0.0*M_PI, 2.0*M_PI, 5, 2, pentagon_A_ZPlane, pentagon_A_RadiiInner, pentagon_A_RadiiOuter );
  //pentagonPyramid_LV = new G4LogicalVolume( pentagonPyramid, MaterialFactory::getInstance()->getEpoxy(), "pentagonPyramid_LV", 0, 0, 0 );
  //pentagonPyramid_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagonPyramid_LV, "pentagonPyramid_PV", world_LV, false, 0 );

/*
  chapeauMultiUnion = new G4MultiUnion( "chapeauMultiUnion" );
  G4ThreeVector pentFuse1Pos = G4ThreeVector( 0.0 * mm, 100.0 * mm, 0.0 * mm );
  G4RotationMatrix pentFuse1Rot = G4RotationMatrix();
  G4Transform3D trans1 = G4Transform3D( pentFuse1Rot, pentFuse1Pos ); 
  chapeauMultiUnion->AddNode( *pentagonPyramid, trans1 );

  G4ThreeVector hexFuse1Pos = G4ThreeVector( 0.0 * mm, 100.0 * mm, 0.0 * mm );
  G4RotationMatrix hexFuse1Rot = G4RotationMatrix();
  hexFuse1Rot.rotate( 30.0 * degree, axisOfRotation3 );
  hexFuse1Rot.rotate( HexRing1Angle * degree, axisOfRotation2 );
  G4Transform3D trans2 = G4Transform3D( hexFuse1Rot, hexFuse1Pos ); 
  //G4Transform3D trans2 = G4Transform3D(); 
  chapeauMultiUnion->AddNode( *hexagonPyramid, trans2 );
  chapeauMultiUnion->Voxelize();

  chapeauMultiUnion_LV = new G4LogicalVolume( chapeauMultiUnion, MaterialFactory::getInstance()->getEpoxy(), "chapeauMultiUnion_LV", 0, 0, 0 );
  //chapeauMultiUnion_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , chapeauMultiUnion_LV, "chapeauMultiUnion_PV", world_LV, false, 0 );
*/

  //  PFM-NOTE: Chapeau work - start

  //  Fixed values to start truncated icosohedron (TI) 
  G4double chapeau_a = ( 3.0 + sqrt( 5.0 )) / 6.0;
  G4double chapeau_b = ( sqrt( 5.0 ) - 1.0 ) / 3.0;
  G4double chapeau_c = sqrt( 5.0 ) / 3.0;
  G4double chapeau_bDiv2 = chapeau_b / 2.0;
  G4double chapeau_zero = 0.0;
  G4double chapeau_one = 1.0;
  G4double chapeau_oneThird = 1.0 / 3.0; 
  G4double chapeau_twoThirds = 2.0 / 3.0;

  G4ThreeVector chapeau_Vertices[ 72 ] = { 
  //const G4ThreeVector chapeau_Vertices[ 72 ] = { 
	//  +++
	G4ThreeVector( chapeau_oneThird, chapeau_a, chapeau_b ),
	G4ThreeVector( chapeau_b, chapeau_oneThird, chapeau_a ),
	G4ThreeVector( chapeau_a, chapeau_b, chapeau_oneThird ),
	G4ThreeVector( chapeau_twoThirds, chapeau_c, chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, chapeau_twoThirds, chapeau_c ),
	G4ThreeVector( chapeau_c, chapeau_bDiv2, chapeau_twoThirds ),
	G4ThreeVector( chapeau_one, chapeau_bDiv2, chapeau_zero ),
	G4ThreeVector( chapeau_zero, chapeau_one, chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, chapeau_zero, chapeau_one ),

	//  -++
	G4ThreeVector( -1.0 * chapeau_oneThird, chapeau_a, chapeau_b ),
	G4ThreeVector( -1.0 * chapeau_b, chapeau_oneThird, chapeau_a ),
	G4ThreeVector( -1.0 * chapeau_a, chapeau_b, chapeau_oneThird ),
	G4ThreeVector( -1.0 * chapeau_twoThirds, chapeau_c, chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, chapeau_twoThirds, chapeau_c ),
	G4ThreeVector( -1.0 * chapeau_c, chapeau_bDiv2, chapeau_twoThirds ),
	G4ThreeVector( -1.0 * chapeau_one, chapeau_bDiv2, chapeau_zero ),
	G4ThreeVector( -1.0 * chapeau_zero, chapeau_one, chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, chapeau_zero, chapeau_one ),

	//  +-+
	G4ThreeVector( chapeau_oneThird, -1.0 * chapeau_a, chapeau_b ),
	G4ThreeVector( chapeau_b, -1.0 * chapeau_oneThird, chapeau_a ),
	G4ThreeVector( chapeau_a, -1.0 * chapeau_b, chapeau_oneThird ),
	G4ThreeVector( chapeau_twoThirds, -1.0 * chapeau_c, chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, -1.0 * chapeau_twoThirds, chapeau_c ),
	G4ThreeVector( chapeau_c, -1.0 * chapeau_bDiv2, chapeau_twoThirds ),
	G4ThreeVector( chapeau_one, -1.0 * chapeau_bDiv2, chapeau_zero ),
	G4ThreeVector( chapeau_zero, -1.0 * chapeau_one, chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, -1.0 * chapeau_zero, chapeau_one ),

	//  --+
	G4ThreeVector( -1.0 * chapeau_oneThird, -1.0 * chapeau_a, chapeau_b ),
	G4ThreeVector( -1.0 * chapeau_b, -1.0 * chapeau_oneThird, chapeau_a ),
	G4ThreeVector( -1.0 * chapeau_a, -1.0 * chapeau_b, chapeau_oneThird ),
	G4ThreeVector( -1.0 * chapeau_twoThirds, -1.0 * chapeau_c, chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds, chapeau_c ),
	G4ThreeVector( -1.0 * chapeau_c, -1.0 * chapeau_bDiv2, chapeau_twoThirds ),
	G4ThreeVector( -1.0 * chapeau_one, -1.0 * chapeau_bDiv2, chapeau_zero ),
	G4ThreeVector( -1.0 * chapeau_zero, -1.0 * chapeau_one, chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero, chapeau_one ),

	//  ++-
	G4ThreeVector( chapeau_oneThird, chapeau_a, -1.0 * chapeau_b ),
	G4ThreeVector( chapeau_b, chapeau_oneThird, -1.0 * chapeau_a ),
	G4ThreeVector( chapeau_a, chapeau_b, -1.0 * chapeau_oneThird ),
	G4ThreeVector( chapeau_twoThirds, chapeau_c, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, chapeau_twoThirds, -1.0 * chapeau_c ),
	G4ThreeVector( chapeau_c, chapeau_bDiv2, -1.0 * chapeau_twoThirds ),
	G4ThreeVector( chapeau_one, chapeau_bDiv2, -1.0 * chapeau_zero ),
	G4ThreeVector( chapeau_zero, chapeau_one, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, chapeau_zero, -1.0 * chapeau_one ),

	//  -+-
	G4ThreeVector( -1.0 * chapeau_oneThird, chapeau_a, -1.0 * chapeau_b ),
	G4ThreeVector( -1.0 * chapeau_b, chapeau_oneThird, -1.0 * chapeau_a ),
	G4ThreeVector( -1.0 * chapeau_a, chapeau_b, -1.0 * chapeau_oneThird ),
	G4ThreeVector( -1.0 * chapeau_twoThirds, chapeau_c, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, chapeau_twoThirds, -1.0 * chapeau_c ),
	G4ThreeVector( -1.0 * chapeau_c, chapeau_bDiv2, -1.0 * chapeau_twoThirds ),
	G4ThreeVector( -1.0 * chapeau_one, chapeau_bDiv2, -1.0 * chapeau_zero ),
	G4ThreeVector( -1.0 * chapeau_zero, chapeau_one, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, chapeau_zero, -1.0 * chapeau_one ),

	//  +--
	G4ThreeVector( chapeau_oneThird, -1.0 * chapeau_a, -1.0 * chapeau_b ),
	G4ThreeVector( chapeau_b, -1.0 * chapeau_oneThird, -1.0 * chapeau_a ),
	G4ThreeVector( chapeau_a, -1.0 * chapeau_b, -1.0 * chapeau_oneThird ),
	G4ThreeVector( chapeau_twoThirds, -1.0 * chapeau_c, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, -1.0 * chapeau_twoThirds, -1.0 * chapeau_c ),
	G4ThreeVector( chapeau_c, -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds ),
	G4ThreeVector( chapeau_one, -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero ),
	G4ThreeVector( chapeau_zero, -1.0 * chapeau_one, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( chapeau_bDiv2, -1.0 * chapeau_zero, -1.0 * chapeau_one ),

	//  ---
	G4ThreeVector( -1.0 * chapeau_oneThird, -1.0 * chapeau_a, -1.0 * chapeau_b ),
	G4ThreeVector( -1.0 * chapeau_b, -1.0 * chapeau_oneThird, -1.0 * chapeau_a ),
	G4ThreeVector( -1.0 * chapeau_a, -1.0 * chapeau_b, -1.0 * chapeau_oneThird ),
	G4ThreeVector( -1.0 * chapeau_twoThirds, -1.0 * chapeau_c, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds, -1.0 * chapeau_c ),
	G4ThreeVector( -1.0 * chapeau_c, -1.0 * chapeau_bDiv2, -1.0 * chapeau_twoThirds ),
	G4ThreeVector( -1.0 * chapeau_one, -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero ),
	G4ThreeVector( -1.0 * chapeau_zero, -1.0 * chapeau_one, -1.0 * chapeau_bDiv2 ),
	G4ThreeVector( -1.0 * chapeau_bDiv2, -1.0 * chapeau_zero, -1.0 * chapeau_one ),

	};


  const G4ThreeVector zero_Vertices[ 1 ] = { 
	G4ThreeVector( 0.0, 0.0, 0.0 )
	};

  int xVertexOffset = 9;
  int yVertexOffset = 18;
  int xyVertexOffset = 27;
  int zVertexOffset = 36;
  int xzVertexOffset = 45;
  int yzVertexOffset = 54;
  int xyzVertexOffset = 63;

  const int pentagonVertexIndices[ 12 ][ 5 ] = {
	{ 5, 1, 8, 1 + yVertexOffset, 5 + yVertexOffset }, // 1
	{ 1 + xyzVertexOffset, 5 + xyzVertexOffset, 5 + xzVertexOffset, 1 + xzVertexOffset, 8 + xzVertexOffset }, // 2
	{ 0 + xyVertexOffset, 7 + yVertexOffset, 0 + yVertexOffset, 4 + yVertexOffset, 4 + xyVertexOffset }, // 3
	{ 7 + zVertexOffset, 0 + zVertexOffset, 4 + zVertexOffset, 4 + xzVertexOffset, 0 + xzVertexOffset }, // 4
	{ 3 + yzVertexOffset, 2 + yzVertexOffset, 6 + yVertexOffset, 2 + yVertexOffset, 3 + yVertexOffset }, // 5
	{ 5 + zVertexOffset, 5 + yzVertexOffset, 1 + yzVertexOffset, 8 + yzVertexOffset, 1 + zVertexOffset }, // 6
	{ 2 + zVertexOffset, 3 + zVertexOffset, 3, 2, 6 }, // 7
	{ 0 + yzVertexOffset, 7 + yzVertexOffset, 0 + xyzVertexOffset, 4 + xyzVertexOffset, 4 + yzVertexOffset }, // 8
	{ 7, 0 + xVertexOffset, 4 + xVertexOffset, 4, 0 }, // 9
	{ 3 + xyVertexOffset, 2 + xyVertexOffset, 6 + xyVertexOffset, 2 + xyzVertexOffset, 3 + xyzVertexOffset}, // 10
	{ 5 + xVertexOffset, 5 + xyVertexOffset, 1 + xyVertexOffset, 8 + xVertexOffset, 1 + xVertexOffset }, // 11
	{ 2 + xVertexOffset, 3 + xVertexOffset, 3 + xzVertexOffset, 2 + xzVertexOffset, 6 + xVertexOffset }, // 12
	};

  const int hexagonVertexIndices[ 20 ][ 6 ] = {
	{ 3, 0, 4, 1, 5, 2 }, // 1
	{ 0 + xyzVertexOffset, 3 + xyzVertexOffset, 2 + xyzVertexOffset, 5 + xyzVertexOffset, 1 + xyzVertexOffset, 4 + xyzVertexOffset }, // 2
	{ 4 + xVertexOffset, 1 + xVertexOffset, 8 + xVertexOffset, 8, 1, 4 }, // 3
	{ 6 + xyVertexOffset, 6 + xVertexOffset, 2 + xzVertexOffset, 5 + xzVertexOffset, 5 + xyzVertexOffset, 2 + xyzVertexOffset }, // 4
	{ 1 + xyVertexOffset, 4 + xyVertexOffset, 4 + yVertexOffset, 1 + yVertexOffset, 8 + yVertexOffset, 8 + xVertexOffset }, // 5
	{ 3 + xzVertexOffset, 0 + xzVertexOffset, 4 + xzVertexOffset, 1 + xzVertexOffset, 5 + xzVertexOffset, 2 + xzVertexOffset }, // 6
	{ 0 + yVertexOffset, 3 + yVertexOffset, 2 + yVertexOffset, 5 + yVertexOffset, 1 + yVertexOffset, 4 + yVertexOffset }, // 7
	{ 4 + zVertexOffset, 1 + zVertexOffset, 8 + zVertexOffset, 8 + xzVertexOffset, 1 + xzVertexOffset, 4 + xzVertexOffset }, // 8
	{ 6 + yVertexOffset, 6, 2, 5, 5 + yVertexOffset, 2 + yVertexOffset }, // 9
	{ 1 + yzVertexOffset, 4 + yzVertexOffset, 4 + xyzVertexOffset, 1 + xyzVertexOffset, 8 +xzVertexOffset, 8 + zVertexOffset }, // 10
	{ 0 + zVertexOffset, 7 + zVertexOffset, 7, 0, 3, 3 + zVertexOffset }, // 11
	{ 7 + yVertexOffset, 0 + xyVertexOffset, 3 + xyVertexOffset, 3 + xyzVertexOffset, 0 + xyzVertexOffset, 7 + yzVertexOffset }, // 12
	{ 3 + xVertexOffset, 2 + xVertexOffset, 5 + xVertexOffset, 1 + xVertexOffset, 4 + xVertexOffset, 0 + xVertexOffset }, // 13
	{ 5 + xyVertexOffset, 5 + xVertexOffset, 2 + xVertexOffset, 6 + xVertexOffset, 6 + xyVertexOffset, 2 + xyVertexOffset }, // 14
	{ 2 + xyVertexOffset, 3 + xyVertexOffset, 0 + xyVertexOffset, 4 + xyVertexOffset, 1 + xyVertexOffset, 5 + xyVertexOffset }, // 15
	{ 0 + xVertexOffset, 7, 7 + zVertexOffset, 0 + xzVertexOffset, 3 + xzVertexOffset, 3 + xVertexOffset }, // 16
	{ 7 + yzVertexOffset, 0 + yzVertexOffset, 3 + yzVertexOffset, 3 + yVertexOffset, 0 + yVertexOffset, 7 + yVertexOffset }, // 17
	{ 3 + zVertexOffset, 2 + zVertexOffset, 5 + zVertexOffset, 1 + zVertexOffset, 4 + zVertexOffset, 0 + zVertexOffset }, // 18
	{ 5 + yzVertexOffset, 5 + zVertexOffset, 2 + zVertexOffset, 6, 6 + yVertexOffset, 2 + yzVertexOffset }, // 19
	{ 2 + yzVertexOffset, 3 + yzVertexOffset, 0 + yzVertexOffset, 4 + yzVertexOffset, 1 + yzVertexOffset, 5 + yzVertexOffset }, // 20
	};

  //  PFM-NOTE: work to simplify geometry before construction
  //  Find the center of the pentagon
  //  Find the destination vector (positive Z axis, ( 0, 0, 1 )
  //  Find the normal to those two vectors (axis of rotation) 
  //  Get the angular difference from first to second
  //  Do the rotation of all the points and confirm everything is correct
  //  NA: do the second rotation so all that remains is a simple 90 degree flip to get the LV to its PV location 
  G4ThreeVector P1_CoG = ( chapeau_Vertices[ 1 ] + chapeau_Vertices[ 5 ] + chapeau_Vertices[ 5 + yVertexOffset ] + chapeau_Vertices[ 1 + yVertexOffset ] + chapeau_Vertices[ 8 ] ) / 5.0;
  G4ThreeVector zPosAxis = G4ThreeVector( 0.0 * mm, 0.0 * mm, 1.0 * mm );
  G4ThreeVector myCross1 = zPosAxis.cross( P1_CoG );
  G4double angleDelta1 = zPosAxis.theta( P1_CoG );

  int vertIndex = 0;

  while ( vertIndex < 72 ) {
	chapeau_Vertices[ vertIndex ] = chapeau_Vertices[ vertIndex ].rotate( myCross1, -1.0 * angleDelta1 );
	vertIndex = vertIndex + 1;
  }


  P1_CoG = ( chapeau_Vertices[ 1 ] + chapeau_Vertices[ 5 ] + chapeau_Vertices[ 5 + yVertexOffset ] + chapeau_Vertices[ 1 + yVertexOffset ] + chapeau_Vertices[ 8 ] ) / 5.0;
  G4ThreeVector P1_axis1 = ( chapeau_Vertices[ 1 + yVertexOffset ] - P1_CoG ) * -1.0; 
  G4ThreeVector yPosAxis = G4ThreeVector( 0.0 * mm, 1.0 * mm, 0.0 * mm );
  G4ThreeVector myCross2 = yPosAxis.cross( P1_axis1 );
  G4double angleDelta2 = yPosAxis.theta( P1_axis1 );

  vertIndex = 0;

  while ( vertIndex < 72 ) {
	chapeau_Vertices[ vertIndex ] = chapeau_Vertices[ vertIndex ].rotate( myCross2, -1.0 * ( angleDelta2 + ( 90.0 * deg )));
	vertIndex = vertIndex + 1;
  }

/*
  G4ThreeVector xPosAxis = G4ThreeVector( 1.0 * mm, 0.0 * mm, 0.0 * mm );
  G4double angleDelta3 = 90.0 * deg;

  vertIndex = 0;

  while ( vertIndex < 72 ) {
	chapeau_Vertices[ vertIndex ] = chapeau_Vertices[ vertIndex ].rotate( xPosAxis, +1.0 * angleDelta3 );
	vertIndex = vertIndex + 1;
  }
*/

/*
  G4ThreeVector chapeauPoof_P1B_axis1 = (( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) - chapeauPoof_P1B_CoG ) * -1.0; 
  G4ThreeVector chapeauPoof_P1B_axis2 = (( chapeau_Vertices[ 5 ] * chapeau_scale ) - ( chapeau_Vertices[ 1 ] * chapeau_scale )) * -1.0;
  chapeauPoof_P1B_axis1 = chapeauPoof_P1B_axis1 / chapeauPoof_P1B_axis1.mag();
  chapeauPoof_P1B_axis2 = chapeauPoof_P1B_axis2 / chapeauPoof_P1B_axis2.mag();
*/

  //  Figure out the scaling factor to get everything to line up
  G4double bigR = sqrt(( 21.0 - sqrt( 5.0 )) / 18.0 );
  G4double littleR = ( sqrt( 3.0 ) * ( sqrt( 5.0 ) + 1.0 )) / 6.0;
  G4double ratioRs = bigR / littleR;
  G4double knownR = 34.5 * mm;
  G4double targetR = knownR * ratioRs;
  G4double chapeau_scale = targetR / bigR;
  // The supplied radius locates the optical entrance plane.
  // The upstream default housing is placed by its center; shift outward by its half-depth.
  G4double sipmR = 39.5 * mm;
  // Housing front touches the guide outlet; guide radius stays unchanged.
  const G4double sensorCenterR = sipmR + housing->getDz() / 2.0;
  G4double pyramidHeight = sqrt( ( 85.0 - sqrt(5)) / 90.0 );
  G4double pyramidHeightScaled = pyramidHeight * chapeau_scale;
  G4double targetHBThickness = sipmR - knownR;
  G4double targetPBThickness = sipmR - pyramidHeightScaled;

  //G4double chapeau_scale = 50.0 * mm;
  G4double chapeau_casting_scale = 2.0;

  //  Do the creation of the vertices for the B sections
  G4ThreeVector pentagonQuad_Vertices[ 12 ][ 4 * 2 ];
  G4ThreeVector hexagonQuad_Vertices[ 20 ][ 4 * 2 ];
  //G4ThreeVector hexagonQuadExt_Vertices[ 20 ][ 4 * 2 ];

  int pentagonIndex = 0;
  int pentagonCount = 12;
  while ( pentagonIndex < pentagonCount ) {

	G4ThreeVector PB_CoG = (( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale ) + 
( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale )) / 5.0;
	G4ThreeVector PB_axis1 = ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale ) - ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale );
	G4ThreeVector PB_axis2 = ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale ) - PB_CoG;

	//  Convert to unit length
	PB_CoG = PB_CoG / PB_CoG.mag();
	PB_axis1 = PB_axis1 / PB_axis1.mag();
	PB_axis2 = PB_axis2 / PB_axis2.mag();

	G4double CoG_scale = sipmR / knownR;
	G4double pbHalf = 9.0 * mm;

	pentagonQuad_Vertices[ pentagonIndex ][ 0 ] = G4ThreeVector((( zero_Vertices[ 0 ] - PB_axis1 - PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR ));
	pentagonQuad_Vertices[ pentagonIndex ][ 1 ] = G4ThreeVector((( zero_Vertices[ 0 ] + PB_axis1 - PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR ));
	pentagonQuad_Vertices[ pentagonIndex ][ 2 ] = G4ThreeVector((( zero_Vertices[ 0 ] + PB_axis1 + PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR ));
	pentagonQuad_Vertices[ pentagonIndex ][ 3 ] = G4ThreeVector((( zero_Vertices[ 0 ] - PB_axis1 + PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR ));

	pentagonQuad_Vertices[ pentagonIndex ][ 4 ] = G4ThreeVector((( zero_Vertices[ 0 ] - PB_axis1 - PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR * chapeau_casting_scale ));
	pentagonQuad_Vertices[ pentagonIndex ][ 5 ] = G4ThreeVector((( zero_Vertices[ 0 ] + PB_axis1 - PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR * chapeau_casting_scale ));
	pentagonQuad_Vertices[ pentagonIndex ][ 6 ] = G4ThreeVector((( zero_Vertices[ 0 ] + PB_axis1 + PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR * chapeau_casting_scale ));
	pentagonQuad_Vertices[ pentagonIndex ][ 7 ] = G4ThreeVector((( zero_Vertices[ 0 ] - PB_axis1 + PB_axis2 ) * pbHalf ) + ( PB_CoG * sipmR * chapeau_casting_scale ));

	pentagonIndex = pentagonIndex + 1;
  }

  int hexagonIndex = 0;
  int hexagonCount = 20;
  while ( hexagonIndex < hexagonCount ) {

	G4ThreeVector HB_CoG = (( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]]  * chapeau_scale ) + ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale )) / 2.0;
	G4ThreeVector HB_axis1 = ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale ) - HB_CoG;
	G4ThreeVector HB_axis2 = ((( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale ) + ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale )) / 2.0 ) - HB_CoG;

	//  Convert to unit length
	HB_CoG = HB_CoG / HB_CoG.mag();
	HB_axis1 = HB_axis1 / HB_axis1.mag();
	HB_axis2 = HB_axis2 / HB_axis2.mag();
	G4double CoG_scale = sipmR / knownR;
	G4double hbHalf = 9.0 * mm;

	//  PFM-RESUME: complete the rescaling of the CoG vector and confirm it's correct 

	hexagonQuad_Vertices[ hexagonIndex ][ 0 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR ));
	hexagonQuad_Vertices[ hexagonIndex ][ 1 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR ));
	hexagonQuad_Vertices[ hexagonIndex ][ 2 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR ));
	hexagonQuad_Vertices[ hexagonIndex ][ 3 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR ));

	hexagonQuad_Vertices[ hexagonIndex ][ 4 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 5 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 6 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 7 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * sipmR * chapeau_casting_scale ));

/*
	hexagonQuad_Vertices[ hexagonIndex ][ 0 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 1 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 2 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 3 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale ));

	hexagonQuad_Vertices[ hexagonIndex ][ 4 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 5 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 - HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 6 ] = G4ThreeVector((( zero_Vertices[ 0 ] + HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale * chapeau_casting_scale ));
	hexagonQuad_Vertices[ hexagonIndex ][ 7 ] = G4ThreeVector((( zero_Vertices[ 0 ] - HB_axis1 + HB_axis2 ) * hbHalf ) + ( HB_CoG * CoG_scale * chapeau_casting_scale ));
*/
  	hexagonIndex = hexagonIndex + 1;
  }


  //  PFM-NOTE: Here's the new geometry creation code 
  //  Code to build the pentagon facets
   pentagonIndex = 0;
  //int pentagonCount = 1;
  int pentagonFacetIndex = 0;

  while ( pentagonIndex < pentagonCount ) {
  	chapeauPoof_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

  	chapeauPoof_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

  	chapeauPoof_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

	pentagonIndex = pentagonIndex + 1;

  }

  hexagonIndex = 0;
  //int hexagonCount = 1;
  int hexagonFacetIndex = 0;

  while ( hexagonIndex < hexagonCount ) {
  	chapeauPoof_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauPoof_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauPoof_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauPoof_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

	hexagonIndex = hexagonIndex + 1;

  }

  //if (config.integer("verbose") > 0) std::cout << "DetectorConstruction::DetectorConstruction: oink!" << std::endl;

  //  "Skirt" facets
  int skirtIndex = 0;
  hexagonIndex = 10;

  while ( skirtIndex < 5 ) {
	chapeauPoof_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	hexagonIndex = hexagonIndex + 2;
	skirtIndex = skirtIndex + 1;
  }

  skirtIndex = 5;
  hexagonIndex = 11;

  //while ( skirtIndex < 10 ) {
  while ( skirtIndex < 20 ) {
//	if (config.integer("verbose") > 0) std::cout << "DetectorConstruction::DetectorConstruction: moo!" << std::endl;

	chapeauPoof_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	chapeauPoof_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	chapeauPoof_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	hexagonIndex = hexagonIndex + 2;
  }

  int squareIndex = 0;
  int squareCount = 2 * 20;
  hexagonIndex = 0;

  while ( squareIndex < squareCount ) { 

// hexagonQuad_Vertices[ hexagonIndex ][ 0 ] 

	chapeauPoof_A2_SquareHexagonFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 0 ], hexagonQuad_Vertices[ hexagonIndex ][ 1 ], hexagonQuad_Vertices[ hexagonIndex ][ 2 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauPoof_A2_SquareHexagonFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 2 ], hexagonQuad_Vertices[ hexagonIndex ][ 3 ], hexagonQuad_Vertices[ hexagonIndex ][ 0 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	hexagonIndex = hexagonIndex + 1;
  }

  squareIndex = 0;
  squareCount = 2 * 12;
  pentagonIndex = 0;

  while ( squareIndex < squareCount ) { 

	//chapeauPoof_A2_SquarePentagonFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 4 ], pentagonQuad_Vertices[ pentagonIndex ][ 5 ], pentagonQuad_Vertices[ pentagonIndex ][ 6 ], ABSOLUTE );
	chapeauPoof_A2_SquarePentagonFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 0 ], pentagonQuad_Vertices[ pentagonIndex ][ 1 ], pentagonQuad_Vertices[ pentagonIndex ][ 2 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	//chapeauPoof_A2_SquarePentagonFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 6 ], pentagonQuad_Vertices[ pentagonIndex ][ 7 ], pentagonQuad_Vertices[ pentagonIndex ][ 4 ], ABSOLUTE );
	chapeauPoof_A2_SquarePentagonFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 2 ], pentagonQuad_Vertices[ pentagonIndex ][ 3 ], pentagonQuad_Vertices[ pentagonIndex ][ 0 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	pentagonIndex = pentagonIndex + 1;
  }

  int pentagonSkirtIndex = 0;
  //int pentagonSkirtCount = 9;
  pentagonIndex = 0;

  while ( pentagonIndex < pentagonCount ) { 

	//  0
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  1
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], pentagonQuad_Vertices[ pentagonIndex ][ 3 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  2
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 3 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  3
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 3 ], pentagonQuad_Vertices[ pentagonIndex ][ 2 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  4
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 2 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  5
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 2 ], pentagonQuad_Vertices[ pentagonIndex ][ 1 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  6
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 1 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  7
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 1 ], pentagonQuad_Vertices[ pentagonIndex ][ 0 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  8
	chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	pentagonIndex = pentagonIndex + 1;
  }

  int hexagonSkirtIndex = 0;
  hexagonIndex = 0;

  while ( hexagonIndex < hexagonCount ) { 

	//  0
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  1
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], hexagonQuad_Vertices[ hexagonIndex ][ 3 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  2
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 3 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  3
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 3 ], hexagonQuad_Vertices[ hexagonIndex ][ 2 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  4
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  5
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  6
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], hexagonQuad_Vertices[ hexagonIndex ][ 1 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  7
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 1 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  8
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 1 ], hexagonQuad_Vertices[ hexagonIndex ][ 0 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  9
	chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	hexagonIndex = hexagonIndex + 1;
  }

  //  PFM-NOTE: Cloned code to make the "punch-out" geometry - START

  //  PFM-NOTE: Here's the new geometry creation code 
  //  Code to build the pentagon facets
  pentagonIndex = 0;
  //int pentagonCount = 1;
  pentagonFacetIndex = 0;

  while ( pentagonIndex < pentagonCount ) {
  	chapeauCasting_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

  	chapeauCasting_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

  	chapeauCasting_A2_PentagonFacets[ pentagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonFacetIndex = pentagonFacetIndex + 1;

	pentagonIndex = pentagonIndex + 1;

  }

  hexagonIndex = 0;
  //int hexagonCount = 1;
  hexagonFacetIndex = 0;

  while ( hexagonIndex < hexagonCount ) {
  	chapeauCasting_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauCasting_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauCasting_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

  	chapeauCasting_A2_HexagonFacets[ hexagonFacetIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonFacetIndex = hexagonFacetIndex + 1;

	hexagonIndex = hexagonIndex + 1;

  }

  //if (config.integer("verbose") > 0) std::cout << "DetectorConstruction::DetectorConstruction: oink!" << std::endl;

  //  "Skirt" facets
  skirtIndex = 0;
  hexagonIndex = 10;

  while ( skirtIndex < 5 ) {
	chapeauCasting_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	hexagonIndex = hexagonIndex + 2;
	skirtIndex = skirtIndex + 1;
  }

  skirtIndex = 5;
  hexagonIndex = 11;

  //while ( skirtIndex < 10 ) {
  while ( skirtIndex < 20 ) {
//	if (config.integer("verbose") > 0) std::cout << "DetectorConstruction::DetectorConstruction: moo!" << std::endl;

	chapeauCasting_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	chapeauCasting_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	chapeauCasting_A2_SkirtFacets[ skirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
	skirtIndex = skirtIndex + 1;

	hexagonIndex = hexagonIndex + 2;
  }

  squareIndex = 0;
  squareCount = 10 * 20;
  //squareCount = 2 * 20;
  hexagonIndex = 0;

  while ( squareIndex < squareCount ) { 

// hexagonQuad_Vertices[ hexagonIndex ][ 0 ] 

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 4 ], hexagonQuad_Vertices[ hexagonIndex ][ 5 ], hexagonQuad_Vertices[ hexagonIndex ][ 6 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 6 ], hexagonQuad_Vertices[ hexagonIndex ][ 7 ], hexagonQuad_Vertices[ hexagonIndex ][ 4 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

//

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 0 ], hexagonQuad_Vertices[ hexagonIndex ][ 4 ], hexagonQuad_Vertices[ hexagonIndex ][ 3 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 1 ], hexagonQuad_Vertices[ hexagonIndex ][ 5 ], hexagonQuad_Vertices[ hexagonIndex ][ 0 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 2 ], hexagonQuad_Vertices[ hexagonIndex ][ 6 ], hexagonQuad_Vertices[ hexagonIndex ][ 1 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 3 ], hexagonQuad_Vertices[ hexagonIndex ][ 7 ], hexagonQuad_Vertices[ hexagonIndex ][ 2 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

//

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 4 ], hexagonQuad_Vertices[ hexagonIndex ][ 0 ], hexagonQuad_Vertices[ hexagonIndex ][ 5 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 5 ], hexagonQuad_Vertices[ hexagonIndex ][ 1 ], hexagonQuad_Vertices[ hexagonIndex ][ 6 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 6 ], hexagonQuad_Vertices[ hexagonIndex ][ 2 ], hexagonQuad_Vertices[ hexagonIndex ][ 7 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquareHexagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( hexagonQuad_Vertices[ hexagonIndex ][ 7 ], hexagonQuad_Vertices[ hexagonIndex ][ 3 ], hexagonQuad_Vertices[ hexagonIndex ][ 4 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	hexagonIndex = hexagonIndex + 1;
  }

  squareIndex = 0;
  squareCount = 10 * 12;
  //squareCount = 2 * 20;
  pentagonIndex = 0;

  while ( squareIndex < squareCount ) { 

// pentagonQuad_Vertices[ pentagonIndex ][ 0 ] 

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 4 ], pentagonQuad_Vertices[ pentagonIndex ][ 5 ], pentagonQuad_Vertices[ pentagonIndex ][ 6 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 6 ], pentagonQuad_Vertices[ pentagonIndex ][ 7 ], pentagonQuad_Vertices[ pentagonIndex ][ 4 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

//

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 0 ], pentagonQuad_Vertices[ pentagonIndex ][ 4 ], pentagonQuad_Vertices[ pentagonIndex ][ 3 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 1 ], pentagonQuad_Vertices[ pentagonIndex ][ 5 ], pentagonQuad_Vertices[ pentagonIndex ][ 0 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 2 ], pentagonQuad_Vertices[ pentagonIndex ][ 6 ], pentagonQuad_Vertices[ pentagonIndex ][ 1 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 3 ], pentagonQuad_Vertices[ pentagonIndex ][ 7 ], pentagonQuad_Vertices[ pentagonIndex ][ 2 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

//

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 4 ], pentagonQuad_Vertices[ pentagonIndex ][ 0 ], pentagonQuad_Vertices[ pentagonIndex ][ 5 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 5 ], pentagonQuad_Vertices[ pentagonIndex ][ 1 ], pentagonQuad_Vertices[ pentagonIndex ][ 6 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 6 ], pentagonQuad_Vertices[ pentagonIndex ][ 2 ], pentagonQuad_Vertices[ pentagonIndex ][ 7 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	chapeauCasting_A2_SquarePentagonExtensionFacets[ squareIndex ] = new G4TriangularFacet ( pentagonQuad_Vertices[ pentagonIndex ][ 7 ], pentagonQuad_Vertices[ pentagonIndex ][ 3 ], pentagonQuad_Vertices[ pentagonIndex ][ 4 ], ABSOLUTE );
	squareIndex = squareIndex + 1;

	pentagonIndex = pentagonIndex + 1;
  }

  pentagonSkirtIndex = 0;
  //int pentagonSkirtCount = 9;
  pentagonIndex = 0;

  while ( pentagonIndex < pentagonCount ) { 

	//  0
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  1
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], pentagonQuad_Vertices[ pentagonIndex ][ 3 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  2
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 3 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  3
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 3 ], pentagonQuad_Vertices[ pentagonIndex ][ 2 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  4
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 2 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  5
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 2 ], pentagonQuad_Vertices[ pentagonIndex ][ 1 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  6
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 1 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  7
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 1 ], pentagonQuad_Vertices[ pentagonIndex ][ 0 ], ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	//  8
	chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale, pentagonQuad_Vertices[ pentagonIndex ][ 0 ], chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	pentagonSkirtIndex = pentagonSkirtIndex + 1;

	pentagonIndex = pentagonIndex + 1;
  }

  hexagonSkirtIndex = 0;
  hexagonIndex = 0;

  while ( hexagonIndex < hexagonCount ) { 

	//  0
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  1
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], hexagonQuad_Vertices[ hexagonIndex ][ 3 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  2
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 5 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 3 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  3
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 3 ], hexagonQuad_Vertices[ hexagonIndex ][ 2 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  4
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  5
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  6
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 2 ], hexagonQuad_Vertices[ hexagonIndex ][ 1 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  7
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 1 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  8
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 1 ], hexagonQuad_Vertices[ hexagonIndex ][ 0 ], ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	//  9
	chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] = new G4TriangularFacet ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 1 ]] * chapeau_scale, hexagonQuad_Vertices[ hexagonIndex ][ 0 ], chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]] * chapeau_scale, ABSOLUTE );
	hexagonSkirtIndex = hexagonSkirtIndex + 1;

	hexagonIndex = hexagonIndex + 1;
  }

  //  PFM-NOTE: Cloned code to make the "punch-out" geometry - END

  chapeauPoof_H1A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );

  chapeauPoof_P1A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );

  chapeauPoof_H2A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H2A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauPoof_H3A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauPoof_H4A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauPoof_H5A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );


  //  H1B
  G4ThreeVector chapeauPoof_H1B_CoG = (( chapeau_Vertices[ 2 ]  * chapeau_scale ) + ( chapeau_Vertices[ 4 ] * chapeau_scale )) / 2.0;
  G4ThreeVector chapeauPoof_H1B_axis1 = ((( chapeau_Vertices[ 1 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 ] * chapeau_scale )) / 2.0 ) - chapeauPoof_H1B_CoG;
  G4ThreeVector chapeauPoof_H1B_axis2 = ( chapeau_Vertices[ 2 ] * chapeau_scale ) - chapeauPoof_H1B_CoG;
  chapeauPoof_H1B_axis1 = chapeauPoof_H1B_axis1 / chapeauPoof_H1B_axis1.mag();
  chapeauPoof_H1B_axis2 = chapeauPoof_H1B_axis2 / chapeauPoof_H1B_axis2.mag();
  G4double chapeau_CoG_scale = sipmR / knownR;
  //G4double chapeau_CoG_scale = 1.2;
  G4double hbHalf = 9.0 * mm;


  const G4ThreeVector chapeauPoof_H1B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H1B_axis1 + chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H1B_axis1 + chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H1B_axis1 - chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H1B_axis1 - chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale )),
	};

  const G4ThreeVector chapeauPoof_H1BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H1B_axis1 + chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H1B_axis1 + chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H1B_axis1 - chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H1B_axis1 - chapeauPoof_H1B_axis2 ) * hbHalf ) + ( chapeauPoof_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauPoof_H1B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 0 ], chapeauPoof_H1B_Vertices[ 1 ], chapeauPoof_H1B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 1 ], chapeauPoof_H1B_Vertices[ 2 ], chapeauPoof_H1B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 3 ], chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 3 ], chapeauPoof_H1B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 2 ], chapeau_Vertices[ 3 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 2 ], chapeauPoof_H1B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 1 ], chapeauPoof_H1B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 0 ], chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H1B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauPoof_H1B_Vertices[ 0 ], chapeauPoof_H1B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_H1B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 0 ], chapeauPoof_H1BX_Vertices[ 0 ], chapeauPoof_H1BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 3 ], chapeauPoof_H1B_Vertices[ 3 ], chapeauPoof_H1B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_H1B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 3 ], chapeauPoof_H1BX_Vertices[ 3 ], chapeauPoof_H1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 2 ], chapeauPoof_H1B_Vertices[ 2 ], chapeauPoof_H1B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_H1B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 2 ], chapeauPoof_H1BX_Vertices[ 2 ], chapeauPoof_H1BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 1 ], chapeauPoof_H1B_Vertices[ 1 ], chapeauPoof_H1B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_H1B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_H1B_Vertices[ 1 ], chapeauPoof_H1BX_Vertices[ 1 ], chapeauPoof_H1BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 0 ], chapeauPoof_H1B_Vertices[ 0 ], chapeauPoof_H1B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_H1B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 0 ], chapeauPoof_H1BX_Vertices[ 1 ], chapeauPoof_H1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H1B_Facets[ 21 ] = new G4TriangularFacet ( chapeauPoof_H1BX_Vertices[ 2 ], chapeauPoof_H1BX_Vertices[ 3 ], chapeauPoof_H1BX_Vertices[ 0 ], ABSOLUTE );

  //  H2B
  G4ThreeVector chapeauPoof_H2B_CoG = (( chapeau_Vertices[ 4 ] * chapeau_scale ) + ( chapeau_Vertices[ 8 + xVertexOffset ]  * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauPoof_H2B_axis1 = ((( chapeau_Vertices[ 8 ] * chapeau_scale ) + ( chapeau_Vertices[ 1 ] * chapeau_scale )) / 2.0 ) - chapeauPoof_H2B_CoG;
  G4ThreeVector chapeauPoof_H2B_axis2 = ( chapeau_Vertices[ 4 ] * chapeau_scale ) - chapeauPoof_H2B_CoG;
  chapeauPoof_H2B_axis1 = chapeauPoof_H2B_axis1 / chapeauPoof_H2B_axis1.mag();
  chapeauPoof_H2B_axis2 = chapeauPoof_H2B_axis2 / chapeauPoof_H2B_axis2.mag();

  const G4ThreeVector chapeauPoof_H2B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H2B_axis1 + chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H2B_axis1 + chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H2B_axis1 - chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H2B_axis1 - chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale ))
	};

  const G4ThreeVector chapeauPoof_H2BX_Vertices[ 4 ] = { 
 	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H2B_axis1 + chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H2B_axis1 + chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H2B_axis1 - chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H2B_axis1 - chapeauPoof_H2B_axis2 ) * hbHalf ) + ( chapeauPoof_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauPoof_H2B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 0 ], chapeauPoof_H2B_Vertices[ 1 ], chapeauPoof_H2B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 1 ], chapeauPoof_H2B_Vertices[ 2 ], chapeauPoof_H2B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 2 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 3 ],  ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ), ABSOLUTE );
  chapeauPoof_H2B_Facets[ 3 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 3 ], chapeauPoof_H2B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 4 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale ), ( chapeau_Vertices[ 8 + xVertexOffset ]  * chapeau_scale ), chapeauPoof_H2B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 5 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 2 ], ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), ABSOLUTE );
  chapeauPoof_H2B_Facets[ 6 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 2 ], chapeauPoof_H2B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 7 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 8 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 1 ], chapeauPoof_H2B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 9 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 0 ], ( chapeau_Vertices[ 1 ] * chapeau_scale ), ABSOLUTE );
  chapeauPoof_H2B_Facets[ 10 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 0 ], ( chapeau_Vertices[ 8 ] * chapeau_scale ), ABSOLUTE );
  chapeauPoof_H2B_Facets[ 11 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 ] * chapeau_scale ), chapeauPoof_H2B_Vertices[ 0 ], chapeauPoof_H2B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_H2B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 0 ], chapeauPoof_H2BX_Vertices[ 0 ], chapeauPoof_H2BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 3 ], chapeauPoof_H2B_Vertices[ 3 ], chapeauPoof_H2B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_H2B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 3 ], chapeauPoof_H2BX_Vertices[ 3 ], chapeauPoof_H2BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 2 ], chapeauPoof_H2B_Vertices[ 2 ], chapeauPoof_H2B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_H2B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 2 ], chapeauPoof_H2BX_Vertices[ 2 ], chapeauPoof_H2BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 1 ], chapeauPoof_H2B_Vertices[ 1 ], chapeauPoof_H2B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_H2B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_H2B_Vertices[ 1 ], chapeauPoof_H2BX_Vertices[ 1 ], chapeauPoof_H2BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 0 ], chapeauPoof_H2B_Vertices[ 0 ], chapeauPoof_H2B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_H2B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 0 ], chapeauPoof_H2BX_Vertices[ 1 ], chapeauPoof_H2BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H2B_Facets[ 21 ] = new G4TriangularFacet ( chapeauPoof_H2BX_Vertices[ 2 ], chapeauPoof_H2BX_Vertices[ 3 ], chapeauPoof_H2BX_Vertices[ 0 ], ABSOLUTE );

  //  H3B
  G4ThreeVector chapeauPoof_H3B_CoG = (( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) ) / 2.0;

  G4ThreeVector chapeauPoof_H3B_axis1 = ((( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 8 ] * chapeau_scale )) / 2.0 ) - chapeauPoof_H3B_CoG;
  G4ThreeVector chapeauPoof_H3B_axis2 = ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ) - chapeauPoof_H3B_CoG;
  chapeauPoof_H3B_axis1 = chapeauPoof_H3B_axis1 / chapeauPoof_H3B_axis1.mag();
  chapeauPoof_H3B_axis2 = chapeauPoof_H3B_axis2 / chapeauPoof_H3B_axis2.mag();

  const G4ThreeVector chapeauPoof_H3B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H3B_axis1 + chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H3B_axis1 + chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H3B_axis1 - chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H3B_axis1 - chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauPoof_H3BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H3B_axis1 + chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H3B_axis1 + chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H3B_axis1 - chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H3B_axis1 - chapeauPoof_H3B_axis2 ) * hbHalf ) + ( chapeauPoof_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauPoof_H3B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 0 ], chapeauPoof_H3B_Vertices[ 1 ], chapeauPoof_H3B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 1 ], chapeauPoof_H3B_Vertices[ 2 ], chapeauPoof_H3B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 3 ], chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 3 ], chapeauPoof_H3B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 2 ], chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 2 ], chapeauPoof_H3B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 1 ], chapeauPoof_H3B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 0 ], chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 0 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H3B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauPoof_H3B_Vertices[ 0 ], chapeauPoof_H3B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_H3B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 0 ], chapeauPoof_H3BX_Vertices[ 0 ], chapeauPoof_H3BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 3 ], chapeauPoof_H3B_Vertices[ 3 ], chapeauPoof_H3B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_H3B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 3 ], chapeauPoof_H3BX_Vertices[ 3 ], chapeauPoof_H3BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 2 ], chapeauPoof_H3B_Vertices[ 2 ], chapeauPoof_H3B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_H3B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 2 ], chapeauPoof_H3BX_Vertices[ 2 ], chapeauPoof_H3BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 1 ], chapeauPoof_H3B_Vertices[ 1 ], chapeauPoof_H3B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_H3B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_H3B_Vertices[ 1 ], chapeauPoof_H3BX_Vertices[ 1 ], chapeauPoof_H3BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 0 ], chapeauPoof_H3B_Vertices[ 0 ], chapeauPoof_H3B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_H3B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 0 ], chapeauPoof_H3BX_Vertices[ 1 ], chapeauPoof_H3BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H3B_Facets[ 21 ] = new G4TriangularFacet ( chapeauPoof_H3BX_Vertices[ 2 ], chapeauPoof_H3BX_Vertices[ 3 ], chapeauPoof_H3BX_Vertices[ 0 ], ABSOLUTE );

  //  H4B
  G4ThreeVector chapeauPoof_H4B_CoG = (( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauPoof_H4B_axis1 = ((( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale )) / 2.0 ) - chapeauPoof_H4B_CoG;
  G4ThreeVector chapeauPoof_H4B_axis2 = ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) - chapeauPoof_H4B_CoG;
  chapeauPoof_H4B_axis1 = chapeauPoof_H4B_axis1 / chapeauPoof_H4B_axis1.mag();
  chapeauPoof_H4B_axis2 = chapeauPoof_H4B_axis2 / chapeauPoof_H4B_axis2.mag();

  const G4ThreeVector chapeauPoof_H4B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H4B_axis1 + chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H4B_axis1 + chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H4B_axis1 - chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H4B_axis1 - chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauPoof_H4BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H4B_axis1 + chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H4B_axis1 + chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H4B_axis1 - chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H4B_axis1 - chapeauPoof_H4B_axis2 ) * hbHalf ) + ( chapeauPoof_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauPoof_H4B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 0 ], chapeauPoof_H4B_Vertices[ 1 ], chapeauPoof_H4B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 1 ], chapeauPoof_H4B_Vertices[ 2 ], chapeauPoof_H4B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 3 ], chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 3 ], chapeauPoof_H4B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 2 ], chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 2 ], chapeauPoof_H4B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 1 ], chapeauPoof_H4B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 0 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 0 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H4B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauPoof_H4B_Vertices[ 0 ], chapeauPoof_H4B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_H4B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 0 ], chapeauPoof_H4BX_Vertices[ 0 ], chapeauPoof_H4BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 3 ], chapeauPoof_H4B_Vertices[ 3 ], chapeauPoof_H4B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_H4B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 3 ], chapeauPoof_H4BX_Vertices[ 3 ], chapeauPoof_H4BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 2 ], chapeauPoof_H4B_Vertices[ 2 ], chapeauPoof_H4B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_H4B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 2 ], chapeauPoof_H4BX_Vertices[ 2 ], chapeauPoof_H4BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 1 ], chapeauPoof_H4B_Vertices[ 1 ], chapeauPoof_H4B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_H4B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_H4B_Vertices[ 1 ], chapeauPoof_H4BX_Vertices[ 1 ], chapeauPoof_H4BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 0 ], chapeauPoof_H4B_Vertices[ 0 ], chapeauPoof_H4B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_H4B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 0 ], chapeauPoof_H4BX_Vertices[ 1 ], chapeauPoof_H4BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H4B_Facets[ 21 ] = new G4TriangularFacet ( chapeauPoof_H4BX_Vertices[ 2 ], chapeauPoof_H4BX_Vertices[ 3 ], chapeauPoof_H4BX_Vertices[ 0 ], ABSOLUTE );

  //  H5B
  G4ThreeVector chapeauPoof_H5B_CoG = (( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 2 ] * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauPoof_H5B_axis1 = ((( chapeau_Vertices[ 5 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale )) / 2.0 ) - chapeauPoof_H5B_CoG;
  G4ThreeVector chapeauPoof_H5B_axis2 = ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale ) - chapeauPoof_H5B_CoG;
  chapeauPoof_H5B_axis1 = chapeauPoof_H5B_axis1 / chapeauPoof_H5B_axis1.mag();
  chapeauPoof_H5B_axis2 = chapeauPoof_H5B_axis2 / chapeauPoof_H5B_axis2.mag();

  const G4ThreeVector chapeauPoof_H5B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H5B_axis1 + chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H5B_axis1 + chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H5B_axis1 - chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H5B_axis1 - chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauPoof_H5BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H5B_axis1 + chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H5B_axis1 + chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_H5B_axis1 - chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_H5B_axis1 - chapeauPoof_H5B_axis2 ) * hbHalf ) + ( chapeauPoof_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauPoof_H5B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 0 ], chapeauPoof_H5B_Vertices[ 1 ], chapeauPoof_H5B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 1 ], chapeauPoof_H5B_Vertices[ 2 ], chapeauPoof_H5B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5  ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 3 ], chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 3 ], chapeauPoof_H5B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 2 ], chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 2 ], chapeauPoof_H5B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 1 ], chapeauPoof_H5B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 0 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_H5B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauPoof_H5B_Vertices[ 0 ], chapeauPoof_H5B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_H5B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 0 ], chapeauPoof_H5BX_Vertices[ 0 ], chapeauPoof_H5BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 3 ], chapeauPoof_H5B_Vertices[ 3 ], chapeauPoof_H5B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_H5B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 3 ], chapeauPoof_H5BX_Vertices[ 3 ], chapeauPoof_H5BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 2 ], chapeauPoof_H5B_Vertices[ 2 ], chapeauPoof_H5B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_H5B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 2 ], chapeauPoof_H5BX_Vertices[ 2 ], chapeauPoof_H5BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 1 ], chapeauPoof_H5B_Vertices[ 1 ], chapeauPoof_H5B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_H5B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_H5B_Vertices[ 1 ], chapeauPoof_H5BX_Vertices[ 1 ], chapeauPoof_H5BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 0 ], chapeauPoof_H5B_Vertices[ 0 ], chapeauPoof_H5B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_H5B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 0 ], chapeauPoof_H5BX_Vertices[ 1 ], chapeauPoof_H5BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_H5B_Facets[ 21 ] = new G4TriangularFacet ( chapeauPoof_H5BX_Vertices[ 2 ], chapeauPoof_H5BX_Vertices[ 3 ], chapeauPoof_H5BX_Vertices[ 0 ], ABSOLUTE );

  //  P1B
  G4ThreeVector chapeauPoof_P1B_CoG = (( chapeau_Vertices[ 1 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 8 ] * chapeau_scale )) / 5.0;

  //G4ThreeVector chapeauPoof_P1B_axis1 = (( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) - chapeauPoof_P1B_CoG ); 
  G4ThreeVector chapeauPoof_P1B_axis1 = (( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) - chapeauPoof_P1B_CoG ) * -1.0; 
  G4ThreeVector chapeauPoof_P1B_axis2 = (( chapeau_Vertices[ 5 ] * chapeau_scale ) - ( chapeau_Vertices[ 1 ] * chapeau_scale )) * -1.0;
  //G4ThreeVector chapeauPoof_P1B_axis2 = ( chapeau_Vertices[ 5 ] * chapeau_scale ) - ( chapeau_Vertices[ 1 ] * chapeau_scale );
  chapeauPoof_P1B_axis1 = chapeauPoof_P1B_axis1 / chapeauPoof_P1B_axis1.mag();
  chapeauPoof_P1B_axis2 = chapeauPoof_P1B_axis2 / chapeauPoof_P1B_axis2.mag();

  //  PFM-RESUME: confirm orientation of axis normals, then proceed with finishing geometry 
  G4double chapeau_CoG_scale_Pent = ( chapeauPoof_H1B_CoG.mag() * chapeau_CoG_scale ) / chapeauPoof_P1B_CoG.mag();

  const G4ThreeVector chapeauPoof_P1B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_P1B_axis1 + chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_P1B_axis1 + chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_P1B_axis1 - chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_P1B_axis1 - chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent ))
	};
  const G4ThreeVector chapeauPoof_P1BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_P1B_axis1 + chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_P1B_axis1 + chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauPoof_P1B_axis1 - chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauPoof_P1B_axis1 - chapeauPoof_P1B_axis2 ) * hbHalf ) + ( chapeauPoof_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale ))
	};

  chapeauPoof_P1B_Facets[ 0 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 0 ], chapeauPoof_P1B_Vertices[ 1 ], chapeauPoof_P1B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 1 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 1 ], chapeauPoof_P1B_Vertices[ 2 ], chapeauPoof_P1B_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 3 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 3 ], chapeauPoof_P1B_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 2 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 2 ], chapeauPoof_P1B_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 1 ], chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 1 ], chapeauPoof_P1B_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 0 ], chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauPoof_P1B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauPoof_P1B_Vertices[ 0 ], chapeauPoof_P1B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauPoof_P1B_Facets[ 11 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 0 ], chapeauPoof_P1BX_Vertices[ 0 ], chapeauPoof_P1BX_Vertices[ 3 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 12 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 3 ], chapeauPoof_P1B_Vertices[ 3 ], chapeauPoof_P1B_Vertices[ 0 ], ABSOLUTE );

  chapeauPoof_P1B_Facets[ 13 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 3 ], chapeauPoof_P1BX_Vertices[ 3 ], chapeauPoof_P1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 14 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 2 ], chapeauPoof_P1B_Vertices[ 2 ], chapeauPoof_P1B_Vertices[ 3 ], ABSOLUTE );

  chapeauPoof_P1B_Facets[ 15 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 2 ], chapeauPoof_P1BX_Vertices[ 2 ], chapeauPoof_P1BX_Vertices[ 1 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 16 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 1 ], chapeauPoof_P1B_Vertices[ 1 ], chapeauPoof_P1B_Vertices[ 2 ], ABSOLUTE );

  chapeauPoof_P1B_Facets[ 17 ] = new G4TriangularFacet ( chapeauPoof_P1B_Vertices[ 1 ], chapeauPoof_P1BX_Vertices[ 1 ], chapeauPoof_P1BX_Vertices[ 0 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 18 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 0 ], chapeauPoof_P1B_Vertices[ 0 ], chapeauPoof_P1B_Vertices[ 1 ], ABSOLUTE );

  chapeauPoof_P1B_Facets[ 19 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 0 ], chapeauPoof_P1BX_Vertices[ 1 ], chapeauPoof_P1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauPoof_P1B_Facets[ 20 ] = new G4TriangularFacet ( chapeauPoof_P1BX_Vertices[ 2 ], chapeauPoof_P1BX_Vertices[ 3 ], chapeauPoof_P1BX_Vertices[ 0 ], ABSOLUTE );


  chapeauPoof_A2 = new G4TessellatedSolid( "chapeauPoof_A2" );

  int pentagonFaceIndex = 0;
  while ( pentagonFaceIndex < 12 ) {

	//  PFM-HACK: This is gross but it's just for A2 so I can quibble about it later.
	if ( true ) {
	/*
	if (( pentagonFaceIndex == 0 ) ||
		( pentagonFaceIndex == 2 ) ||
		( pentagonFaceIndex == 4 ) ||
		( pentagonFaceIndex == 6 ) ||
		( pentagonFaceIndex == 8 ) ||
		( pentagonFaceIndex == 10 )) {
	*/
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 3 ) {
			//chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_PentagonFacets[ ( pentagonFaceIndex * 3 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 9 ) {
			chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquarePentagonSkirtFacets[ ( pentagonFaceIndex * 9 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 2 ) {
			chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquarePentagonFacets[ ( pentagonFaceIndex * 2 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
	}
	else {

	}

	pentagonFaceIndex = pentagonFaceIndex + 1;
  }

  int hexagonFaceIndex = 0;
  while ( hexagonFaceIndex < 20 ) {

	if ( true ) {
/*	if (( hexagonFaceIndex == 0 ) ||
		( hexagonFaceIndex == 2 ) ||
		( hexagonFaceIndex == 4 ) ||
		( hexagonFaceIndex == 6 ) ||
		( hexagonFaceIndex == 8 ) ||

		( hexagonFaceIndex == 10 ) ||
		( hexagonFaceIndex == 12 ) ||
		( hexagonFaceIndex == 14 ) ||
		( hexagonFaceIndex == 16 ) ||
		( hexagonFaceIndex == 18 ) ||

		( hexagonFaceIndex == 11 ) ||
		( hexagonFaceIndex == 13 ) ||
		( hexagonFaceIndex == 15 ) ||
		( hexagonFaceIndex == 17 ) ||
		( hexagonFaceIndex == 19 )) {
*/
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 4 ) {
			//chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_HexagonFacets[ ( hexagonFaceIndex * 4 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 10 ) {
			chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquareHexagonSkirtFacets[ ( hexagonFaceIndex * 10 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 2 ) {
			chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquareHexagonFacets[ ( hexagonFaceIndex * 2 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
	}
	else {

	}

	hexagonFaceIndex = hexagonFaceIndex + 1;
  }

/*
  pentagonSkirtIndex = 0;
  while ( pentagonSkirtIndex < ( 9 * 12 )) {
	chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] ));
	pentagonSkirtIndex = pentagonSkirtIndex + 1;
  }

  hexagonSkirtIndex = 0;
  while ( hexagonSkirtIndex < ( 10 * 20 )) {
	chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] ));
	hexagonSkirtIndex = hexagonSkirtIndex + 1;
  }
*/


//  PFM-NOTE: Turning this off (A2->B1) since they're no longer necessary 
/*
  skirtIndex = 0;
  //while ( skirtIndex < 5 ) {
  while ( skirtIndex < 20 ) {
	chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SkirtFacets[ skirtIndex ] ));
	skirtIndex = skirtIndex + 1;
  }
*/

/*  squareIndex = 0;
  while ( squareIndex < ( 2 * 20 )) {
	chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquareHexagonFacets[ squareIndex ] ));
	//squareIndex = squareIndex + 2;
	squareIndex = squareIndex + 1;
  }

  squareIndex = 0;
  while ( squareIndex < ( 2 * 12 )) {
	chapeauPoof_A2->AddFacet( (G4VFacet *)( chapeauPoof_A2_SquarePentagonFacets[ squareIndex ] ));
	squareIndex = squareIndex + 1;
  }
*/

  chapeauPoof_A2->SetSolidClosed( true );

  chapeauCasting_A2 = new G4TessellatedSolid( "chapeauCasting_A2" );

  pentagonFaceIndex = 0;
  while ( pentagonFaceIndex < 12 ) {

	//  PFM-HACK: This is gross but it's just for A2 so I can quibble about it later.
	if ( true ) {
	/*
	if (( pentagonFaceIndex == 0 ) ||
		( pentagonFaceIndex == 2 ) ||
		( pentagonFaceIndex == 4 ) ||
		( pentagonFaceIndex == 6 ) ||
		( pentagonFaceIndex == 8 ) ||
		( pentagonFaceIndex == 10 )) {
	*/
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 3 ) {
			//chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_PentagonFacets[ ( pentagonFaceIndex * 3 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 9 ) {
			chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquarePentagonSkirtFacets[ ( pentagonFaceIndex * 9 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
		pentagonFacetIndex = 0;
		while ( pentagonFacetIndex < 10 ) {
		//while ( pentagonFacetIndex < 2 ) {
			chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquarePentagonExtensionFacets[ ( pentagonFaceIndex * 10 ) + pentagonFacetIndex ] ));
			pentagonFacetIndex = pentagonFacetIndex + 1;

		}
	}
	else {

	}

	pentagonFaceIndex = pentagonFaceIndex + 1;
  }

  hexagonFaceIndex = 0;
  while ( hexagonFaceIndex < 20 ) {

	if ( true ) {
	/*
	if (( hexagonFaceIndex == 0 ) ||
		( hexagonFaceIndex == 2 ) ||
		( hexagonFaceIndex == 4 ) ||
		( hexagonFaceIndex == 6 ) ||
		( hexagonFaceIndex == 8 ) ||

		( hexagonFaceIndex == 10 ) ||
		( hexagonFaceIndex == 12 ) ||
		( hexagonFaceIndex == 14 ) ||
		( hexagonFaceIndex == 16 ) ||
		( hexagonFaceIndex == 18 ) ||

		( hexagonFaceIndex == 11 ) ||
		( hexagonFaceIndex == 13 ) ||
		( hexagonFaceIndex == 15 ) ||
		( hexagonFaceIndex == 17 ) ||
		( hexagonFaceIndex == 19 )) {
	*/
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 4 ) {
			//chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_HexagonFacets[ ( hexagonFaceIndex * 4 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 10 ) {
			chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquareHexagonSkirtFacets[ ( hexagonFaceIndex * 10 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
		hexagonFacetIndex = 0;
		while ( hexagonFacetIndex < 10 ) {
			chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquareHexagonExtensionFacets[ ( hexagonFaceIndex * 10 ) + hexagonFacetIndex ] ));
			hexagonFacetIndex = hexagonFacetIndex + 1;
		}
	}
	else {

	}

	hexagonFaceIndex = hexagonFaceIndex + 1;
  }

/*
  pentagonSkirtIndex = 0;
  while ( pentagonSkirtIndex < ( 9 * 12 )) {
	chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquarePentagonSkirtFacets[ pentagonSkirtIndex ] ));
	pentagonSkirtIndex = pentagonSkirtIndex + 1;
  }

  hexagonSkirtIndex = 0;
  while ( hexagonSkirtIndex < ( 10 * 20 )) {
	chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquareHexagonSkirtFacets[ hexagonSkirtIndex ] ));
	hexagonSkirtIndex = hexagonSkirtIndex + 1;
  }
*/


//  PFM-NOTE: Turning this off (A2->B1) since they're no longer necessary 
/*
  skirtIndex = 0;
  //while ( skirtIndex < 5 ) {
  while ( skirtIndex < 20 ) {
	chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SkirtFacets[ skirtIndex ] ));
	skirtIndex = skirtIndex + 1;
  }
*/

/*  squareIndex = 0;
  while ( squareIndex < ( 2 * 20 )) {
	chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquareHexagonFacets[ squareIndex ] ));
	//squareIndex = squareIndex + 2;
	squareIndex = squareIndex + 1;
  }

  squareIndex = 0;
  while ( squareIndex < ( 2 * 12 )) {
	chapeauCasting_A2->AddFacet( (G4VFacet *)( chapeauCasting_A2_SquarePentagonFacets[ squareIndex ] ));
	squareIndex = squareIndex + 1;
  }
*/
  chapeauCasting_A2->SetSolidClosed( true );

  chapeauPoof = new G4TessellatedSolid( "chapeauPoof" );

  //  H1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 0 ] )); // H5A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 3 ] ));
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 4 ] )); // H2A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 5 ] )); // P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 6 ] )); // H1B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 7 ] )); // H1B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 8 ] )); // H1B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1A_Facets[ 9 ] )); // H1B

  //  P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 0 ] )); // H3A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 1 ] )); // H4A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 2 ] )); // H5A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 3 ] )); // H1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 4 ] )); // H2A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 5 ] )); // P1B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 6 ] )); // P1B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1A_Facets[ 7 ] )); // P1B

  //  H2A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 0 ] )); // H3A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 1 ] )); // P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 2 ] )); // H1A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 4 ] )); 
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 5 ] ));
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 6 ] )); // H2B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 7 ] )); // H2B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 8 ] )); // H2B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2A_Facets[ 9 ] )); // H2B

  //  H3A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 1 ] )); 
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 2 ] )); // H4A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 3 ] )); // P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 4 ] )); // H2A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 5 ] ));
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 6 ] )); // H3B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 7 ] )); // H3B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 8 ] )); // H3B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3A_Facets[ 9 ] )); // H3B

  //  H4A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 0 ] )); // H3A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 1 ] )); 
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 2 ] )); 
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 3 ] ));
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 4 ] )); // H5A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 5 ] )); // P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 6 ] )); // H4B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 7 ] )); // H4B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 8 ] )); // H4B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4A_Facets[ 9 ] )); // H4B

  //  H5A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 0 ] )); // P1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 1 ] )); // H4A
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 2 ] )); 
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 4 ] )); 
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 5 ] )); // H1A
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 6 ] )); // H5B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 7 ] )); // H5B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 8 ] )); // H5B
  //chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5A_Facets[ 9 ] )); // H5B

  //  H1B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 10 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H1B_Facets[ 11 ] ));

  //  H2B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 10 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H2B_Facets[ 11 ] ));

  //  H3B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 10 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H3B_Facets[ 11 ] ));

  //  H4B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 10 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H4B_Facets[ 11 ] ));

  //  H5B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 10 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_H5B_Facets[ 11 ] ));

  //  P1B
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 0 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 1 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 2 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 3 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 4 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 5 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 6 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 7 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 8 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 9 ] ));
  chapeauPoof->AddFacet( (G4VFacet *)( chapeauPoof_P1B_Facets[ 10 ] ));
  chapeauPoof->SetSolidClosed( true );

  chapeauCasting_H1A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );

  chapeauCasting_P1A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );

  chapeauCasting_H2A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H2A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauCasting_H3A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 8 ] * chapeau_scale, chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauCasting_H4A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );

  chapeauCasting_H5A_Facets[ 0 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 1 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, zero_Vertices[ 0 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5A_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );


  //  H1B
  G4ThreeVector chapeauCasting_H1B_CoG = (( chapeau_Vertices[ 2 ]  * chapeau_scale ) + ( chapeau_Vertices[ 4 ] * chapeau_scale )) / 2.0;
  G4ThreeVector chapeauCasting_H1B_axis1 = ((( chapeau_Vertices[ 1 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 ] * chapeau_scale )) / 2.0 ) - chapeauCasting_H1B_CoG;
  G4ThreeVector chapeauCasting_H1B_axis2 = ( chapeau_Vertices[ 2 ] * chapeau_scale ) - chapeauCasting_H1B_CoG;
  chapeauCasting_H1B_axis1 = chapeauCasting_H1B_axis1 / chapeauCasting_H1B_axis1.mag();
  chapeauCasting_H1B_axis2 = chapeauCasting_H1B_axis2 / chapeauCasting_H1B_axis2.mag();


  const G4ThreeVector chapeauCasting_H1B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H1B_axis1 + chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H1B_axis1 + chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H1B_axis1 - chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H1B_axis1 - chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale )),
	};

  const G4ThreeVector chapeauCasting_H1BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H1B_axis1 + chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H1B_axis1 + chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H1B_axis1 - chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H1B_axis1 - chapeauCasting_H1B_axis2 ) * hbHalf ) + ( chapeauCasting_H1B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauCasting_H1B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 0 ], chapeauCasting_H1B_Vertices[ 1 ], chapeauCasting_H1B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 1 ], chapeauCasting_H1B_Vertices[ 2 ], chapeauCasting_H1B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 3 ], chapeau_Vertices[ 4 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 3 ], chapeauCasting_H1B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeau_Vertices[ 4 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 2 ], chapeau_Vertices[ 3 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 2 ], chapeauCasting_H1B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeau_Vertices[ 3 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 1 ], chapeauCasting_H1B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 0 ], chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H1B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauCasting_H1B_Vertices[ 0 ], chapeauCasting_H1B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_H1B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 0 ], chapeauCasting_H1BX_Vertices[ 0 ], chapeauCasting_H1BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 3 ], chapeauCasting_H1B_Vertices[ 3 ], chapeauCasting_H1B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_H1B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 3 ], chapeauCasting_H1BX_Vertices[ 3 ], chapeauCasting_H1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 2 ], chapeauCasting_H1B_Vertices[ 2 ], chapeauCasting_H1B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_H1B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 2 ], chapeauCasting_H1BX_Vertices[ 2 ], chapeauCasting_H1BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 1 ], chapeauCasting_H1B_Vertices[ 1 ], chapeauCasting_H1B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_H1B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_H1B_Vertices[ 1 ], chapeauCasting_H1BX_Vertices[ 1 ], chapeauCasting_H1BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 0 ], chapeauCasting_H1B_Vertices[ 0 ], chapeauCasting_H1B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_H1B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 0 ], chapeauCasting_H1BX_Vertices[ 1 ], chapeauCasting_H1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H1B_Facets[ 21 ] = new G4TriangularFacet ( chapeauCasting_H1BX_Vertices[ 2 ], chapeauCasting_H1BX_Vertices[ 3 ], chapeauCasting_H1BX_Vertices[ 0 ], ABSOLUTE );

  //  H2B
  G4ThreeVector chapeauCasting_H2B_CoG = (( chapeau_Vertices[ 4 ] * chapeau_scale ) + ( chapeau_Vertices[ 8 + xVertexOffset ]  * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauCasting_H2B_axis1 = ((( chapeau_Vertices[ 8 ] * chapeau_scale ) + ( chapeau_Vertices[ 1 ] * chapeau_scale )) / 2.0 ) - chapeauCasting_H2B_CoG;
  G4ThreeVector chapeauCasting_H2B_axis2 = ( chapeau_Vertices[ 4 ] * chapeau_scale ) - chapeauCasting_H2B_CoG;
  chapeauCasting_H2B_axis1 = chapeauCasting_H2B_axis1 / chapeauCasting_H2B_axis1.mag();
  chapeauCasting_H2B_axis2 = chapeauCasting_H2B_axis2 / chapeauCasting_H2B_axis2.mag();

  const G4ThreeVector chapeauCasting_H2B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H2B_axis1 + chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H2B_axis1 + chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H2B_axis1 - chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H2B_axis1 - chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale ))
	};

  const G4ThreeVector chapeauCasting_H2BX_Vertices[ 4 ] = { 
 	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H2B_axis1 + chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H2B_axis1 + chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H2B_axis1 - chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H2B_axis1 - chapeauCasting_H2B_axis2 ) * hbHalf ) + ( chapeauCasting_H2B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauCasting_H2B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 0 ], chapeauCasting_H2B_Vertices[ 1 ], chapeauCasting_H2B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 1 ], chapeauCasting_H2B_Vertices[ 2 ], chapeauCasting_H2B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 2 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 3 ],  ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ), ABSOLUTE );
  chapeauCasting_H2B_Facets[ 3 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 3 ], chapeauCasting_H2B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 4 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale ), ( chapeau_Vertices[ 8 + xVertexOffset ]  * chapeau_scale ), chapeauCasting_H2B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 5 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 + xVertexOffset ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 2 ], ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), ABSOLUTE );
  chapeauCasting_H2B_Facets[ 6 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 2 ], chapeauCasting_H2B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 7 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), ( chapeau_Vertices[ 4 + xVertexOffset ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 8 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 1 ], chapeauCasting_H2B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 9 ] = new G4TriangularFacet (( chapeau_Vertices[ 4 ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 0 ], ( chapeau_Vertices[ 1 ] * chapeau_scale ), ABSOLUTE );
  chapeauCasting_H2B_Facets[ 10 ] = new G4TriangularFacet (( chapeau_Vertices[ 1 ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 0 ], ( chapeau_Vertices[ 8 ] * chapeau_scale ), ABSOLUTE );
  chapeauCasting_H2B_Facets[ 11 ] = new G4TriangularFacet (( chapeau_Vertices[ 8 ] * chapeau_scale ), chapeauCasting_H2B_Vertices[ 0 ], chapeauCasting_H2B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_H2B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 0 ], chapeauCasting_H2BX_Vertices[ 0 ], chapeauCasting_H2BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 3 ], chapeauCasting_H2B_Vertices[ 3 ], chapeauCasting_H2B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_H2B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 3 ], chapeauCasting_H2BX_Vertices[ 3 ], chapeauCasting_H2BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 2 ], chapeauCasting_H2B_Vertices[ 2 ], chapeauCasting_H2B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_H2B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 2 ], chapeauCasting_H2BX_Vertices[ 2 ], chapeauCasting_H2BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 1 ], chapeauCasting_H2B_Vertices[ 1 ], chapeauCasting_H2B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_H2B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_H2B_Vertices[ 1 ], chapeauCasting_H2BX_Vertices[ 1 ], chapeauCasting_H2BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 0 ], chapeauCasting_H2B_Vertices[ 0 ], chapeauCasting_H2B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_H2B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 0 ], chapeauCasting_H2BX_Vertices[ 1 ], chapeauCasting_H2BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H2B_Facets[ 21 ] = new G4TriangularFacet ( chapeauCasting_H2BX_Vertices[ 2 ], chapeauCasting_H2BX_Vertices[ 3 ], chapeauCasting_H2BX_Vertices[ 0 ], ABSOLUTE );

  //  H3B
  G4ThreeVector chapeauCasting_H3B_CoG = (( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) ) / 2.0;

  G4ThreeVector chapeauCasting_H3B_axis1 = ((( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 8 ] * chapeau_scale )) / 2.0 ) - chapeauCasting_H3B_CoG;
  G4ThreeVector chapeauCasting_H3B_axis2 = ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale ) - chapeauCasting_H3B_CoG;
  chapeauCasting_H3B_axis1 = chapeauCasting_H3B_axis1 / chapeauCasting_H3B_axis1.mag();
  chapeauCasting_H3B_axis2 = chapeauCasting_H3B_axis2 / chapeauCasting_H3B_axis2.mag();

  const G4ThreeVector chapeauCasting_H3B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H3B_axis1 + chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H3B_axis1 + chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H3B_axis1 - chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H3B_axis1 - chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauCasting_H3BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H3B_axis1 + chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H3B_axis1 + chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H3B_axis1 - chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H3B_axis1 - chapeauCasting_H3B_axis2 ) * hbHalf ) + ( chapeauCasting_H3B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauCasting_H3B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 0 ], chapeauCasting_H3B_Vertices[ 1 ], chapeauCasting_H3B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 1 ], chapeauCasting_H3B_Vertices[ 2 ], chapeauCasting_H3B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 3 ], chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 3 ], chapeauCasting_H3B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + xyVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 2 ], chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 2 ], chapeauCasting_H3B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeau_Vertices[ 1 + xyVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 1 ], chapeauCasting_H3B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 + xVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 0 ], chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 0 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H3B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauCasting_H3B_Vertices[ 0 ], chapeauCasting_H3B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_H3B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 0 ], chapeauCasting_H3BX_Vertices[ 0 ], chapeauCasting_H3BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 3 ], chapeauCasting_H3B_Vertices[ 3 ], chapeauCasting_H3B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_H3B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 3 ], chapeauCasting_H3BX_Vertices[ 3 ], chapeauCasting_H3BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 2 ], chapeauCasting_H3B_Vertices[ 2 ], chapeauCasting_H3B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_H3B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 2 ], chapeauCasting_H3BX_Vertices[ 2 ], chapeauCasting_H3BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 1 ], chapeauCasting_H3B_Vertices[ 1 ], chapeauCasting_H3B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_H3B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_H3B_Vertices[ 1 ], chapeauCasting_H3BX_Vertices[ 1 ], chapeauCasting_H3BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 0 ], chapeauCasting_H3B_Vertices[ 0 ], chapeauCasting_H3B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_H3B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 0 ], chapeauCasting_H3BX_Vertices[ 1 ], chapeauCasting_H3BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H3B_Facets[ 21 ] = new G4TriangularFacet ( chapeauCasting_H3BX_Vertices[ 2 ], chapeauCasting_H3BX_Vertices[ 3 ], chapeauCasting_H3BX_Vertices[ 0 ], ABSOLUTE );

  //  H4B
  G4ThreeVector chapeauCasting_H4B_CoG = (( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauCasting_H4B_axis1 = ((( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale )) / 2.0 ) - chapeauCasting_H4B_CoG;
  G4ThreeVector chapeauCasting_H4B_axis2 = ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale ) - chapeauCasting_H4B_CoG;
  chapeauCasting_H4B_axis1 = chapeauCasting_H4B_axis1 / chapeauCasting_H4B_axis1.mag();
  chapeauCasting_H4B_axis2 = chapeauCasting_H4B_axis2 / chapeauCasting_H4B_axis2.mag();

  const G4ThreeVector chapeauCasting_H4B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H4B_axis1 + chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H4B_axis1 + chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H4B_axis1 - chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H4B_axis1 - chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauCasting_H4BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H4B_axis1 + chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H4B_axis1 + chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H4B_axis1 - chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H4B_axis1 - chapeauCasting_H4B_axis2 ) * hbHalf ) + ( chapeauCasting_H4B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauCasting_H4B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 0 ], chapeauCasting_H4B_Vertices[ 1 ], chapeauCasting_H4B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 1 ], chapeauCasting_H4B_Vertices[ 2 ], chapeauCasting_H4B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 3 ], chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 3 ], chapeauCasting_H4B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 3 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 2 ], chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 2 ], chapeauCasting_H4B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 0 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 1 ], chapeauCasting_H4B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 4 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 0 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 0 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H4B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauCasting_H4B_Vertices[ 0 ], chapeauCasting_H4B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_H4B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 0 ], chapeauCasting_H4BX_Vertices[ 0 ], chapeauCasting_H4BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 3 ], chapeauCasting_H4B_Vertices[ 3 ], chapeauCasting_H4B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_H4B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 3 ], chapeauCasting_H4BX_Vertices[ 3 ], chapeauCasting_H4BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 2 ], chapeauCasting_H4B_Vertices[ 2 ], chapeauCasting_H4B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_H4B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 2 ], chapeauCasting_H4BX_Vertices[ 2 ], chapeauCasting_H4BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 1 ], chapeauCasting_H4B_Vertices[ 1 ], chapeauCasting_H4B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_H4B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_H4B_Vertices[ 1 ], chapeauCasting_H4BX_Vertices[ 1 ], chapeauCasting_H4BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 0 ], chapeauCasting_H4B_Vertices[ 0 ], chapeauCasting_H4B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_H4B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 0 ], chapeauCasting_H4BX_Vertices[ 1 ], chapeauCasting_H4BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H4B_Facets[ 21 ] = new G4TriangularFacet ( chapeauCasting_H4BX_Vertices[ 2 ], chapeauCasting_H4BX_Vertices[ 3 ], chapeauCasting_H4BX_Vertices[ 0 ], ABSOLUTE );

  //  H5B
  G4ThreeVector chapeauCasting_H5B_CoG = (( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 2 ] * chapeau_scale )) / 2.0;

  G4ThreeVector chapeauCasting_H5B_axis1 = ((( chapeau_Vertices[ 5 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale )) / 2.0 ) - chapeauCasting_H5B_CoG;
  G4ThreeVector chapeauCasting_H5B_axis2 = ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale ) - chapeauCasting_H5B_CoG;
  chapeauCasting_H5B_axis1 = chapeauCasting_H5B_axis1 / chapeauCasting_H5B_axis1.mag();
  chapeauCasting_H5B_axis2 = chapeauCasting_H5B_axis2 / chapeauCasting_H5B_axis2.mag();

  const G4ThreeVector chapeauCasting_H5B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H5B_axis1 + chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H5B_axis1 + chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H5B_axis1 - chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H5B_axis1 - chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale ))
	};
  const G4ThreeVector chapeauCasting_H5BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H5B_axis1 + chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H5B_axis1 + chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_H5B_axis1 - chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_H5B_axis1 - chapeauCasting_H5B_axis2 ) * hbHalf ) + ( chapeauCasting_H5B_CoG * chapeau_CoG_scale * chapeau_casting_scale ))
	};

  chapeauCasting_H5B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 0 ], chapeauCasting_H5B_Vertices[ 1 ], chapeauCasting_H5B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 1 ], chapeauCasting_H5B_Vertices[ 2 ], chapeauCasting_H5B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5  ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 3 ], chapeau_Vertices[ 2 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 3 ], chapeauCasting_H5B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeau_Vertices[ 2 ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 2 ], chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 2 ], chapeauCasting_H5B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeau_Vertices[ 6 + yVertexOffset ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 1 ], chapeauCasting_H5B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 2 + yVertexOffset ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 0 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_H5B_Facets[ 11 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauCasting_H5B_Vertices[ 0 ], chapeauCasting_H5B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_H5B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 0 ], chapeauCasting_H5BX_Vertices[ 0 ], chapeauCasting_H5BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 3 ], chapeauCasting_H5B_Vertices[ 3 ], chapeauCasting_H5B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_H5B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 3 ], chapeauCasting_H5BX_Vertices[ 3 ], chapeauCasting_H5BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 2 ], chapeauCasting_H5B_Vertices[ 2 ], chapeauCasting_H5B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_H5B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 2 ], chapeauCasting_H5BX_Vertices[ 2 ], chapeauCasting_H5BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 1 ], chapeauCasting_H5B_Vertices[ 1 ], chapeauCasting_H5B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_H5B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_H5B_Vertices[ 1 ], chapeauCasting_H5BX_Vertices[ 1 ], chapeauCasting_H5BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 0 ], chapeauCasting_H5B_Vertices[ 0 ], chapeauCasting_H5B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_H5B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 0 ], chapeauCasting_H5BX_Vertices[ 1 ], chapeauCasting_H5BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_H5B_Facets[ 21 ] = new G4TriangularFacet ( chapeauCasting_H5BX_Vertices[ 2 ], chapeauCasting_H5BX_Vertices[ 3 ], chapeauCasting_H5BX_Vertices[ 0 ], ABSOLUTE );

  //  P1B
  G4ThreeVector chapeauCasting_P1B_CoG = (( chapeau_Vertices[ 1 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 ] * chapeau_scale ) + ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) + ( chapeau_Vertices[ 8 ] * chapeau_scale )) / 5.0;

  //G4ThreeVector chapeauCasting_P1B_axis1 = (( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) - chapeauCasting_P1B_CoG ); 
  G4ThreeVector chapeauCasting_P1B_axis1 = (( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale ) - chapeauCasting_P1B_CoG ) * -1.0; 
  G4ThreeVector chapeauCasting_P1B_axis2 = (( chapeau_Vertices[ 5 ] * chapeau_scale ) - ( chapeau_Vertices[ 1 ] * chapeau_scale )) * -1.0;
  //G4ThreeVector chapeauCasting_P1B_axis2 = ( chapeau_Vertices[ 5 ] * chapeau_scale ) - ( chapeau_Vertices[ 1 ] * chapeau_scale );
  chapeauCasting_P1B_axis1 = chapeauCasting_P1B_axis1 / chapeauCasting_P1B_axis1.mag();
  chapeauCasting_P1B_axis2 = chapeauCasting_P1B_axis2 / chapeauCasting_P1B_axis2.mag();

  //  PFM-RESUME: confirm orientation of axis normals, then proceed with finishing geometry 
  //G4double chapeauCasting_CoG_scale_Pent = ( chapeauCasting_H1B_CoG.mag() * chapeau_CoG_scale ) / chapeauCasting_P1B_CoG.mag();

  const G4ThreeVector chapeauCasting_P1B_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_P1B_axis1 + chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_P1B_axis1 + chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_P1B_axis1 - chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_P1B_axis1 - chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent ))
	};
  const G4ThreeVector chapeauCasting_P1BX_Vertices[ 4 ] = { 
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_P1B_axis1 + chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_P1B_axis1 + chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] - chapeauCasting_P1B_axis1 - chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale )),
	G4ThreeVector((( zero_Vertices[ 0 ] + chapeauCasting_P1B_axis1 - chapeauCasting_P1B_axis2 ) * hbHalf ) + ( chapeauCasting_P1B_CoG * chapeau_CoG_scale_Pent * chapeau_casting_scale ))
	};

  chapeauCasting_P1B_Facets[ 0 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 0 ], chapeauCasting_P1B_Vertices[ 1 ], chapeauCasting_P1B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 1 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 1 ], chapeauCasting_P1B_Vertices[ 2 ], chapeauCasting_P1B_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 2 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 3 ], chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1B_Facets[ 3 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 3 ], chapeauCasting_P1B_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 4 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 + yVertexOffset ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 2 ], chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1B_Facets[ 5 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 2 ], chapeauCasting_P1B_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 6 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 + yVertexOffset ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 1 ], chapeau_Vertices[ 8 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1B_Facets[ 7 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 1 ], chapeauCasting_P1B_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 8 ] = new G4TriangularFacet ( chapeau_Vertices[ 8 ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 0 ], chapeau_Vertices[ 1 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1B_Facets[ 9 ] = new G4TriangularFacet ( chapeau_Vertices[ 1 ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 0 ], chapeau_Vertices[ 5 ] * chapeau_scale, ABSOLUTE );
  chapeauCasting_P1B_Facets[ 10 ] = new G4TriangularFacet ( chapeau_Vertices[ 5 ] * chapeau_scale, chapeauCasting_P1B_Vertices[ 0 ], chapeauCasting_P1B_Vertices[ 3 ], ABSOLUTE );

  //  casting facets
  chapeauCasting_P1B_Facets[ 11 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 0 ], chapeauCasting_P1BX_Vertices[ 0 ], chapeauCasting_P1BX_Vertices[ 3 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 12 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 3 ], chapeauCasting_P1B_Vertices[ 3 ], chapeauCasting_P1B_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting_P1B_Facets[ 13 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 3 ], chapeauCasting_P1BX_Vertices[ 3 ], chapeauCasting_P1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 14 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 2 ], chapeauCasting_P1B_Vertices[ 2 ], chapeauCasting_P1B_Vertices[ 3 ], ABSOLUTE );

  chapeauCasting_P1B_Facets[ 15 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 2 ], chapeauCasting_P1BX_Vertices[ 2 ], chapeauCasting_P1BX_Vertices[ 1 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 16 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 1 ], chapeauCasting_P1B_Vertices[ 1 ], chapeauCasting_P1B_Vertices[ 2 ], ABSOLUTE );

  chapeauCasting_P1B_Facets[ 17 ] = new G4TriangularFacet ( chapeauCasting_P1B_Vertices[ 1 ], chapeauCasting_P1BX_Vertices[ 1 ], chapeauCasting_P1BX_Vertices[ 0 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 18 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 0 ], chapeauCasting_P1B_Vertices[ 0 ], chapeauCasting_P1B_Vertices[ 1 ], ABSOLUTE );

  chapeauCasting_P1B_Facets[ 19 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 0 ], chapeauCasting_P1BX_Vertices[ 1 ], chapeauCasting_P1BX_Vertices[ 2 ], ABSOLUTE );
  chapeauCasting_P1B_Facets[ 20 ] = new G4TriangularFacet ( chapeauCasting_P1BX_Vertices[ 2 ], chapeauCasting_P1BX_Vertices[ 3 ], chapeauCasting_P1BX_Vertices[ 0 ], ABSOLUTE );

  chapeauCasting = new G4TessellatedSolid( "chapeauCasting" );

  //  H1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 0 ] )); // H5A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 3 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 4 ] )); // H2A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 5 ] )); // P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 6 ] )); // H1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 7 ] )); // H1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 8 ] )); // H1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1A_Facets[ 9 ] )); // H1B

  //  P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 0 ] )); // H3A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 1 ] )); // H4A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 2 ] )); // H5A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 3 ] )); // H1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 4 ] )); // H2A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 5 ] )); // P1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 6 ] )); // P1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1A_Facets[ 7 ] )); // P1B

  //  H2A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 0 ] )); // H3A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 1 ] )); // P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 2 ] )); // H1A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 4 ] )); 
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 5 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 6 ] )); // H2B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 7 ] )); // H2B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 8 ] )); // H2B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2A_Facets[ 9 ] )); // H2B

  //  H3A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 0 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 1 ] )); 
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 2 ] )); // H4A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 3 ] )); // P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 4 ] )); // H2A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 5 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 6 ] )); // H3B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 7 ] )); // H3B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 8 ] )); // H3B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3A_Facets[ 9 ] )); // H3B

  //  H4A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 0 ] )); // H3A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 1 ] )); 
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 2 ] )); 
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 3 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 4 ] )); // H5A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 5 ] )); // P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 6 ] )); // H4B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 7 ] )); // H4B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 8 ] )); // H4B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4A_Facets[ 9 ] )); // H4B

  //  H5A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 0 ] )); // P1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 1 ] )); // H4A
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 2 ] )); 
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 4 ] )); 
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 5 ] )); // H1A
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 6 ] )); // H5B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 7 ] )); // H5B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 8 ] )); // H5B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5A_Facets[ 9 ] )); // H5B

  //  H1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 10 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 11 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 20 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H1B_Facets[ 21 ] ));

  //  H2B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 10 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 11 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 20 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H2B_Facets[ 21 ] ));

  //  H3B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 10 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 11 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 20 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H3B_Facets[ 21 ] ));

  //  H4B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 10 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 11 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 20 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H4B_Facets[ 21 ] ));

  //  H5B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 10 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 11 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 20 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_H5B_Facets[ 21 ] ));

  //  P1B
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 0 ] ));
  //chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 1 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 2 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 3 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 4 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 5 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 6 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 7 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 8 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 9 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 10 ] ));

  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 11 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 12 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 13 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 14 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 15 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 16 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 17 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 18 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 19 ] ));
  chapeauCasting->AddFacet( (G4VFacet *)( chapeauCasting_P1B_Facets[ 20 ] ));

  chapeauCasting->SetSolidClosed( true );

  //  PFM-NOTE: Debugging for new geomtry, will be similar disabled as A1 
  //chapeauPoof_A2_LV = new G4LogicalVolume( chapeauPoof_A2, MaterialFactory::getInstance()->getEpoxy(), "chapeauPoof_A2", 0, 0, 0 );
  //chapeauPoof_A2_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 1.0)));
  //chapeauPoof_A2_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm), chapeauPoof_A2_LV, "chapeauPoof_A2_PV", world_LV, false, 0 );

  //chapeauPoof_LV = new G4LogicalVolume( chapeauPoof, MaterialFactory::getInstance()->getEpoxy(), "chapeauPoof", 0, 0, 0 );
  //chapeauPoof_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 1.0)));
  //chapeauPoof_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , chapeauPoof_LV, "chapeauPoof_PV", world_LV, false, 0 );



  chapeauPoofMinusHomeOrb = new G4SubtractionSolid( "chapeauPoofMinusHomeOrb", chapeauPoof_A2, lg_orb_void ); 
  //chapeauPoofMinusHomeOrb = new G4SubtractionSolid( "chapeauPoofMinusHomeOrb", chapeauPoof, lg_orb_void ); 
  chapeauPoofMinusHomeOrb_LV = new G4LogicalVolume( chapeauPoofMinusHomeOrb, MaterialFactory::getInstance()->getEpoxy(), "chapeauPoofMinusHomeOrb", 0, 0, 0 );
  //chapeauPoofMinusHomeOrb_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 0.9)));
  chapeauPoofMinusHomeOrb_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 1.0)));
  G4RotationMatrix* chapeauPoofMinusHomeOrbMatrix = new G4RotationMatrix();
  chapeauPoofMinusHomeOrbMatrix->rotateX( 0.0 * deg );	
  //chapeauPoofMinusHomeOrbMatrix->rotateX( -90.0 * deg );  //  original transformation	

  //  PFM-NOTE: Disabled due to geometry revision from A1->A2
  chapeauPoofMinusHomeOrb_PV = new G4PVPlacement( chapeauPoofMinusHomeOrbMatrix, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , chapeauPoofMinusHomeOrb_LV, "chapeauPoofMinusHomeOrb_PV", world_LV, false, 0 );  //  main piece to re-enable 

  //chapeauPoofMinusHomeOrb_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , chapeauPoofMinusHomeOrb_LV, "chapeauPoofMinusHomeOrb_PV", world_LV, false, 0 );


  chapeauCasting_LV = new G4LogicalVolume( chapeauCasting_A2, MaterialFactory::getInstance()->getEpoxy(), "chapeauCasting_A2", 0, 0, 0 );
  chapeauCasting_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 1.0)));
  //chapeauCasting_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , chapeauCasting_LV, "chapeauCasting_PV", world_LV, false, 0 );

  //homeSphere = new G4Sphere( "homeSphere", hemisphereRadiusOuterScaled, chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 90.0 * deg );
  homeSphere = new G4Sphere( "homeSphere", ( chapeau_Vertices[ 0 ] * chapeau_scale ).mag(), chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 180.0 * deg );  //  A2
  //homeSphere = new G4Sphere( "homeSphere", ( chapeau_Vertices[ 0 ] * chapeau_scale ).mag(), chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 135.0 * deg );  //  A2
  //homeSphere = new G4Sphere( "homeSphere", ( chapeau_Vertices[ 0 ] * chapeau_scale ).mag(), chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 90.0 * deg );
  //homeSphere = new G4Sphere( "homeSphere", ( chapeau_Vertices[ 0 ] * chapeau_scale ).mag(), chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 90.0 * deg );
  //homeSphere = new G4Sphere( "homeSphere", chapeau_Vertices[ 0 ].mag(), chapeauCasting_H1B_Vertices[ 0 ].mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 90.0 * deg );
  //homeSphere_LV = new G4LogicalVolume( homeSphere, MaterialFactory::getInstance()->getEpoxy(), "homeSphere_LV", 0, 0, 0 );
  //homeSphere_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , homeSphere_LV, "homeSphere_PV", world_LV, false, 0 );

  homeSphereMinusChapeauCasting = new G4SubtractionSolid( "homeSphereMinusChapeauCasting", homeSphere, chapeauCasting_A2 ); 
  homeSphereMinusChapeauCasting_LV = new G4LogicalVolume( homeSphereMinusChapeauCasting, MaterialFactory::getInstance()->getEpoxy(), "homeSphereMinusChapeauCasting_A2", 0, 0, 0 );
  //homeSphereMinusChapeauCasting = new G4SubtractionSolid( "homeSphereMinusChapeauCasting", homeSphere, chapeauCasting ); 
  //homeSphereMinusChapeauCasting_LV = new G4LogicalVolume( homeSphereMinusChapeauCasting, MaterialFactory::getInstance()->getEpoxy(), "homeSphereMinusChapeauCasting", 0, 0, 0 );
  homeSphereMinusChapeauCasting_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
  //homeSphereMinusChapeauCasting_LV->SetVisAttributes(G4VisAttributes::Invisible);
  //homeSphereMinusChapeauCasting_PV = new G4PVPlacement( chapeauPoofMinusHomeOrbMatrix, G4ThreeVector( 0.0*mm, 50.0*mm, 0.0*mm) , homeSphereMinusChapeauCasting_LV, "homeSphereMinusChapeauCasting_PV", world_LV, false, 0 );

  //  PFM-NOTE: Disabled due to geometry revision from A1->A2
  homeSphereMinusChapeauCasting_PV = new G4PVPlacement( chapeauPoofMinusHomeOrbMatrix, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , homeSphereMinusChapeauCasting_LV, "homeSphereMinusChapeauCasting_PV", world_LV, false, 0 );  //  main piece to re-enable 

  //  PFM-NOTE: Bangeroo 

  scintShellCollar = new G4Sphere( "scintShellCollar", hemisphereRadiusOuterScaled, ( chapeau_Vertices[ 0 ] * chapeau_scale ).mag(), 0.0 * deg, 360.0 * deg, 0.0 * deg, 90.0 * deg );
  //scintShellCollar_LV = new G4LogicalVolume( scintShellCollar, MaterialFactory::getInstance()->getEpoxy(), "scintShellCollar", 0, 0, 0 );
  //scintShellCollar_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , scintShellCollar_LV, "scintShellCollar_PV", world_LV, false, 0 );

  scintShellCollarMinusChapeauPoof = new G4SubtractionSolid( "scintShellCollarMinusChapeauPoof", scintShellCollar, chapeauPoof ); 
  scintShellCollarMinusChapeauPoof_LV = new G4LogicalVolume( scintShellCollarMinusChapeauPoof, MaterialFactory::getInstance()->getEpoxy(), "scintShellCollarMinusChapeauPoof", 0, 0, 0 );
  scintShellCollarMinusChapeauPoof_LV->SetVisAttributes(new G4VisAttributes(G4Color(white.GetRed(), white.GetGreen(), white.GetBlue(), 0.5)));
  //scintShellCollarMinusChapeauPoof_PV->SetVisAttributes(G4VisAttributes::Invisible);
  //scintShellCollarMinusChapeauPoof_PV = new G4PVPlacement( chapeauPoofMinusHomeOrbMatrix, G4ThreeVector( 0.0*mm, 50.0*mm, 0.0*mm) , scintShellCollarMinusChapeauPoof_LV, "scintShellCollarMinusChapeauPoof_PV", world_LV, false, 0 );
  //scintShellCollarMinusChapeauPoof_PV = new G4PVPlacement( chapeauPoofMinusHomeOrbMatrix, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , scintShellCollarMinusChapeauPoof_LV, "scintShellCollarMinusChapeauPoof_PV", world_LV, false, 0 );

  //  PFM-NOTE: This is just to confirm that it doesn't go all exothermic.  Move it to the rest of the surface code for full testing
  
	G4String absorptiveOpticalSurfaceChapeauPoofString = G4String( "absorptiveOpticalSurfaceChapeauPoof" );	
	G4OpticalSurface *absorptiveOpticalSurfaceChapeauPoof = new G4OpticalSurface( absorptiveOpticalSurfaceChapeauPoofString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceChapeauPoofString, chapeauPoofMinusHomeOrb_PV, homeSphereMinusChapeauCasting_PV, absorptiveOpticalSurfaceChapeauPoof);
	//new G4LogicalBorderSurface( absorptiveOpticalSurfaceChapeauPoofString, hexagon_B_A_PV, hexagon_B_BBox1_Minus_hexagon_B_PV, absorptiveOpticalSurfaceChapeauPoof);

	absorptiveOpticalSurfaceChapeauPoof->SetType( dielectric_metal );
	absorptiveOpticalSurfaceChapeauPoof->SetFinish( polished );
	absorptiveOpticalSurfaceChapeauPoof->SetModel( glisur );

	G4double ppChapeauPoof[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numChapeauPoof = sizeof( ppChapeauPoof ) / sizeof( G4double );
	G4double reflectivityChapeauPoof[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityChapeauPoof ) == sizeof( ppChapeauPoof ));
	G4double efficiencyChapeauPoof[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyChapeauPoof ) == sizeof( ppChapeauPoof ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyChapeauPoof = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyChapeauPoof->AddProperty( "REFLECTIVITY", ppChapeauPoof, reflectivityChapeauPoof, numChapeauPoof );
	absorptiveOpticalSurfacePropertyChapeauPoof->AddProperty( "EFFICIENCY", ppChapeauPoof, efficiencyChapeauPoof, numChapeauPoof );
	absorptiveOpticalSurfaceChapeauPoof->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyChapeauPoof );

  //  PFM-NOTE: Chapeau work - end


	//  PFM-NOTE: The following four surfaces are to cover the interior shell
	//  Before this was one piece but due to a) the cutting of the hemisphere of scint in half and b) the epoxy "gasket" connecting the two is complicates the geometry somewhat

	//  hemisphereNorthScint_PV
	G4String absorptiveOpticalSurfacehemisphereNorthScintString = G4String( "absorptiveOpticalSurfacehemisphereNorthScint" );	
	G4OpticalSurface *absorptiveOpticalSurfacehemisphereNorthScint = new G4OpticalSurface( absorptiveOpticalSurfacehemisphereNorthScintString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacehemisphereNorthScintString, hemisphereNorthScint_PV, innermost_hemisphere_PV, absorptiveOpticalSurfacehemisphereNorthScint);  //  B1

	absorptiveOpticalSurfacehemisphereNorthScint->SetType( dielectric_metal );
	absorptiveOpticalSurfacehemisphereNorthScint->SetFinish( ground );
	//absorptiveOpticalSurfacehemisphereNorthScint->SetFinish( polished );
	absorptiveOpticalSurfacehemisphereNorthScint->SetModel( glisur );

	G4double pphemisphereNorthScint[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numhemisphereNorthScint = sizeof( pphemisphereNorthScint ) / sizeof( G4double );
	G4double reflectivityhemisphereNorthScint[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityhemisphereNorthScint ) == sizeof( pphemisphereNorthScint ));
	G4double efficiencyhemisphereNorthScint[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyhemisphereNorthScint ) == sizeof( pphemisphereNorthScint ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereNorthScint = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyhemisphereNorthScint->AddProperty( "REFLECTIVITY", pphemisphereNorthScint, reflectivityhemisphereNorthScint, numhemisphereNorthScint );
	absorptiveOpticalSurfacePropertyhemisphereNorthScint->AddProperty( "EFFICIENCY", pphemisphereNorthScint, efficiencyhemisphereNorthScint, numhemisphereNorthScint );
	absorptiveOpticalSurfacehemisphereNorthScint->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyhemisphereNorthScint );

	//  hemisphereNorthGasket_PV
	G4String absorptiveOpticalSurfacehemisphereNorthGasketString = G4String( "absorptiveOpticalSurfacehemisphereNorthGasket" );	
	G4OpticalSurface *absorptiveOpticalSurfacehemisphereNorthGasket = new G4OpticalSurface( absorptiveOpticalSurfacehemisphereNorthGasketString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacehemisphereNorthGasketString, hemisphereNorthGasket_PV, innermost_hemisphere_PV, absorptiveOpticalSurfacehemisphereNorthGasket);  //  B1

	absorptiveOpticalSurfacehemisphereNorthGasket->SetType( dielectric_metal );
	absorptiveOpticalSurfacehemisphereNorthGasket->SetFinish( ground );
	//absorptiveOpticalSurfacehemisphereNorthGasket->SetFinish( polished );
	absorptiveOpticalSurfacehemisphereNorthGasket->SetModel( glisur );

	G4double pphemisphereNorthGasket[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numhemisphereNorthGasket = sizeof( pphemisphereNorthGasket ) / sizeof( G4double );
	G4double reflectivityhemisphereNorthGasket[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityhemisphereNorthGasket ) == sizeof( pphemisphereNorthGasket ));
	G4double efficiencyhemisphereNorthGasket[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyhemisphereNorthGasket ) == sizeof( pphemisphereNorthGasket ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereNorthGasket = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyhemisphereNorthGasket->AddProperty( "REFLECTIVITY", pphemisphereNorthGasket, reflectivityhemisphereNorthGasket, numhemisphereNorthGasket );
	absorptiveOpticalSurfacePropertyhemisphereNorthGasket->AddProperty( "EFFICIENCY", pphemisphereNorthGasket, efficiencyhemisphereNorthGasket, numhemisphereNorthGasket );
	absorptiveOpticalSurfacehemisphereNorthGasket->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyhemisphereNorthGasket );
	
	//  hemisphereSouthScint_PV
	G4String absorptiveOpticalSurfacehemisphereSouthScintString = G4String( "absorptiveOpticalSurfacehemisphereSouthScint" );	
	G4OpticalSurface *absorptiveOpticalSurfacehemisphereSouthScint = new G4OpticalSurface( absorptiveOpticalSurfacehemisphereSouthScintString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacehemisphereSouthScintString, hemisphereSouthScint_PV, innermost_hemisphere_PV, absorptiveOpticalSurfacehemisphereSouthScint);  //  B1

	absorptiveOpticalSurfacehemisphereSouthScint->SetType( dielectric_metal );
	absorptiveOpticalSurfacehemisphereSouthScint->SetFinish( ground );
	//absorptiveOpticalSurfacehemisphereSouthScint->SetFinish( polished );
	absorptiveOpticalSurfacehemisphereSouthScint->SetModel( glisur );

	G4double pphemisphereSouthScint[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numhemisphereSouthScint = sizeof( pphemisphereSouthScint ) / sizeof( G4double );
	G4double reflectivityhemisphereSouthScint[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityhemisphereSouthScint ) == sizeof( pphemisphereSouthScint ));
	G4double efficiencyhemisphereSouthScint[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyhemisphereSouthScint ) == sizeof( pphemisphereSouthScint ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereSouthScint = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyhemisphereSouthScint->AddProperty( "REFLECTIVITY", pphemisphereSouthScint, reflectivityhemisphereSouthScint, numhemisphereSouthScint );
	absorptiveOpticalSurfacePropertyhemisphereSouthScint->AddProperty( "EFFICIENCY", pphemisphereSouthScint, efficiencyhemisphereSouthScint, numhemisphereSouthScint );
	absorptiveOpticalSurfacehemisphereSouthScint->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyhemisphereSouthScint );

	//  hemisphereSouthGasket_PV
	G4String absorptiveOpticalSurfacehemisphereSouthGasketString = G4String( "absorptiveOpticalSurfacehemisphereSouthGasket" );	
	G4OpticalSurface *absorptiveOpticalSurfacehemisphereSouthGasket = new G4OpticalSurface( absorptiveOpticalSurfacehemisphereSouthGasketString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacehemisphereSouthGasketString, hemisphereSouthGasket_PV, innermost_hemisphere_PV, absorptiveOpticalSurfacehemisphereSouthGasket);  //  B1

	absorptiveOpticalSurfacehemisphereSouthGasket->SetType( dielectric_metal );
	absorptiveOpticalSurfacehemisphereSouthGasket->SetFinish( ground );
	//absorptiveOpticalSurfacehemisphereSouthGasket->SetFinish( polished );
	absorptiveOpticalSurfacehemisphereSouthGasket->SetModel( glisur );

	G4double pphemisphereSouthGasket[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numhemisphereSouthGasket = sizeof( pphemisphereSouthGasket ) / sizeof( G4double );
	G4double reflectivityhemisphereSouthGasket[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityhemisphereSouthGasket ) == sizeof( pphemisphereSouthGasket ));
	G4double efficiencyhemisphereSouthGasket[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyhemisphereSouthGasket ) == sizeof( pphemisphereSouthGasket ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyhemisphereSouthGasket = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyhemisphereSouthGasket->AddProperty( "REFLECTIVITY", pphemisphereSouthGasket, reflectivityhemisphereSouthGasket, numhemisphereSouthGasket );
	absorptiveOpticalSurfacePropertyhemisphereSouthGasket->AddProperty( "EFFICIENCY", pphemisphereSouthGasket, efficiencyhemisphereSouthGasket, numhemisphereSouthGasket );
	absorptiveOpticalSurfacehemisphereSouthGasket->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyhemisphereSouthGasket );


//  A2 stuff: end of the hemisphere
/*
	G4String absorptiveOpticalSurfaceinnermostHemiString = G4String( "absorptiveOpticalSurfaceinnermostHemi" );	
	G4OpticalSurface *absorptiveOpticalSurfaceinnermostHemi = new G4OpticalSurface( absorptiveOpticalSurfaceinnermostHemiString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceinnermostHemiString, fTestScint_PV, innermost_hemisphere_PV, absorptiveOpticalSurfaceinnermostHemi);  //  B1
	//new G4LogicalBorderSurface( absorptiveOpticalSurfaceinnermostHemiString, fTestScint_PV, innermost_hemisphere_PV, absorptiveOpticalSurfaceinnermostHemi);  //  A2
	//new G4LogicalBorderSurface( absorptiveOpticalSurfaceinnermostHemiString, waterTubsMinusScintBox_PV, innermost_hemisphere_PV, absorptiveOpticalSurfaceinnermostHemi);
	//new G4LogicalBorderSurface( absorptiveOpticalSurfaceinnermostHemiString, innermost_hemisphere_PV, waterTubsMinusScintBox_PV, absorptiveOpticalSurfaceinnermostHemi);

	absorptiveOpticalSurfaceinnermostHemi->SetType( dielectric_metal );
	absorptiveOpticalSurfaceinnermostHemi->SetFinish( ground );
	//absorptiveOpticalSurfaceinnermostHemi->SetFinish( polished );
	absorptiveOpticalSurfaceinnermostHemi->SetModel( glisur );

	G4double ppinnermostHemi[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numinnermostHemi = sizeof( ppinnermostHemi ) / sizeof( G4double );
	G4double reflectivityinnermostHemi[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityinnermostHemi ) == sizeof( ppinnermostHemi ));
	G4double efficiencyinnermostHemi[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyinnermostHemi ) == sizeof( ppinnermostHemi ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyinnermostHemi = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyinnermostHemi->AddProperty( "REFLECTIVITY", ppinnermostHemi, reflectivityinnermostHemi, numinnermostHemi );
	absorptiveOpticalSurfacePropertyinnermostHemi->AddProperty( "EFFICIENCY", ppinnermostHemi, efficiencyinnermostHemi, numinnermostHemi );
	absorptiveOpticalSurfaceinnermostHemi->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyinnermostHemi );
*/

/*
//  ScintCube surface work
//  Scint1->Scint2
	G4String opticalSurfaceScint1Scint2String = G4String( "opticalSurfaceScint1Scint2" );	
	G4OpticalSurface *opticalSurfaceScint1Scint2 = new G4OpticalSurface( opticalSurfaceScint1Scint2String );

	new G4LogicalBorderSurface( opticalSurfaceScint1Scint2String, hackScint1of4_PV, hackScint2of4_PV, opticalSurfaceScint1Scint2);

	opticalSurfaceScint1Scint2->SetType( dielectric_metal );
	//opticalSurfaceScint1Scint2->SetFinish( ground );
	opticalSurfaceScint1Scint2->SetFinish( polished );
	opticalSurfaceScint1Scint2->SetModel( glisur );

	G4double ppopticalSurfaceScint1Scint2[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint1Scint2 = sizeof( ppopticalSurfaceScint1Scint2 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint1Scint2[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint1Scint2 ) == sizeof( ppopticalSurfaceScint1Scint2 ));
	G4double efficiencyopticalSurfaceScint1Scint2[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint1Scint2 ) == sizeof( ppopticalSurfaceScint1Scint2 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint1Scint2 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint1Scint2->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint1Scint2, reflectivityopticalSurfaceScint1Scint2, numopticalSurfaceScint1Scint2 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint1Scint2->AddProperty( "EFFICIENCY", ppopticalSurfaceScint1Scint2, efficiencyopticalSurfaceScint1Scint2, numopticalSurfaceScint1Scint2 );
	opticalSurfaceScint1Scint2->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint1Scint2 );

//  Scint2->Scint1

	G4String opticalSurfaceScint2Scint1String = G4String( "opticalSurfaceScint2Scint1" );	
	G4OpticalSurface *opticalSurfaceScint2Scint1 = new G4OpticalSurface( opticalSurfaceScint2Scint1String );

	new G4LogicalBorderSurface( opticalSurfaceScint2Scint1String, hackScint2of4_PV, hackScint1of4_PV, opticalSurfaceScint2Scint1);

	opticalSurfaceScint2Scint1->SetType( dielectric_metal );
	//opticalSurfaceScint2Scint1->SetFinish( ground );
	opticalSurfaceScint2Scint1->SetFinish( polished );
	opticalSurfaceScint2Scint1->SetModel( glisur );

	G4double ppopticalSurfaceScint2Scint1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint2Scint1 = sizeof( ppopticalSurfaceScint2Scint1 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint2Scint1[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint2Scint1 ) == sizeof( ppopticalSurfaceScint2Scint1 ));
	G4double efficiencyopticalSurfaceScint2Scint1[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint2Scint1 ) == sizeof( ppopticalSurfaceScint2Scint1 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint1 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint1->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint2Scint1, reflectivityopticalSurfaceScint2Scint1, numopticalSurfaceScint2Scint1 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint1->AddProperty( "EFFICIENCY", ppopticalSurfaceScint2Scint1, efficiencyopticalSurfaceScint2Scint1, numopticalSurfaceScint2Scint1 );
	opticalSurfaceScint2Scint1->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint1 );

//  Scint2->Scint3
	G4String opticalSurfaceScint2Scint3String = G4String( "opticalSurfaceScint2Scint3" );	
	G4OpticalSurface *opticalSurfaceScint2Scint3 = new G4OpticalSurface( opticalSurfaceScint2Scint3String );

	new G4LogicalBorderSurface( opticalSurfaceScint2Scint3String, hackScint2of4_PV, hackScint3of4_PV, opticalSurfaceScint2Scint3);

	opticalSurfaceScint2Scint3->SetType( dielectric_metal );
	//opticalSurfaceScint2Scint3->SetFinish( ground );
	opticalSurfaceScint2Scint3->SetFinish( polished );
	opticalSurfaceScint2Scint3->SetModel( glisur );

	G4double ppopticalSurfaceScint2Scint3[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint2Scint3 = sizeof( ppopticalSurfaceScint2Scint3 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint2Scint3[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint2Scint3 ) == sizeof( ppopticalSurfaceScint2Scint3 ));
	G4double efficiencyopticalSurfaceScint2Scint3[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint2Scint3 ) == sizeof( ppopticalSurfaceScint2Scint3 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint3 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint3->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint2Scint3, reflectivityopticalSurfaceScint2Scint3, numopticalSurfaceScint2Scint3 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint3->AddProperty( "EFFICIENCY", ppopticalSurfaceScint2Scint3, efficiencyopticalSurfaceScint2Scint3, numopticalSurfaceScint2Scint3 );
	opticalSurfaceScint2Scint3->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint2Scint3 );

//  Scint3->Scint2
	G4String opticalSurfaceScint3Scint2String = G4String( "opticalSurfaceScint3Scint2" );	
	G4OpticalSurface *opticalSurfaceScint3Scint2 = new G4OpticalSurface( opticalSurfaceScint3Scint2String );

	new G4LogicalBorderSurface( opticalSurfaceScint3Scint2String, hackScint3of4_PV, hackScint2of4_PV, opticalSurfaceScint3Scint2);

	opticalSurfaceScint3Scint2->SetType( dielectric_metal );
	//opticalSurfaceScint3Scint2->SetFinish( ground );
	opticalSurfaceScint3Scint2->SetFinish( polished );
	opticalSurfaceScint3Scint2->SetModel( glisur );

	G4double ppopticalSurfaceScint3Scint2[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint3Scint2 = sizeof( ppopticalSurfaceScint3Scint2 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint3Scint2[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint3Scint2 ) == sizeof( ppopticalSurfaceScint3Scint2 ));
	G4double efficiencyopticalSurfaceScint3Scint2[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint3Scint2 ) == sizeof( ppopticalSurfaceScint3Scint2 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint2 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint2->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint3Scint2, reflectivityopticalSurfaceScint3Scint2, numopticalSurfaceScint3Scint2 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint2->AddProperty( "EFFICIENCY", ppopticalSurfaceScint3Scint2, efficiencyopticalSurfaceScint3Scint2, numopticalSurfaceScint3Scint2 );
	opticalSurfaceScint3Scint2->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint2 );

//  Scint3->Scint4
	G4String opticalSurfaceScint3Scint4String = G4String( "opticalSurfaceScint3Scint4" );	
	G4OpticalSurface *opticalSurfaceScint3Scint4 = new G4OpticalSurface( opticalSurfaceScint3Scint4String );

	new G4LogicalBorderSurface( opticalSurfaceScint3Scint4String, hackScint3of4_PV, hackScint4of4_PV, opticalSurfaceScint3Scint4);

	opticalSurfaceScint3Scint4->SetType( dielectric_metal );
	//opticalSurfaceScint3Scint4->SetFinish( ground );
	opticalSurfaceScint3Scint4->SetFinish( polished );
	opticalSurfaceScint3Scint4->SetModel( glisur );

	G4double ppopticalSurfaceScint3Scint4[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint3Scint4 = sizeof( ppopticalSurfaceScint3Scint4 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint3Scint4[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint3Scint4 ) == sizeof( ppopticalSurfaceScint3Scint4 ));
	G4double efficiencyopticalSurfaceScint3Scint4[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint3Scint4 ) == sizeof( ppopticalSurfaceScint3Scint4 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint4 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint4->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint3Scint4, reflectivityopticalSurfaceScint3Scint4, numopticalSurfaceScint3Scint4 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint4->AddProperty( "EFFICIENCY", ppopticalSurfaceScint3Scint4, efficiencyopticalSurfaceScint3Scint4, numopticalSurfaceScint3Scint4 );
	opticalSurfaceScint3Scint4->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint3Scint4 );

// Scint4->Scint3
	G4String opticalSurfaceScint4Scint3String = G4String( "opticalSurfaceScint4Scint3" );	
	G4OpticalSurface *opticalSurfaceScint4Scint3 = new G4OpticalSurface( opticalSurfaceScint4Scint3String );

	new G4LogicalBorderSurface( opticalSurfaceScint4Scint3String, hackScint4of4_PV, hackScint3of4_PV, opticalSurfaceScint4Scint3);

	opticalSurfaceScint4Scint3->SetType( dielectric_metal );
	//opticalSurfaceScint4Scint3->SetFinish( ground );
	opticalSurfaceScint4Scint3->SetFinish( polished );
	opticalSurfaceScint4Scint3->SetModel( glisur );

	G4double ppopticalSurfaceScint4Scint3[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numopticalSurfaceScint4Scint3 = sizeof( ppopticalSurfaceScint4Scint3 ) / sizeof( G4double );
	G4double reflectivityopticalSurfaceScint4Scint3[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityopticalSurfaceScint4Scint3 ) == sizeof( ppopticalSurfaceScint4Scint3 ));
	G4double efficiencyopticalSurfaceScint4Scint3[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyopticalSurfaceScint4Scint3 ) == sizeof( ppopticalSurfaceScint4Scint3 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyopticalSurfaceScint4Scint3 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyopticalSurfaceScint4Scint3->AddProperty( "REFLECTIVITY", ppopticalSurfaceScint4Scint3, reflectivityopticalSurfaceScint4Scint3, numopticalSurfaceScint4Scint3 );
	absorptiveOpticalSurfacePropertyopticalSurfaceScint4Scint3->AddProperty( "EFFICIENCY", ppopticalSurfaceScint4Scint3, efficiencyopticalSurfaceScint4Scint3, numopticalSurfaceScint4Scint3 );
	opticalSurfaceScint4Scint3->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyopticalSurfaceScint4Scint3 );
*/
/*
	G4String absorptiveOpticalSurfacecollarPieceString = G4String( "absorptiveOpticalSurfacecollarPiece" );	
	G4OpticalSurface *absorptiveOpticalSurfacecollarPiece = new G4OpticalSurface( absorptiveOpticalSurfacecollarPieceString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacecollarPieceString, fTestScint_PV, scintShellCollarMinusChapeauPoof_PV, absorptiveOpticalSurfacecollarPiece);
	//new G4LogicalBorderSurface( absorptiveOpticalSurfacecollarPieceString, waterTubsMinusScintBox_PV, innermost_hemisphere_PV, absorptiveOpticalSurfacecollarPiece);
	//new G4LogicalBorderSurface( absorptiveOpticalSurfacecollarPieceString, innermost_hemisphere_PV, waterTubsMinusScintBox_PV, absorptiveOpticalSurfacecollarPiece);

	absorptiveOpticalSurfacecollarPiece->SetType( dielectric_metal );
	absorptiveOpticalSurfacecollarPiece->SetFinish( ground );
	//absorptiveOpticalSurfacecollarPiece->SetFinish( polished );
	absorptiveOpticalSurfacecollarPiece->SetModel( glisur );

	G4double ppcollarPiece[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numcollarPiece = sizeof( ppcollarPiece ) / sizeof( G4double );
	G4double reflectivitycollarPiece[] = { 0.0, 0.0 };
	assert( sizeof( reflectivitycollarPiece ) == sizeof( ppcollarPiece ));
	G4double efficiencycollarPiece[] = { 0.0, 0.0 };
	assert( sizeof( efficiencycollarPiece ) == sizeof( ppcollarPiece ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertycollarPiece = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertycollarPiece->AddProperty( "REFLECTIVITY", ppcollarPiece, reflectivitycollarPiece, numcollarPiece );
	absorptiveOpticalSurfacePropertycollarPiece->AddProperty( "EFFICIENCY", ppcollarPiece, efficiencycollarPiece, numcollarPiece );
	absorptiveOpticalSurfacecollarPiece->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertycollarPiece );
*/

  pentagon_A = new G4Polyhedra( "lgC", 0.0*M_PI, 2.0*M_PI, 5, 2, pentagon_A_ZPlane, pentagon_A_RadiiInner, pentagon_A_RadiiOuter );
  pentagon_A_MinusOrb = new G4SubtractionSolid( "PentagonPyramid-Orb", pentagon_A, lg_orb_void ); 
  pentagon_A_LV = new G4LogicalVolume( pentagon_A_MinusOrb, MaterialFactory::getInstance()->getEpoxy(), "pentagon_A_LV", 0, 0, 0 );
  //pentagon_A_logicalVolume->SetVisAttributes(G4VisAttributes::Invisible);
  //pentagon_A_logicalVolume->SetColour( 1.0, 1.0, 0.0 );
  //G4Colour yellow = G4Color::Yellow();
  G4Colour magenta = G4Color::Magenta();
  pentagon_A_LV->SetVisAttributes(new G4VisAttributes(G4Color(magenta.GetRed(), magenta.GetGreen(), magenta.GetBlue(), 0.3)));
  //pentagon_A_logicalVolume->SetVisAttributes(new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 0.3)));

  G4RotationMatrix* rm_x2 = new G4RotationMatrix();
  rm_x2->rotateX( 90.0 * deg );	

  //pentagon_A_PV = new G4PVPlacement( rm_x2, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_LV, "pentagon_A_PV", world_LV, false, 0 );

  
  G4RotationMatrix *rotMatrix12 = new G4RotationMatrix();
  rotMatrix12->rotate( -1.0 * (( 90 + 36 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix12->rotate((( 90.0 - 63.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix12->rotate(( 90 / 180.0 ) * M_PI, axisOfRotation3 );
  //pentagon_A_physicalVolumeDummyPentA = new G4PVPlacement( rotMatrix12, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "pentagon_A_physicalVolumeDummyPentA", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix13 = new G4RotationMatrix();
  rotMatrix13->rotate( -1.0 * (( 90 + 36 + 72 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix13->rotate((( 90.0 - 63.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix13->rotate(( 90 / 180.0 ) * M_PI, axisOfRotation3 );
  //pentagon_A_physicalVolumeDummyPentB = new G4PVPlacement( rotMatrix13, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "pentagon_A_physicalVolumeDummyPentB", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix14 = new G4RotationMatrix();
  rotMatrix14->rotate( -1.0 * (( 90 + 36 + 144 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix14->rotate((( 90.0 - 63.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix14->rotate(( 90 / 180.0 ) * M_PI, axisOfRotation3 );
  //pentagon_A_physicalVolumeDummyPentC = new G4PVPlacement( rotMatrix14, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "pentagon_A_physicalVolumeDummyPentC", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix15 = new G4RotationMatrix();
  rotMatrix15->rotate( -1.0 * (( 90 + 36 + 216 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix15->rotate((( 90.0 - 63.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix15->rotate(( 90 / 180.0 ) * M_PI, axisOfRotation3 );
  //pentagon_A_physicalVolumeDummyPentD = new G4PVPlacement( rotMatrix15, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "pentagon_A_physicalVolumeDummyPentD", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix16 = new G4RotationMatrix();
  rotMatrix16->rotate( -1.0 * (( 90 + 36 + 288 ) / 180.0 ) * M_PI, axisOfRotation2 );
  rotMatrix16->rotate((( 90.0 - 63.4 ) / 180.0 ) * M_PI, axisOfRotation1 );
  rotMatrix16->rotate(( 90 / 180.0 ) * M_PI, axisOfRotation3 );
  //pentagon_A_physicalVolumeDummyPentE = new G4PVPlacement( rotMatrix16, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "pentagon_A_physicalVolumeDummyPentE", world_LV, false, 0 );




/*
  G4ThreeVector axisOfRotation1C = G4ThreeVector( 1.0, 0.0, 0.0 ).unit();
  G4ThreeVector axisOfRotation1C1 = G4ThreeVector( 0.0, 0.0, 1.0 ).unit();
  G4RotationMatrix *rotMatrix1C = new G4RotationMatrix();
  rotMatrix1C->rotate((( 90.0 + 37.38 ) / 180.0 ) * M_PI, axisOfRotation1C );
  rotMatrix1C->rotate( 0.5 * M_PI, axisOfRotation1C1 );
  pentagon_A_physicalVolumeDummyPentA = new G4PVPlacement( rotMatrix1C, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "lightguideDummyPentA", world_LV, false, 0 );

  G4ThreeVector axisOfRotation2C = G4ThreeVector( 0.0, 1.0, 0.0 ).unit();
  G4RotationMatrix *rotMatrix2C = new G4RotationMatrix();
  rotMatrix2C->rotate(( 120 / 180.0 ) * M_PI, axisOfRotation2C );
  rotMatrix2C->rotate((( 90.0 + 37.38 ) / 180.0 ) * M_PI, axisOfRotation1C );
  rotMatrix2C->rotate( 0.5 * M_PI, axisOfRotation1C1 );
  pentagon_A_physicalVolumeDummyPentB = new G4PVPlacement( rotMatrix2C, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "lightguideDummyPentB", world_LV, false, 0 );

  G4RotationMatrix *rotMatrix3C = new G4RotationMatrix();

  rotMatrix3C->rotate(( 120 / 180.0 ) * M_PI * -1.0, axisOfRotation2C );
  rotMatrix3C->rotate((( 90.0 + 37.38 ) / 180.0 ) * M_PI, axisOfRotation1C );
  rotMatrix3C->rotate( 0.5 * M_PI, axisOfRotation1C1 );

  pentagon_A_physicalVolumeDummyPentC = new G4PVPlacement( rotMatrix3C, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , pentagon_A_logicalVolume, "lightguideDummyHexC", world_LV, false, 0 );
*/
//  PFM-RESUME: rehab Hex_B, then add Pent_B & test 

  //  Create part B of the hexagonal lightguide
  hexagon_B = new G4TessellatedSolid( "hexagon_B" );
  //testSolidCube = new G4TessellatedSolid( "testSolidCube" );
  G4double hexLong = 22.05 * mm;
  G4double hexLongScaled = hexLong * shellScale;
  G4double hexShort = 11.03 * mm;
  G4double hexShortScaled = hexShort * shellScale;
  G4double hexMid = 19.1 * mm;
  G4double hexMidScaled = hexMid * shellScale;
  G4double square = 12.7 * mm;
  G4double squareScaled = square * shellScale;
  //G4double square = 13.5 * mm;
  //G4double hexOffset = 0.0 * mm;
  //G4double squareOffset = 10.0 * mm;
  G4double hexagon_B_top = lightguideThicknessHexB * 0.5;
  G4double hexagon_B_bottom = lightguideThicknessHexB * -0.5;
  //G4double squareOffset = -10.0 * mm;

  const G4ThreeVector hexagon_B_Vertices[ 10 ] = { 

 G4ThreeVector( hexShortScaled * -1.0 , hexagon_B_top, hexMidScaled * +1.0 ),
 G4ThreeVector( hexShortScaled * +1.0 , hexagon_B_top, hexMidScaled * +1.0 ),
 G4ThreeVector( hexLongScaled * +1.0 , hexagon_B_top, hexLongScaled * +0.0 ),
 G4ThreeVector( hexShortScaled * +1.0 , hexagon_B_top, hexMidScaled * -1.0 ),
 G4ThreeVector( hexShortScaled * -1.0 , hexagon_B_top, hexMidScaled * -1.0 ),
 G4ThreeVector( hexLongScaled * -1.0 , hexagon_B_top, hexLongScaled * +0.0 ),
 G4ThreeVector( squareScaled * -1.0 , hexagon_B_bottom, squareScaled * +1.0 ),
 G4ThreeVector( squareScaled * +1.0 , hexagon_B_bottom, squareScaled * +1.0 ),
 G4ThreeVector( squareScaled * +1.0 , hexagon_B_bottom, squareScaled * -1.0 ),
 G4ThreeVector( squareScaled * -1.0 , hexagon_B_bottom, squareScaled * -1.0 )

/*
 G4ThreeVector( hexShort * -1.0 , hexagon_B_top, hexMid * +1.0 ),
 G4ThreeVector( hexShort * +1.0 , hexagon_B_top, hexMid * +1.0 ),
 G4ThreeVector( hexLong * +1.0 , hexagon_B_top, hexLong * +0.0 ),
 G4ThreeVector( hexShort * +1.0 , hexagon_B_top, hexMid * -1.0 ),
 G4ThreeVector( hexShort * -1.0 , hexagon_B_top, hexMid * -1.0 ),
 G4ThreeVector( hexLong * -1.0 , hexagon_B_top, hexLong * +0.0 ),
 G4ThreeVector( square * -1.0 , hexagon_B_bottom, square * +1.0 ),
 G4ThreeVector( square * +1.0 , hexagon_B_bottom, square * +1.0 ),
 G4ThreeVector( square * +1.0 , hexagon_B_bottom, square * -1.0 ),
 G4ThreeVector( square * -1.0 , hexagon_B_bottom, square * -1.0 )
*/

/*
 G4ThreeVector( hexShort * -1.0 , hexMid * +1.0, -50.0 ),
 G4ThreeVector( hexShort * +1.0 , hexMid * +1.0, -50.0 ),
 G4ThreeVector( hexLong * +1.0 , hexLong * +0.0, -50.0 ),
 G4ThreeVector( hexShort * +1.0 , hexMid * -1.0, -50.0 ),
 G4ThreeVector( hexShort * -1.0 , hexMid * -1.0, -50.0 ),
 G4ThreeVector( hexLong * -1.0 , hexLong * +0.0, -50.0 ),
 G4ThreeVector( square * -1.0 , square * +1.0, -60.0 ),
 G4ThreeVector( square * +1.0 , square * +1.0, -60.0 ),
 G4ThreeVector( square * +1.0 , square * -1.0, -60.0 ),
 G4ThreeVector( square * -1.0 , square * -1.0, -60.0 )
*/
/*
 G4ThreeVector( hexShort * -1.0 , squareOffset * +0.5, hexMid * +1.0 ),
 G4ThreeVector( hexShort * +1.0 , squareOffset * +0.5, hexMid * +1.0 ),
 G4ThreeVector( hexLong * +1.0 , squareOffset * +0.5, hexLong * +0.0 ),
 G4ThreeVector( hexShort * +1.0 , squareOffset * +0.5, hexMid * -1.0 ),
 G4ThreeVector( hexShort * -1.0 , squareOffset * +0.5, hexMid * -1.0 ),
 G4ThreeVector( hexLong * -1.0 , squareOffset * +0.5, hexLong * +0.0 ),
 G4ThreeVector( square * -1.0 , squareOffset * +0.25, square * +1.0 ),
 G4ThreeVector( square * +1.0 , squareOffset * +0.25, square * +1.0 ),
 G4ThreeVector( square * +1.0 , squareOffset * +0.25, square * -1.0 ),
 G4ThreeVector( square * -1.0 , squareOffset * +0.25, square * -1.0 ),
*/
};

hexagon_B_Facets[ 0 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 1 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 6 ], ABSOLUTE );
hexagon_B_Facets[ 2 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 7 ], ABSOLUTE );
hexagon_B_Facets[ 3 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 2 ], ABSOLUTE );
hexagon_B_Facets[ 4 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 8 ], ABSOLUTE );
hexagon_B_Facets[ 5 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 3 ], ABSOLUTE );
hexagon_B_Facets[ 6 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 7 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 8 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 9 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 10 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 11 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 12 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 13 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 14 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 0 ], ABSOLUTE );
hexagon_B_Facets[ 15 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 1 ], ABSOLUTE );


//  These are for the old orientation, I needed to reverse the order when I changed it 
/*
hexagon_B_Facets[ 0 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 1 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 6 ], ABSOLUTE );
hexagon_B_Facets[ 2 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 7 ], ABSOLUTE );
hexagon_B_Facets[ 3 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 2 ], ABSOLUTE );
hexagon_B_Facets[ 4 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 8 ], ABSOLUTE );
hexagon_B_Facets[ 5 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 3 ], ABSOLUTE );
hexagon_B_Facets[ 6 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 7 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 8 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 9 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 10 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 11 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 12 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 5 ], ABSOLUTE );
hexagon_B_Facets[ 13 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 14 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 0 ], ABSOLUTE );
hexagon_B_Facets[ 15 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 1 ], ABSOLUTE );
*/

hexagon_B_Facets[ 0 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 5 ], hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 0 ], ABSOLUTE );
hexagon_B_Facets[ 1 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 6 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 0 ], ABSOLUTE );
hexagon_B_Facets[ 2 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 6 ], ABSOLUTE );
hexagon_B_Facets[ 3 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 7 ], ABSOLUTE );
hexagon_B_Facets[ 4 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 7 ], ABSOLUTE );
hexagon_B_Facets[ 5 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 8 ], ABSOLUTE );
hexagon_B_Facets[ 6 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 3 ], hexagon_B_Vertices[ 8 ], ABSOLUTE );
hexagon_B_Facets[ 7 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 8 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 5 ], hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 9 ], ABSOLUTE );
hexagon_B_Facets[ 9 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 5 ], hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 6 ], ABSOLUTE );
hexagon_B_Facets[ 10 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 7 ], hexagon_B_Vertices[ 6 ], ABSOLUTE );
hexagon_B_Facets[ 11 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 9 ], hexagon_B_Vertices[ 8 ], hexagon_B_Vertices[ 7 ], ABSOLUTE );
hexagon_B_Facets[ 12 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 5 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 4 ], ABSOLUTE );
hexagon_B_Facets[ 13 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 4 ], hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 3 ], ABSOLUTE );
hexagon_B_Facets[ 14 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 0 ], hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 3 ], ABSOLUTE );
hexagon_B_Facets[ 15 ] = new G4TriangularFacet ( hexagon_B_Vertices[ 1 ], hexagon_B_Vertices[ 2 ], hexagon_B_Vertices[ 3 ], ABSOLUTE );

  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 0 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 1 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 2 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 3 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 4 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 5 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 6 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 7 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 8 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 9 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 10 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 11 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 12 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 13 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 14 ] ));
  hexagon_B->AddFacet( (G4VFacet *)( hexagon_B_Facets[ 15 ] ));
  hexagon_B->SetSolidClosed( true );

  //  Temporarily disbled while I swizzle geometry 
  hexagon_B_LV = new G4LogicalVolume( hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_LV", 0, 0, 0 );
  hexagon_B_LV->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed()+0.2, brown.GetGreen()+0.2, brown.GetBlue()+0.2, 0.3)));
  //hexagon_B_logicalVolume->SetVisAttributes(new G4VisAttributes(G4Color(brown.GetRed(), brown.GetGreen(), brown.GetBlue(), 0.3)));
  //hexagon_B_logicalVolume->SetVisAttributes(new G4VisAttributes(G4Color(green.GetRed(), green.GetGreen(), green.GetBlue(), 0.3)));

/*
  // PFM-NOTE: This never changes, as long as we're in a 1cm deep "frame" 
  hexagon_B_physicalVolumeA = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * hexAatThinnest ) + ( -1.0 * ( lightguideThicknessHexB * 0.5 )), 0.0*mm) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeA", world_LV, false, 0 );
*/

  //hexagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -55.0*mm, 0.0*mm) , hexagon_B_logicalVolume, "hexagon_B_physicalVolume", world_LV, false, 0 );
  //hexagon_B_physicalVolumeA = new G4PVPlacement( rotMatrix2, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeA", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentAatThinnest ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );

  //  PFM-NOTE: These are the latest pieces, re-enable them to reconstitute current design 
  G4ThreeVector myVec_hexagon_B_physicalVolumeA = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * ( hemisphereRadiusOuterScaled + ( lightguideThicknessAB * 0.5 ))); 
  //G4ThreeVector myVec_hexagon_B_physicalVolumeA = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * ( 50.0 + ( lightguideThicknessAB * 0.5 ))); 
  myVec_hexagon_B_physicalVolumeA.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
  myVec_hexagon_B_physicalVolumeA.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  //hexagon_B_A_PV = new G4PVPlacement( rotMatrix2a, myVec_hexagon_B_physicalVolumeA, hexagon_B_LV, "hexagon_B_A_PV", world_LV, false, 0 );
  //hexagon_B_physicalVolumeA = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeA", world_LV, false, 0 );


  //  PFM-TWEAK4: fixing B ring placement
/*
  G4ThreeVector myVec_test_Box = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * ( 50.0 + ( lightguideThicknessAB * 0.5 )));   
  myVec_test_Box.rotate( -1.0 * ( 45.0 / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
  myVec_test_Box.rotate( -1.0 * ( 45.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

  placement_test_Box = new G4Box("placement_test_Box", 5.0 * mm, 5.0 * mm, 5.0 * mm );
  placement_test_Box_logicalVolume = new G4LogicalVolume( placement_test_Box, MaterialFactory::getInstance()->getEpoxy(), "placement_test_Box", 0, 0, 0 );
  placement_test_Box_physicalVolume = new G4PVPlacement( 0, myVec_test_Box, placement_test_Box_logicalVolume, "placement_test_Box_physicalVolume", world_LV, false, 0 );
  //placement_test_Box_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm), placement_test_Box_logicalVolume, "placement_test_Box_physicalVolume", world_LV, false, 0 );
*/

  G4ThreeVector myVec_hexagon_B_physicalVolumeB = G4ThreeVector( -1.0 * ( hemisphereRadiusOuterScaled + ( lightguideThicknessAB * 0.5 )), 0.0 * mm, 0.0 * mm );
  //G4ThreeVector myVec_hexagon_B_physicalVolumeB = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * ( 50.0 + ( lightguideThicknessAB * 0.5 )));

  myVec_hexagon_B_physicalVolumeB.rotate( 1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  myVec_hexagon_B_physicalVolumeB.rotate( 1.0 * (( 0 + 72 ) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

  //  PFM-NOTE: My intution is this is the problem transformation
  //  The problem is this transformation is not as portable as I thought it was.  I need to go back and redesign it from scratch.
  //  PFM-RESUME: get the transformation that specifically fixes this B placement (the orientation is fine so leave it alone), then make the generic version for the rest of the ring, then get the SiPM array built & placed  

  //hexagon_B_B_PV = new G4PVPlacement( rotMatrix3a, myVec_hexagon_B_physicalVolumeB , hexagon_B_LV, "hexagon_B_B_PV", world_LV, false, 0 );

  //hexagon_B_physicalVolumeB = new G4PVPlacement( rotMatrix3a, myVec_hexagon_B_physicalVolumeB , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeB", world_LV, false, 0 );
  //hexagon_B_physicalVolumeB = new G4PVPlacement( rotMatrix3, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeB", world_LV, false, 0 );

  G4ThreeVector myVec_hexagon_B_physicalVolumeC = G4ThreeVector( -1.0 * ( hemisphereRadiusOuterScaled + ( lightguideThicknessAB * 0.5 )), 0.0 * mm, 0.0 * mm );
  myVec_hexagon_B_physicalVolumeC.rotate( 1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  myVec_hexagon_B_physicalVolumeC.rotate( 1.0 * (( 0 + ( 72 * 2 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
  //hexagon_B_C_PV = new G4PVPlacement( rotMatrix4a, myVec_hexagon_B_physicalVolumeC , hexagon_B_LV, "hexagon_B_C_PV", world_LV, false, 0 );

  G4ThreeVector myVec_hexagon_B_physicalVolumeD = G4ThreeVector( -1.0 * ( hemisphereRadiusOuterScaled + ( lightguideThicknessAB * 0.5 )), 0.0 * mm, 0.0 * mm );
  myVec_hexagon_B_physicalVolumeD.rotate( 1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  myVec_hexagon_B_physicalVolumeD.rotate( 1.0 * (( 0 + ( 72 * 3 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
  //hexagon_B_D_PV = new G4PVPlacement( rotMatrix5a, myVec_hexagon_B_physicalVolumeD , hexagon_B_LV, "hexagon_B_D_PV", world_LV, false, 0 );

  G4ThreeVector myVec_hexagon_B_physicalVolumeE = G4ThreeVector( -1.0 * ( hemisphereRadiusOuterScaled + ( lightguideThicknessAB * 0.5 )), 0.0 * mm, 0.0 * mm );
  myVec_hexagon_B_physicalVolumeE.rotate( 1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  myVec_hexagon_B_physicalVolumeE.rotate( 1.0 * (( 0 + ( 72 * 4 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
  //hexagon_B_E_PV = new G4PVPlacement( rotMatrix6a, myVec_hexagon_B_physicalVolumeE , hexagon_B_LV, "hexagon_B_E_PV", world_LV, false, 0 );


//  PFM-NOTE: Preserve this until fixed
/*
  G4ThreeVector myVec_hexagon_B_physicalVolumeB = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * ( 50.0 + ( lightguideThicknessAB * 0.5 )));
  myVec_hexagon_B_physicalVolumeB.rotate( ( ( 90 + 72 ) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
  //  PFM-NOTE: My intution is this is the problem transformation
  //  The problem is this transformation is not as portable as I thought it was.  I need to go back and redesign it from scratch.
  //  PFM-RESUME: get the transformation that specifically fixes this B placement (the orientation is fine so leave it alone), then make the generic version for the rest of the ring, then get the SiPM array built & placed  
  myVec_hexagon_B_physicalVolumeB.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
  hexagon_B_physicalVolumeB = new G4PVPlacement( rotMatrix3a, myVec_hexagon_B_physicalVolumeB , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeB", world_LV, false, 0 );
  //hexagon_B_physicalVolumeB = new G4PVPlacement( rotMatrix3a, myVec_hexagon_B_physicalVolumeB , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeB", world_LV, false, 0 );
  //hexagon_B_physicalVolumeB = new G4PVPlacement( rotMatrix3, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeB", world_LV, false, 0 );
*/


  //hexagon_B_physicalVolumeC = new G4PVPlacement( rotMatrix4, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeC", world_LV, false, 0 );
  //hexagon_B_physicalVolumeD = new G4PVPlacement( rotMatrix5, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeD", world_LV, false, 0 );
  //hexagon_B_physicalVolumeE = new G4PVPlacement( rotMatrix6, G4ThreeVector( 0.0*mm, -0.0*mm, 0.0*mm ) , hexagon_B_logicalVolume, "hexagon_B_physicalVolumeE", world_LV, false, 0 );

  //  Old placements, don't use 
  //hexagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -50.0*mm, 0.0*mm) , hexagon_B_logicalVolume, "hexagon_B_physicalVolume", world_LV, false, 0 );
  //hexagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -75.0*mm, 0.0*mm) , hexagon_B_logicalVolume, "hexagon_B_physicalVolume", world_LV, false, 0 );
  //hexagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , hexagon_B_logicalVolume, "hexagon_B_physicalVolume", world_LV, false, 0 );


  //  Create part B of the pentagonal lightguide
  pentagon_B = new G4TessellatedSolid( "pentagon_B" );
  //testSolidCube = new G4TessellatedSolid( "testSolidCube" );
  G4double pentXLong = 18.76 * mm;
  G4double pentXLongScaled = pentXLong * shellScale;
  G4double pentXMid = 15.18 * mm;
  G4double pentXMidScaled = pentXMid * shellScale;
  G4double pentXShort = 5.8 * mm;
  G4double pentXShortScaled = pentXShort * shellScale;
  G4double pentYLong = 17.84 * mm;
  G4double pentYLongScaled = pentYLong * shellScale;
  G4double pentYShort = 11.03 * mm;
  G4double pentYShortScaled = pentYShort * shellScale;
  G4double pentSquare = 12.7 * mm;
  G4double pentSquareScaled = pentSquare * shellScale;

  //G4double pentLong = 22.05 * mm;
  //G4double pentShort = 11.03 * mm;
  //G4double pentMid = 19.1 * mm;
  //G4double pentSquare = 13.5 * mm;
  //G4double pentOffset = 0.0 * mm;
  //G4double pentSquareOffset = 10.0 * mm;
  //G4double squareOffset = -10.0 * mm;

  G4double pentagon_B_top = lightguideThicknessPentB * 0.5;
  G4double pentagon_B_bottom = lightguideThicknessPentB * -0.5;
  //G4double pentagon_B_top = 5.0 * mm;
  //G4double pentagon_B_bottom = pentagon_B_top - lightguideThicknessPentB;

  //  Verified by drawing (mostly)
  //  Originally 1cm thick, tweaked so the two B lightguides are the same thickness 
  const G4ThreeVector pentagon_B_Vertices[ 9 ] = { 
    G4ThreeVector( pentXLongScaled, pentagon_B_top, 0 ),
    G4ThreeVector( pentXShortScaled, pentagon_B_top, -1.0 * pentYLongScaled ),
    G4ThreeVector( -1.0 * pentXMidScaled, pentagon_B_top, -1.0 * pentYShortScaled ),
    G4ThreeVector( -1.0 * pentXMidScaled, pentagon_B_top, 1.0 * pentYShortScaled ),
    G4ThreeVector( pentXShortScaled, pentagon_B_top, 1.0 * pentYLongScaled ),

    G4ThreeVector(  1.0 * pentSquareScaled, pentagon_B_bottom,  1.0 * pentSquareScaled ),
    G4ThreeVector(  1.0 * pentSquareScaled, pentagon_B_bottom, -1.0 * pentSquareScaled ),
    G4ThreeVector( -1.0 * pentSquareScaled, pentagon_B_bottom, -1.0 * pentSquareScaled ),
    G4ThreeVector( -1.0 * pentSquareScaled, pentagon_B_bottom,  1.0 * pentSquareScaled ) };

/*
    G4ThreeVector( 18.76, pentagon_B_top, 0 ),
    G4ThreeVector( 5.8, pentagon_B_top, -17.84 ),
    G4ThreeVector( -15.18, pentagon_B_top, -11.03 ),
    G4ThreeVector( -15.18, pentagon_B_top, 11.03 ),
    G4ThreeVector( 5.8, pentagon_B_top, 17.84 ),

    G4ThreeVector( 12.7, pentagon_B_bottom, 12.7 ),
    G4ThreeVector( 12.7, pentagon_B_bottom, -12.7 ),
    G4ThreeVector( -12.7, pentagon_B_bottom, -12.7 ),
    G4ThreeVector( -12.7, pentagon_B_bottom, 12.7 ) };
*/

//  The geometry to be 2.5mm thick
/*
    G4ThreeVector( 18.76, 5.0, 0 ),
    G4ThreeVector( 5.8, 5.0, -17.84 ),
    G4ThreeVector( -15.18, 5.0, -11.03 ),
    G4ThreeVector( -15.18, 5.0, 11.03 ),
    G4ThreeVector( 5.8, 5.0, 17.84 ),
    G4ThreeVector( 12.6, 2.5, 12.6 ),
    G4ThreeVector( 12.6, 2.5, -12.6 ),
    G4ThreeVector( -12.6, 2.5, -12.6 ),
    G4ThreeVector( -12.6, 2.5, 12.6 ) };
*/
//  The geometry to be 10mm thick
/* 
    G4ThreeVector( 12.6, -5.0, 12.6 ),
    G4ThreeVector( 12.6, -5.0, -12.6 ),
    G4ThreeVector( -12.6, -5.0, -12.6 ),
    G4ThreeVector( -12.6, -5.0, 12.6 ) };
*/

//  The geometry to mate to the 4x4 SiPM array (only the square side needed to change) 
/*
    G4ThreeVector( 6.3, -5.0, 6.3 ),
    G4ThreeVector( 6.3, -5.0, -6.3 ),
    G4ThreeVector( -6.3, -5.0, -6.3 ),
    G4ThreeVector( -6.3, -5.0, 6.3 ) };
*/

/*
    G4ThreeVector( 18.76, 0.0, 0 ),
    G4ThreeVector( 5.8, 0.0, -17.84 ),
    G4ThreeVector( -15.18, 0.0, -11.03 ),
    G4ThreeVector( -15.18, 0.0, 11.03 ),
    G4ThreeVector( 5.8, 0.0, 17.84 ),
    G4ThreeVector( 6.3, -10.0, 6.3 ),
    G4ThreeVector( 6.3, -10.0, -6.3 ),
    G4ThreeVector( -6.3, -10.0, -6.3 ),
    G4ThreeVector( -6.3, -10.0, 6.3 ) };
*/
/*
    G4ThreeVector( 0.0, 5.0, 18.76 ),
    G4ThreeVector( 17.84, 5.0, 5.8 ),
    G4ThreeVector( 11.03, 5.0, -15.18 ),
    G4ThreeVector( -11.03, 5.0, -15.18 ),
    G4ThreeVector( -17.84, 5.0, 5.8 ),
    G4ThreeVector( -6.3, -5.0, 6.3 ),
    G4ThreeVector( 6.3, -5.0, 6.3 ),
    G4ThreeVector( 6.3, -5.0, -6.3 ),
    G4ThreeVector( -6.3, -5.0, -6.3 ) };
*/
  pentagon_B_Facets[ 0 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 5 ], pentagon_B_Vertices[ 0 ], pentagon_B_Vertices[ 4 ], ABSOLUTE );
  pentagon_B_Facets[ 1 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 6 ], pentagon_B_Vertices[ 0 ], pentagon_B_Vertices[ 5 ], ABSOLUTE );
  pentagon_B_Facets[ 2 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 6 ], pentagon_B_Vertices[ 1 ], pentagon_B_Vertices[ 0 ], ABSOLUTE );
  pentagon_B_Facets[ 3 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 7 ], pentagon_B_Vertices[ 1 ], pentagon_B_Vertices[ 6 ], ABSOLUTE );
  pentagon_B_Facets[ 4 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 7 ], pentagon_B_Vertices[ 2 ], pentagon_B_Vertices[ 1 ], ABSOLUTE );
  pentagon_B_Facets[ 5 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 8 ], pentagon_B_Vertices[ 2 ], pentagon_B_Vertices[ 7 ], ABSOLUTE );
  pentagon_B_Facets[ 6 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 8 ], pentagon_B_Vertices[ 3 ], pentagon_B_Vertices[ 2 ], ABSOLUTE );
  pentagon_B_Facets[ 7 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 8 ], pentagon_B_Vertices[ 4 ], pentagon_B_Vertices[ 3 ], ABSOLUTE );
  pentagon_B_Facets[ 8 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 5 ], pentagon_B_Vertices[ 4 ], pentagon_B_Vertices[ 8 ], ABSOLUTE );
  pentagon_B_Facets[ 9 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 5 ], pentagon_B_Vertices[ 8 ], pentagon_B_Vertices[ 6 ], ABSOLUTE );
  pentagon_B_Facets[ 10 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 8 ], pentagon_B_Vertices[ 7 ], pentagon_B_Vertices[ 6 ], ABSOLUTE );
  pentagon_B_Facets[ 11 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 0 ], pentagon_B_Vertices[ 3 ], pentagon_B_Vertices[ 4 ], ABSOLUTE );
  pentagon_B_Facets[ 12 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 3 ], pentagon_B_Vertices[ 0 ], pentagon_B_Vertices[ 2 ], ABSOLUTE );
  pentagon_B_Facets[ 13 ] = new G4TriangularFacet ( pentagon_B_Vertices[ 2 ], pentagon_B_Vertices[ 0 ], pentagon_B_Vertices[ 1 ], ABSOLUTE );


  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 0 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 1 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 2 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 3 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 4 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 5 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 6 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 7 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 8 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 9 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 10 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 11 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 12 ] ));
  pentagon_B->AddFacet( (G4VFacet *)( pentagon_B_Facets[ 13 ] ));
  pentagon_B->SetSolidClosed( true );

  pentagon_B_LV = new G4LogicalVolume( pentagon_B, MaterialFactory::getInstance()->getEpoxy(), "pentagon_B_LV", 0, 0, 0 );

  pentagon_B_LV->SetVisAttributes(new G4VisAttributes(G4Color(magenta.GetRed(), magenta.GetGreen(), magenta.GetBlue(), 0.3)));
  //pentagon_B_logicalVolume->SetVisAttributes(new G4VisAttributes(G4Color(yellow.GetRed(), yellow.GetGreen(), yellow.GetBlue(), 0.3)));

  // PFM-NOTE: This never changes, as long as we're in a 1cm deep "frame" 

  //  PFM-NOTE: The top is the most recent
  //pentagon_B_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * hemisphereRadiusOuterScaled ) + ( -1.0 * pentAatThinnestScaled ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_LV, "pentagon_B_PV", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentAatThinnest ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentagon_B_spacing ) + ( -1.0 * lightguideThicknessPentB ), 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentagon_B_spacing ) + ( -1.0 * lightguideThicknessPentB ), 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );

  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -51.327 - 5.0 ) * mm, 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -56.283 * mm, 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -51.283 * mm, 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0 * mm, 0.0*mm) , pentagon_B_logicalVolume, "pentagon_B_physicalVolume", world_LV, false, 0 );

   
  //  PFM-NOTE: The following is the "collar" that the B section fits into to provide the proper geometry for applying optical surface properties.  
  //  Temporarily disabled while I swizzle geometry.
  hexagon_B_BBox1 = new G4Box("hexagon_B_BBox1", ( hexLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessHexB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMidScaled + toleranceTwiddle ) * mm );
  //hexagon_B_BBox1 = new G4Box("hexagon_B_BBox1", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox1_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox1_Minus_hexagon_B", hexagon_B_BBox1, hexagon_B ); 

  hexagon_B_BBox1_Minus_hexagon_B_LV = new G4LogicalVolume( hexagon_B_BBox1_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox1_Minus_hexagon_B_LV", 0, 0, 0 );

  //hexagon_B_BBox1_Minus_hexagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //hexagon_B_BBox1_Minus_hexagon_B_PV = new G4PVPlacement( rotMatrix2a, myVec_hexagon_B_physicalVolumeA, hexagon_B_BBox1_Minus_hexagon_B_LV, "hexagon_B_BBox1_Minus_hexagon_B_PV", world_LV, false, 0 );

  hexagon_B_BBox2 = new G4Box("hexagon_B_BBox2", ( hexLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessHexB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMidScaled + toleranceTwiddle ) * mm );
  //hexagon_B_BBox2 = new G4Box("hexagon_B_BBox2", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox2_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox2_Minus_hexagon_B", hexagon_B_BBox2, hexagon_B ); 

  hexagon_B_BBox2_Minus_hexagon_B_LV = new G4LogicalVolume( hexagon_B_BBox2_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox2_Minus_hexagon_B_LV", 0, 0, 0 );

  //hexagon_B_BBox2_Minus_hexagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //hexagon_B_BBox2_Minus_hexagon_B_PV = new G4PVPlacement( rotMatrix3a, myVec_hexagon_B_physicalVolumeB, hexagon_B_BBox2_Minus_hexagon_B_LV, "hexagon_B_BBox2_Minus_hexagon_B_PV", world_LV, false, 0 );

  hexagon_B_BBox3 = new G4Box("hexagon_B_BBox3", ( hexLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessHexB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMidScaled + toleranceTwiddle ) * mm );
  //hexagon_B_BBox3 = new G4Box("hexagon_B_BBox3", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox3_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox3_Minus_hexagon_B", hexagon_B_BBox3, hexagon_B ); 

  hexagon_B_BBox3_Minus_hexagon_B_LV = new G4LogicalVolume( hexagon_B_BBox3_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox3_Minus_hexagon_B_LV", 0, 0, 0 );

  //hexagon_B_BBox3_Minus_hexagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //hexagon_B_BBox3_Minus_hexagon_B_PV = new G4PVPlacement( rotMatrix4a, myVec_hexagon_B_physicalVolumeC, hexagon_B_BBox3_Minus_hexagon_B_LV, "hexagon_B_BBox3_Minus_hexagon_B_PV", world_LV, false, 0 );

  hexagon_B_BBox4 = new G4Box("hexagon_B_BBox4", ( hexLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessHexB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMidScaled + toleranceTwiddle ) * mm );
  //hexagon_B_BBox4 = new G4Box("hexagon_B_BBox4", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox4_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox4_Minus_hexagon_B", hexagon_B_BBox4, hexagon_B ); 

  hexagon_B_BBox4_Minus_hexagon_B_LV = new G4LogicalVolume( hexagon_B_BBox4_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox4_Minus_hexagon_B_LV", 0, 0, 0 );

  //hexagon_B_BBox4_Minus_hexagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //hexagon_B_BBox4_Minus_hexagon_B_PV = new G4PVPlacement( rotMatrix5a, myVec_hexagon_B_physicalVolumeD, hexagon_B_BBox4_Minus_hexagon_B_LV, "hexagon_B_BBox4_Minus_hexagon_B_PV", world_LV, false, 0 );

  hexagon_B_BBox5 = new G4Box("hexagon_B_BBox5", ( hexLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessHexB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMidScaled + toleranceTwiddle ) * mm );
  //hexagon_B_BBox5 = new G4Box("hexagon_B_BBox5", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox5_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox5_Minus_hexagon_B", hexagon_B_BBox5, hexagon_B ); 

  hexagon_B_BBox5_Minus_hexagon_B_LV = new G4LogicalVolume( hexagon_B_BBox5_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox5_Minus_hexagon_B_LV", 0, 0, 0 );

  //hexagon_B_BBox5_Minus_hexagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //hexagon_B_BBox5_Minus_hexagon_B_PV = new G4PVPlacement( rotMatrix6a, myVec_hexagon_B_physicalVolumeE, hexagon_B_BBox5_Minus_hexagon_B_LV, "hexagon_B_BBox5_Minus_hexagon_B_PV", world_LV, false, 0 );

/*
  hexagon_B_BBox = new G4Box("hexagon_B_BBox", ( hexLong + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( hexMid + toleranceTwiddle ) * mm );
  hexagon_B_BBox_Minus_hexagon_B = new G4SubtractionSolid( "hexagon_B_BBox_Minus_hexagon_B", hexagon_B_BBox, hexagon_B ); 

  hexagon_B_BBox_Minus_hexagon_B_logicalVolume = new G4LogicalVolume( hexagon_B_BBox_Minus_hexagon_B, MaterialFactory::getInstance()->getEpoxy(), "hexagon_B_BBox_Minus_hexagon_B_logicalVolume", 0, 0, 0 );

  hexagon_B_BBox_Minus_hexagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * hexAatThinnest ) + ( -1.0 * ( lightguideThicknessHexB * 0.5 )), 0.0*mm) , hexagon_B_BBox_Minus_hexagon_B_logicalVolume, "hexagon_B_BBox_Minus_hexagon_B_physicalVolume", world_LV, false, 0 );
*/

  pentagon_B_BBox = new G4Box("pentagon_B_BBox", ( pentXLongScaled + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( pentYLongScaled + toleranceTwiddle ) * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", ( 18.79 + toleranceTwiddle ) * mm, (( lightguideThicknessPentB * 0.5 ) - toleranceTwiddle ) * mm, ( 17.84 + toleranceTwiddle ) * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", ( 18.79 + toleranceTwiddle ) * mm, ( 1.25 - toleranceTwiddle ) * mm, ( 17.84 + toleranceTwiddle ) * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", ( 18.79 + 0.01 ) * mm, ( 5.0 - 0.01 ) * mm, ( 17.84 + 0.01 ) * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", ( 18.79 + 0.01 ) * mm, 5.0 * mm, ( 17.84 + 0.01 ) * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", 20.0 * mm, 5.0 * mm, 20.0 * mm );

  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", 20.0 * mm, 5.0 * mm, 20.0 * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", 20.0 * mm, 6.0 * mm, 20.0 * mm );
  //pentagon_B_BBox = new G4Box("pentagon_B_BBox", 18.76 * mm, 5.0 * mm, 18.76 * mm );

  pentagon_B_BBox_Minus_pentagon_B = new G4SubtractionSolid( "pentagon_B_BBox_Minus_pentagon_B", pentagon_B_BBox, pentagon_B ); 
  pentagon_B_BBox_Minus_pentagon_B_LV = new G4LogicalVolume( pentagon_B_BBox_Minus_pentagon_B, MaterialFactory::getInstance()->getEpoxy(), "pentagon_B_BBox_Minus_pentagon_B_LV", 0, 0, 0 );

  //pentagon_B_BBox_Minus_pentagon_B_LV->SetVisAttributes(G4VisAttributes::Invisible);

  //pentagon_B_BBox_Minus_pentagon_B_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * hemisphereRadiusOuterScaled ) + ( -1.0 * pentAatThinnestScaled ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_LV, "pentagon_B_BBox_Minus_pentagon_B_PV", world_LV, false, 0 );
  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentAatThinnest ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );

  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -1.0 * scint_outer_radius ) + ( -1.0 * pentAatThinnest ) + ( -1.0 * ( lightguideThicknessPentB * 0.5 )), 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );

  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -56.327 ) *mm, 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -46.6 - 10.0 ) *mm, 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -46.6 - 9.8 ) *mm, 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -46.6 - 9.8 ) *mm, 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );
  //pentagon_B_BBox_Minus_pentagon_B_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( -46.6 - 10.0 - 30.0 ) *mm, 0.0*mm) , pentagon_B_BBox_Minus_pentagon_B_logicalVolume, "pentagon_B_BBox_Minus_pentagon_B_physicalVolume", world_LV, false, 0 );
  
  //pentagon_B_BBox_Minus_pentagon_B_logicalVolume->SetVisAttributes(G4VisAttributes::Invisible);

  //  PFM-NOTE: Geometry for the original hex box, to be reused later 
  // A
  //lightguideBBox = new G4Box("boxLGHack", 1.0 + (( 4.411 ) / 2.0 ) * cm, 0.125 * cm, 1.0 + (( 3.82 ) / 2.0 ) * cm );

  //  Original
  //lightguideBBox = new G4Box("boxLGHack", 1.0 + (( 4.411 ) / 2.0 ) * cm, ( 1.0 / 2.0 ) * cm, 1.0 + (( 3.82 ) / 2.0 ) * cm );
  //lightguideShellBoxMinusB = new G4SubtractionSolid( "lightguideShellBoxMinusB", lightguideBBox, hexagon_B ); 
  //lightguideShellBoxMinusB_logicalVolume = new G4LogicalVolume( lightguideShellBoxMinusB, MaterialFactory::getInstance()->getEpoxy(), "lightguideShellBoxMinusB_logicalVolume", 0, 0, 0 );
  //lightguideShellBoxMinusB_logicalVolume->SetVisAttributes(G4VisAttributes::Invisible);

  // E
  //lightguideShellBoxMinusB_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -56.2*mm, 0.0*mm) , lightguideShellBoxMinusB_logicalVolume, "lightguideShellBoxMinusB_physicalVolume", world_LV, false, 0 );

  //  C
  //lightguideShellBoxMinusB_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -53.7*mm, 0.0*mm) , lightguideShellBoxMinusB_logicalVolume, "lightguideShellBoxMinusB_physicalVolume", world_LV, false, 0 );
  //  B
  //lightguideShellBoxMinusB_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -52.45*mm, 0.0*mm) , lightguideShellBoxMinusB_logicalVolume, "lightguideShellBoxMinusB_physicalVolume", world_LV, false, 0 );
  //  A
  //lightguideShellBoxMinusB_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, -51.25*mm, 0.0*mm) , lightguideShellBoxMinusB_logicalVolume, "lightguideShellBoxMinusB_physicalVolume", world_LV, false, 0 );

//  PFM-HACK: all the parked optical work gets reinserted here



//  PFM-HACK: Begining of new optical surfaces 
//  Revised optical surfaces: just three for now 
//  Hex 1
/*
	G4String absorptiveOpticalSurfaceHexagonB1String = G4String( "absorptiveOpticalSurfaceHexagonB1" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB1 = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonB1String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonB1String, hexagon_B_A_PV, hexagon_B_BBox1_Minus_hexagon_B_PV, absorptiveOpticalSurfaceHexagonB1);

	absorptiveOpticalSurfaceHexagonB1->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB1->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB1->SetModel( glisur );

	G4double ppHexagonB1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB1 = sizeof( ppHexagonB1 ) / sizeof( G4double );
	G4double reflectivityHexagonB1[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB1 ) == sizeof( ppHexagonB1 ));
	G4double efficiencyHexagonB1[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB1 ) == sizeof( ppHexagonB1 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB1 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB1->AddProperty( "REFLECTIVITY", ppHexagonB1, reflectivityHexagonB1, numHexagonB1 );
	absorptiveOpticalSurfacePropertyHexagonB1->AddProperty( "EFFICIENCY", ppHexagonB1, efficiencyHexagonB1, numHexagonB1 );
	absorptiveOpticalSurfaceHexagonB1->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB1 );

//  Hex 2
	G4String absorptiveOpticalSurfaceHexagonB2String = G4String( "absorptiveOpticalSurfaceHexagonB2" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB2 = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonB2String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonB2String, hexagon_B_B_PV, hexagon_B_BBox2_Minus_hexagon_B_PV, absorptiveOpticalSurfaceHexagonB2);

	absorptiveOpticalSurfaceHexagonB2->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB2->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB2->SetModel( glisur );

	G4double ppHexagonB2[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB2 = sizeof( ppHexagonB2 ) / sizeof( G4double );
	G4double reflectivityHexagonB2[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB2 ) == sizeof( ppHexagonB2 ));
	G4double efficiencyHexagonB2[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB2 ) == sizeof( ppHexagonB2 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB2 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB2->AddProperty( "REFLECTIVITY", ppHexagonB2, reflectivityHexagonB2, numHexagonB2 );
	absorptiveOpticalSurfacePropertyHexagonB2->AddProperty( "EFFICIENCY", ppHexagonB2, efficiencyHexagonB2, numHexagonB2 );
	absorptiveOpticalSurfaceHexagonB2->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB2 );

//  Hex 3
	G4String absorptiveOpticalSurfaceHexagonB3String = G4String( "absorptiveOpticalSurfaceHexagonB3" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB3 = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonB3String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonB3String, hexagon_B_C_PV, hexagon_B_BBox3_Minus_hexagon_B_PV, absorptiveOpticalSurfaceHexagonB3);

	absorptiveOpticalSurfaceHexagonB3->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB3->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB3->SetModel( glisur );

	G4double ppHexagonB3[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB3 = sizeof( ppHexagonB3 ) / sizeof( G4double );
	G4double reflectivityHexagonB3[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB3 ) == sizeof( ppHexagonB3 ));
	G4double efficiencyHexagonB3[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB3 ) == sizeof( ppHexagonB3 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB3 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB3->AddProperty( "REFLECTIVITY", ppHexagonB3, reflectivityHexagonB3, numHexagonB3 );
	absorptiveOpticalSurfacePropertyHexagonB3->AddProperty( "EFFICIENCY", ppHexagonB3, efficiencyHexagonB3, numHexagonB3 );
	absorptiveOpticalSurfaceHexagonB3->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB3 );

//  Hex 4
	G4String absorptiveOpticalSurfaceHexagonB4String = G4String( "absorptiveOpticalSurfaceHexagonB4" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB4 = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonB4String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonB4String, hexagon_B_D_PV, hexagon_B_BBox3_Minus_hexagon_B_PV, absorptiveOpticalSurfaceHexagonB4);

	absorptiveOpticalSurfaceHexagonB4->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB4->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB4->SetModel( glisur );

	G4double ppHexagonB4[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB4 = sizeof( ppHexagonB4 ) / sizeof( G4double );
	G4double reflectivityHexagonB4[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB4 ) == sizeof( ppHexagonB4 ));
	G4double efficiencyHexagonB4[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB4 ) == sizeof( ppHexagonB4 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB4 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB4->AddProperty( "REFLECTIVITY", ppHexagonB4, reflectivityHexagonB4, numHexagonB4 );
	absorptiveOpticalSurfacePropertyHexagonB4->AddProperty( "EFFICIENCY", ppHexagonB4, efficiencyHexagonB4, numHexagonB4 );
	absorptiveOpticalSurfaceHexagonB4->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB4 );

//  Hex 5
	G4String absorptiveOpticalSurfaceHexagonB5String = G4String( "absorptiveOpticalSurfaceHexagonB5" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB5 = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonB5String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonB5String, hexagon_B_E_PV, hexagon_B_BBox3_Minus_hexagon_B_PV, absorptiveOpticalSurfaceHexagonB5);

	absorptiveOpticalSurfaceHexagonB5->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB5->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB5->SetModel( glisur );

	G4double ppHexagonB5[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB5 = sizeof( ppHexagonB5 ) / sizeof( G4double );
	G4double reflectivityHexagonB5[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB5 ) == sizeof( ppHexagonB5 ));
	G4double efficiencyHexagonB5[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB5 ) == sizeof( ppHexagonB5 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB5 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB5->AddProperty( "REFLECTIVITY", ppHexagonB5, reflectivityHexagonB5, numHexagonB5 );
	absorptiveOpticalSurfacePropertyHexagonB5->AddProperty( "EFFICIENCY", ppHexagonB5, efficiencyHexagonB5, numHexagonB5 );
	absorptiveOpticalSurfaceHexagonB5->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB5 );

// Pentagon (1)
	G4String absorptiveOpticalSurfacePentagonB1String = G4String( "absorptiveOpticalSurfacePentagonB1" );	
	G4OpticalSurface *absorptiveOpticalSurfacePentagonB1 = new G4OpticalSurface( absorptiveOpticalSurfacePentagonB1String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacePentagonB1String, pentagon_B_PV, pentagon_B_BBox_Minus_pentagon_B_PV, absorptiveOpticalSurfacePentagonB1);

	absorptiveOpticalSurfacePentagonB1->SetType( dielectric_metal );
	absorptiveOpticalSurfacePentagonB1->SetFinish( polished );
	absorptiveOpticalSurfacePentagonB1->SetModel( glisur );

	G4double ppPentagonB1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numPentagonB1 = sizeof( ppPentagonB1 ) / sizeof( G4double );
	G4double reflectivityPentagonB1[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityPentagonB1 ) == sizeof( ppPentagonB1 ));
	G4double efficiencyPentagonB1[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyPentagonB1 ) == sizeof( ppPentagonB1 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyPentagonB1 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyPentagonB1->AddProperty( "REFLECTIVITY", ppPentagonB1, reflectivityPentagonB1, numPentagonB1 );
	absorptiveOpticalSurfacePropertyPentagonB1->AddProperty( "EFFICIENCY", ppPentagonB1, efficiencyPentagonB1, numPentagonB1 );
	absorptiveOpticalSurfacePentagonB1->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyPentagonB1 );
*/

//  Optical surface: block spread out of Hex_B
//  Temporarily disabled while I swizzle geometry  
/*	G4String absorptiveOpticalSurfaceHexagonBString = G4String( "absorptiveOpticalSurfaceHexagonB" );	
	G4OpticalSurface *absorptiveOpticalSurfaceHexagonB = new G4OpticalSurface( absorptiveOpticalSurfaceHexagonBString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceHexagonBString, hexagon_B_physicalVolumeA, hexagon_B_BBox_Minus_hexagon_B_physicalVolume, absorptiveOpticalSurfaceHexagonB);

	absorptiveOpticalSurfaceHexagonB->SetType( dielectric_metal );
	absorptiveOpticalSurfaceHexagonB->SetFinish( polished );
	absorptiveOpticalSurfaceHexagonB->SetModel( glisur );

	G4double ppHexagonB[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numHexagonB = sizeof( ppHexagonB ) / sizeof( G4double );
	G4double reflectivityHexagonB[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityHexagonB ) == sizeof( ppHexagonB ));
	G4double efficiencyHexagonB[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyHexagonB ) == sizeof( ppHexagonB ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyHexagonB = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyHexagonB->AddProperty( "REFLECTIVITY", ppHexagonB, reflectivityHexagonB, numHexagonB );
	absorptiveOpticalSurfacePropertyHexagonB->AddProperty( "EFFICIENCY", ppHexagonB, efficiencyHexagonB, numHexagonB );
	absorptiveOpticalSurfaceHexagonB->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyHexagonB );
*/

//  Optical surface: block spread out of Pent_B
/*
	G4String absorptiveOpticalSurfacePentagonBString = G4String( "absorptiveOpticalSurfacePentagonB" );	
	G4OpticalSurface *absorptiveOpticalSurfacePentagonB = new G4OpticalSurface( absorptiveOpticalSurfacePentagonBString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfacePentagonBString, pentagon_B_physicalVolume, pentagon_B_BBox_Minus_pentagon_B_physicalVolume, absorptiveOpticalSurfacePentagonB);

	absorptiveOpticalSurfacePentagonB->SetType( dielectric_metal );
	absorptiveOpticalSurfacePentagonB->SetFinish( polished );
	absorptiveOpticalSurfacePentagonB->SetModel( glisur );

	G4double ppPentagonB[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numPentagonB = sizeof( ppPentagonB ) / sizeof( G4double );
	G4double reflectivityPentagonB[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityPentagonB ) == sizeof( ppPentagonB ));
	G4double efficiencyPentagonB[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyPentagonB ) == sizeof( ppPentagonB ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyPentagonB = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyPentagonB->AddProperty( "REFLECTIVITY", ppPentagonB, reflectivityPentagonB, numPentagonB );
	absorptiveOpticalSurfacePropertyPentagonB->AddProperty( "EFFICIENCY", ppPentagonB, efficiencyPentagonB, numPentagonB );
	absorptiveOpticalSurfacePentagonB->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyPentagonB );
*/
	// surface scintInner
	// create the sphere for the surface (the other ones didn't have to do this, they'd already been created)

	//  PFM-NOTE: This will need to be re-enabled when the replacement lightguide is in its final position 
/*

  	orb_surface = new G4Orb( "orb_surface", hemisphereRadiusInner );
  	//orb_surface = new G4Orb( "orb_surface", 4.0 * cm );
  	orb_surface_LV = new G4LogicalVolume( orb_surface, MaterialFactory::getInstance()->getAir(), "orb_surface_LV", 0, 0, 0 );
	//orb_surface_LV->SetVisAttributes(G4VisAttributes::Invisible);
  	orb_surface_PV = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , orb_surface_LV, "orb_surface_PV", world_LV, false, 0 );

	G4String absorptiveOpticalSurfaceOrbString = G4String( "absorptiveOpticalSurfaceOrb" );	
	G4OpticalSurface *absorptiveOpticalSurfaceOrb = new G4OpticalSurface( absorptiveOpticalSurfaceOrbString );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceOrbString, fTestScint_PV, orb_surface_PV, absorptiveOpticalSurfaceOrb);

	absorptiveOpticalSurfaceOrb->SetType( dielectric_metal );
	//absorptiveOpticalSurfaceOrb->SetFinish( polishedfrontpainted );
	//absorptiveOpticalSurfaceOrb->SetFinish( polishedtioair );
	//absorptiveOpticalSurfaceOrb->SetFinish( groundbackpainted );
	absorptiveOpticalSurfaceOrb->SetFinish( ground );
	//absorptiveOpticalSurfaceOrb->SetFinish( polished );
	absorptiveOpticalSurfaceOrb->SetModel( glisur );

	G4double ppOrb[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numOrb = sizeof( ppOrb ) / sizeof( G4double );
	G4double reflectivityOrb[] = { 0.0, 0.0 };
	//G4double reflectivityOrb[] = { 1.0, 1.0 };
	assert( sizeof( reflectivityOrb ) == sizeof( ppOrb ));
	G4double efficiencyOrb[] = { 1.0, 1.0 };
	//G4double efficiencyOrb[] = { 0.0, 0.0 };
	assert( sizeof( efficiencyOrb ) == sizeof( ppOrb ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyOrb = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyOrb->AddProperty( "REFLECTIVITY", ppOrb, reflectivityOrb, numOrb );
	absorptiveOpticalSurfacePropertyOrb->AddProperty( "EFFICIENCY", ppOrb, efficiencyOrb, numOrb );
	absorptiveOpticalSurfaceOrb->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyOrb );

	//  PFM-NOTE: This may break the sim, comment out if so
	//  Experimenting with fixing weird scintillation problem
	//  Results: didn't break it but didn't fix it either
	//delete orb_surface_PV;
*/
/*
  //  Set up the surface properties
	// surface A1
	G4String absorptiveOpticalSurfaceA1String = G4String( "absorptiveOpticalSurfaceA1" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA1 = new G4OpticalSurface( absorptiveOpticalSurfaceA1String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA1String, hexagon_A_physicalVolume, hexagon_A_physicalVolumeDummyHexA, absorptiveOpticalSurfaceA1);

	absorptiveOpticalSurfaceA1->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA1->SetFinish( polished );
	absorptiveOpticalSurfaceA1->SetModel( glisur );

	G4double ppA1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA1 = sizeof( ppA1 ) / sizeof( G4double );
	G4double reflectivityA1[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA1 ) == sizeof( ppA1 ));
	G4double efficiencyA1[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA1 ) == sizeof( ppA1 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA1 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA1->AddProperty( "REFLECTIVITY", ppA1, reflectivityA1, numA1 );
	absorptiveOpticalSurfacePropertyA1->AddProperty( "EFFICIENCY", ppA1, efficiencyA1, numA1 );
	absorptiveOpticalSurfaceA1->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA1 );

	// surface A2
	G4String absorptiveOpticalSurfaceA2String = G4String( "absorptiveOpticalSurfaceA2" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA2 = new G4OpticalSurface( absorptiveOpticalSurfaceA2String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA2String, hexagon_A_physicalVolume, hexagon_A_physicalVolumeDummyHexB, absorptiveOpticalSurfaceA2);

	absorptiveOpticalSurfaceA2->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA2->SetFinish( polished );
	absorptiveOpticalSurfaceA2->SetModel( glisur );

	G4double ppA2[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA2 = sizeof( ppA2 ) / sizeof( G4double );
	G4double reflectivityA2[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA2 ) == sizeof( ppA2 ));
	G4double efficiencyA2[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA2 ) == sizeof( ppA2 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA2 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA2->AddProperty( "REFLECTIVITY", ppA2, reflectivityA2, numA2 );
	absorptiveOpticalSurfacePropertyA2->AddProperty( "EFFICIENCY", ppA2, efficiencyA2, numA2 );
	absorptiveOpticalSurfaceA2->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA2 );

	// surface A3
	G4String absorptiveOpticalSurfaceA3String = G4String( "absorptiveOpticalSurfaceA3" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA3 = new G4OpticalSurface( absorptiveOpticalSurfaceA3String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA3String, hexagon_A_physicalVolume, hexagon_A_physicalVolumeDummyHexC, absorptiveOpticalSurfaceA3);

	absorptiveOpticalSurfaceA3->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA3->SetFinish( polished );
	absorptiveOpticalSurfaceA3->SetModel( glisur );

	G4double ppA3[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA3 = sizeof( ppA3 ) / sizeof( G4double );
	G4double reflectivityA3[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA3 ) == sizeof( ppA3 ));
	G4double efficiencyA3[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA3 ) == sizeof( ppA3 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA3 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA3->AddProperty( "REFLECTIVITY", ppA3, reflectivityA3, numA3 );
	absorptiveOpticalSurfacePropertyA3->AddProperty( "EFFICIENCY", ppA3, efficiencyA3, numA3 );
	absorptiveOpticalSurfaceA3->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA3 );

	// surface A4
	G4String absorptiveOpticalSurfaceA4String = G4String( "absorptiveOpticalSurfaceA4" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA4 = new G4OpticalSurface( absorptiveOpticalSurfaceA4String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA4String, hexagon_A_physicalVolume, pentagon_A_physicalVolumeDummyPentA, absorptiveOpticalSurfaceA4);

	absorptiveOpticalSurfaceA4->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA4->SetFinish( polished );
	absorptiveOpticalSurfaceA4->SetModel( glisur );

	G4double ppA4[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA4 = sizeof( ppA4 ) / sizeof( G4double );
	G4double reflectivityA4[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA4 ) == sizeof( ppA4 ));
	G4double efficiencyA4[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA4 ) == sizeof( ppA4 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA4 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA4->AddProperty( "REFLECTIVITY", ppA4, reflectivityA4, numA4 );
	absorptiveOpticalSurfacePropertyA4->AddProperty( "EFFICIENCY", ppA4, efficiencyA4, numA4 );
	absorptiveOpticalSurfaceA4->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA4 );

	// surface A5
	G4String absorptiveOpticalSurfaceA5String = G4String( "absorptiveOpticalSurfaceA5" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA5 = new G4OpticalSurface( absorptiveOpticalSurfaceA5String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA5String, hexagon_A_physicalVolume, pentagon_A_physicalVolumeDummyPentB, absorptiveOpticalSurfaceA5);

	absorptiveOpticalSurfaceA5->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA5->SetFinish( polished );
	absorptiveOpticalSurfaceA5->SetModel( glisur );

	G4double ppA5[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA5 = sizeof( ppA5 ) / sizeof( G4double );
	G4double reflectivityA5[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA5 ) == sizeof( ppA5 ));
	G4double efficiencyA5[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA5 ) == sizeof( ppA5 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA5 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA5->AddProperty( "REFLECTIVITY", ppA5, reflectivityA5, numA5 );
	absorptiveOpticalSurfacePropertyA5->AddProperty( "EFFICIENCY", ppA5, efficiencyA5, numA5 );
	absorptiveOpticalSurfaceA5->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA5 );

	// surface A6
	G4String absorptiveOpticalSurfaceA6String = G4String( "absorptiveOpticalSurfaceA6" );	
	G4OpticalSurface *absorptiveOpticalSurfaceA6 = new G4OpticalSurface( absorptiveOpticalSurfaceA6String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceA6String, hexagon_A_physicalVolume, pentagon_A_physicalVolumeDummyPentC, absorptiveOpticalSurfaceA6);

	absorptiveOpticalSurfaceA6->SetType( dielectric_metal );
	absorptiveOpticalSurfaceA6->SetFinish( polished );
	absorptiveOpticalSurfaceA6->SetModel( glisur );

	G4double ppA6[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numA6 = sizeof( ppA6 ) / sizeof( G4double );
	G4double reflectivityA6[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityA6 ) == sizeof( ppA6 ));
	G4double efficiencyA6[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyA6 ) == sizeof( ppA6 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyA6 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyA6->AddProperty( "REFLECTIVITY", ppA6, reflectivityA6, numA6 );
	absorptiveOpticalSurfacePropertyA6->AddProperty( "EFFICIENCY", ppA6, efficiencyA6, numA6 );
	absorptiveOpticalSurfaceA6->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyA6 );

	// surface B1
	G4String absorptiveOpticalSurfaceB1String = G4String( "absorptiveOpticalSurfaceB1" );	
	G4OpticalSurface *absorptiveOpticalSurfaceB1 = new G4OpticalSurface( absorptiveOpticalSurfaceB1String );

	new G4LogicalBorderSurface( absorptiveOpticalSurfaceB1String, hexagon_B_physicalVolume, lightguideShellBoxMinusB_physicalVolume, absorptiveOpticalSurfaceB1);

	absorptiveOpticalSurfaceB1->SetType( dielectric_metal );
	absorptiveOpticalSurfaceB1->SetFinish( polished );
	absorptiveOpticalSurfaceB1->SetModel( glisur );

	G4double ppB1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
	const G4int numB1 = sizeof( ppB1 ) / sizeof( G4double );
	G4double reflectivityB1[] = { 0.0, 0.0 };
	assert( sizeof( reflectivityB1 ) == sizeof( ppB1 ));
	G4double efficiencyB1[] = { 1.0, 1.0 };
	assert( sizeof( efficiencyB1 ) == sizeof( ppB1 ));

	G4MaterialPropertiesTable* absorptiveOpticalSurfacePropertyB1 = new G4MaterialPropertiesTable();

	absorptiveOpticalSurfacePropertyB1->AddProperty( "REFLECTIVITY", ppB1, reflectivityB1, numB1 );
	absorptiveOpticalSurfacePropertyB1->AddProperty( "EFFICIENCY", ppB1, efficiencyB1, numB1 );
	absorptiveOpticalSurfaceB1->SetMaterialPropertiesTable( absorptiveOpticalSurfacePropertyB1 );

*/
//  PFM-HACK: end of new optical surfaces


  //  PFM-NOTE: Disabled while changing simulation 
  /*
  //  Create the two end caps, using identical geometry as the annulus.
  //  PFM-NOTE: This is kind of hacky, I'm just using it to make sure everything lines up.  Later I can make dedicated geometry that more closely matches realiy but since the resulting geometry will be invisible it's just an elegance thing.
  //fTestWater_logicalVolumeA = new G4LogicalVolume( scint_poly, MaterialFactory::getInstance()->getWater(), "testWater_logicalVolumeA", 0, 0, 0 );
  fTestWater_logicalVolumeA = new G4LogicalVolume( union_solid_scint, MaterialFactory::getInstance()->getAir(), "testWater_logicalVolumeA", 0, 0, 0 );
  //fTestWater_logicalVolumeA = new G4LogicalVolume( union_solid_scint, MaterialFactory::getInstance()->getWater(), "testWater_logicalVolumeA", 0, 0, 0 );
  fTestWater_logicalVolumeA->SetVisAttributes(G4VisAttributes::Invisible);
  fTestWater_physicalVolumeEndcap1 = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, -1.0 * scintillatorZ ), fTestWater_logicalVolumeA, "endcap1", world_LV, false, 0 );
  fTestWater_physicalVolumeEndcap2 = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, +1.0 * scintillatorZ ), fTestWater_logicalVolumeA, "endcap2", world_LV, false, 0 );
  */
  /*
  //  Create the fake objects to serve as optical system placeholders to get the surfaces to work properly.
  fTestWater1_box = new G4Box("testWater1_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
  fTestWater1_logicalVolume = new G4LogicalVolume( fTestWater1_box, MaterialFactory::getInstance()->getWater(), "testTestWater1_logicalVolume", 0, 0, 0 );
  fTestWater1_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( ( flooper * 1.0 )*mm, 0.0*mm, 0.0*mm) , fTestWater1_logicalVolume, "dummy1", world_LV, false, 0 );
  //fTestWater1_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( ( flooper * 1.0 )*mm, 0.0*mm, (( flooper * 0.5 ) + 0.25 )*mm) , fTestWater1_logicalVolume, "dummy1", world_LV, false, 0 );

  fTestWater2_box = new G4Box("testWater2_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
  fTestWater2_logicalVolume = new G4LogicalVolume( fTestWater2_box, MaterialFactory::getInstance()->getWater(), "testTestWater2_logicalVolume", 0, 0, 0 );
  fTestWater2_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( ( flooper * -1.0 )*mm, 0.0*mm, 0.0*mm) , fTestWater2_logicalVolume, "dummy2", world_LV, false, 0 );
  //fTestWater2_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( ( flooper * -1.0 )*mm, 0.0*mm, (( flooper * 0.5 ) + 0.25 )*mm) , fTestWater2_logicalVolume, "dummy2", world_LV, false, 0 );

  fTestWater3_box = new G4Box("testWater3_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
  fTestWater3_logicalVolume = new G4LogicalVolume( fTestWater3_box, MaterialFactory::getInstance()->getWater(), "testTestWater3_logicalVolume", 0, 0, 0 );
  fTestWater3_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm , ( flooper * 1.0 )*mm) , fTestWater3_logicalVolume, "dummy3", world_LV, false, 0 );
  //fTestWater3_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( flooper * 1.0 )*mm , (( flooper * 0.5 ) + 0.25 )*mm) , fTestWater3_logicalVolume, "dummy3", world_LV, false, 0 );

  fTestWater4_box = new G4Box("testWater4_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
  fTestWater4_logicalVolume = new G4LogicalVolume( fTestWater4_box, MaterialFactory::getInstance()->getWater(), "testTestWater4_logicalVolume", 0, 0, 0 );
  fTestWater4_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, ( flooper * -1.0 )*mm) , fTestWater4_logicalVolume, "dummy4", world_LV, false, 0 );
  //fTestWater4_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, ( flooper * -1.0 )*mm, (( flooper * 0.5 ) + 0.25 )*mm) , fTestWater4_logicalVolume, "dummy4", world_LV, false, 0 );
  */

  //  PFM-NOTE: Disabled while converting sim 
  /*
  scint_tube_donut_hole = new G4Tubs( "scint_tube_donut_hole", 0.0, radiusInner,
				      halfZ, 0.0 * deg, 360.0 * deg);
  //fTestWater5_box = new G4Box("testWater5_box", ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm, ( flooper * 0.5 )*mm );
  fTestWater5_logicalVolume = new G4LogicalVolume( scint_tube_donut_hole, MaterialFactory::getInstance()->getAir(), "testTestWater5_logicalVolume", 0, 0, 0 );
  //fTestWater5_logicalVolume = new G4LogicalVolume( scint_tube_donut_hole, MaterialFactory::getInstance()->getWater(), "testTestWater5_logicalVolume", 0, 0, 0 );
  fTestWater5_logicalVolume->SetVisAttributes(G4VisAttributes::Invisible);
  fTestWater5_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, 0.0*mm) , fTestWater5_logicalVolume, "dummy5", world_LV, false, 0 );
  //fTestWater5_physicalVolume = new G4PVPlacement( 0, G4ThreeVector( 0.0*mm, 0.0*mm, (( flooper * 1.5 ) + 0.25 )*mm) , fTestWater5_logicalVolume, "dummy5", world_LV, false, 0 );
  */

  //  Make 4 to wrap around longitudinally and be 100% absorptive 
  // #1
  /*
    G4String absorptiveOpticalSurface1String = G4String( "absorptiveOpticalSurface1" );	
    G4OpticalSurface *absorptiveOpticalSurface1 = new G4OpticalSurface( absorptiveOpticalSurface1String );

    new G4LogicalBorderSurface( absorptiveOpticalSurface1String, fTestScint_physicalVolume, fTestWater1_physicalVolume, absorptiveOpticalSurface1);

    absorptiveOpticalSurface1->SetType( dielectric_metal );
    absorptiveOpticalSurface1->SetFinish( polished );
    absorptiveOpticalSurface1->SetModel( glisur );

    G4double pp1[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
    const G4int num1 = sizeof( pp1 ) / sizeof( G4double );
    G4double reflectivity1[] = { 0.0, 0.0 };
    assert( sizeof( reflectivity1 ) == sizeof( pp1 ));
    G4double efficiency1[] = { 1.0, 1.0 };
    assert( sizeof( efficiency1 ) == sizeof( pp1 ));

    G4MaterialPropertiesTable* absorptiveOpticalSurfaceProperty1 = new G4MaterialPropertiesTable();

    absorptiveOpticalSurfaceProperty1->AddProperty( "REFLECTIVITY", pp1, reflectivity1, num1 );
    absorptiveOpticalSurfaceProperty1->AddProperty( "EFFICIENCY", pp1, efficiency1, num1 );
    absorptiveOpticalSurface1->SetMaterialPropertiesTable( absorptiveOpticalSurfaceProperty1 );

    // #2
    G4String absorptiveOpticalSurface2String = G4String( "absorptiveOpticalSurface2" );	
    G4OpticalSurface *absorptiveOpticalSurface2 = new G4OpticalSurface( absorptiveOpticalSurface2String );

    new G4LogicalBorderSurface( absorptiveOpticalSurface2String, fTestScint_physicalVolume, fTestWater2_physicalVolume, absorptiveOpticalSurface2);

    absorptiveOpticalSurface2->SetType( dielectric_metal );
    absorptiveOpticalSurface2->SetFinish( polished );
    absorptiveOpticalSurface2->SetModel( glisur );

    G4double pp2[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
    const G4int num2 = sizeof( pp2 ) / sizeof( G4double );
    G4double reflectivity2[] = { 0.0, 0.0 };
    assert( sizeof( reflectivity2 ) == sizeof( pp2 ));
    G4double efficiency2[] = { 1.0, 1.0 };
    assert( sizeof( efficiency2 ) == sizeof( pp2 ));

    G4MaterialPropertiesTable* absorptiveOpticalSurfaceProperty2 = new G4MaterialPropertiesTable();

    absorptiveOpticalSurfaceProperty2->AddProperty( "REFLECTIVITY", pp2, reflectivity2, num2 );
    absorptiveOpticalSurfaceProperty2->AddProperty( "EFFICIENCY", pp2, efficiency2, num2 );
    absorptiveOpticalSurface2->SetMaterialPropertiesTable( absorptiveOpticalSurfaceProperty2 );	
  */


  //  PFM-NOTE: disbled while modifying simulation 
  /*
  // #3
  G4String absorptiveOpticalSurface3String = G4String( "absorptiveOpticalSurface3" );	
  G4OpticalSurface *absorptiveOpticalSurface3 = new G4OpticalSurface( absorptiveOpticalSurface3String );

  new G4LogicalBorderSurface( absorptiveOpticalSurface3String, fTestScint_physicalVolume, fTestWater_physicalVolumeEndcap1, absorptiveOpticalSurface3);
  //new G4LogicalBorderSurface( absorptiveOpticalSurface3String, fTestScint_physicalVolume, fTestWater3_physicalVolume, absorptiveOpticalSurface3);

  absorptiveOpticalSurface3->SetType( dielectric_metal );
  absorptiveOpticalSurface3->SetFinish( polished );
  absorptiveOpticalSurface3->SetModel( glisur );

  G4double pp3[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
  const G4int num3 = sizeof( pp3 ) / sizeof( G4double );
  G4double reflectivity3[] = { 0.0, 0.0 };
  assert( sizeof( reflectivity3 ) == sizeof( pp3 ));
  G4double efficiency3[] = { 1.0, 1.0 };
  assert( sizeof( efficiency3 ) == sizeof( pp3 ));

  G4MaterialPropertiesTable* absorptiveOpticalSurfaceProperty3 = new G4MaterialPropertiesTable();

  absorptiveOpticalSurfaceProperty3->AddProperty( "REFLECTIVITY", pp3, reflectivity3, num3 );
  absorptiveOpticalSurfaceProperty3->AddProperty( "EFFICIENCY", pp3, efficiency3, num3 );
  absorptiveOpticalSurface3->SetMaterialPropertiesTable( absorptiveOpticalSurfaceProperty3 );	

  // #4
  G4String absorptiveOpticalSurface4String = G4String( "absorptiveOpticalSurface4" );	
  G4OpticalSurface *absorptiveOpticalSurface4 = new G4OpticalSurface( absorptiveOpticalSurface4String );

  new G4LogicalBorderSurface( absorptiveOpticalSurface4String, fTestScint_physicalVolume, fTestWater_physicalVolumeEndcap2, absorptiveOpticalSurface4);
  //new G4LogicalBorderSurface( absorptiveOpticalSurface4String, fTestScint_physicalVolume, fTestWater4_physicalVolume, absorptiveOpticalSurface4);

  absorptiveOpticalSurface4->SetType( dielectric_metal );
  absorptiveOpticalSurface4->SetFinish( polished );
  absorptiveOpticalSurface4->SetModel( glisur );

  G4double pp4[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
  const G4int num4 = sizeof( pp4 ) / sizeof( G4double );
  G4double reflectivity4[] = { 0.0, 0.0 };
  assert( sizeof( reflectivity4 ) == sizeof( pp4 ));
  G4double efficiency4[] = { 1.0, 1.0 };
  assert( sizeof( efficiency4 ) == sizeof( pp4 ));

  G4MaterialPropertiesTable* absorptiveOpticalSurfaceProperty4 = new G4MaterialPropertiesTable();

  absorptiveOpticalSurfaceProperty4->AddProperty( "REFLECTIVITY", pp4, reflectivity4, num4 );
  absorptiveOpticalSurfaceProperty4->AddProperty( "EFFICIENCY", pp4, efficiency4, num4 );
  absorptiveOpticalSurface4->SetMaterialPropertiesTable( absorptiveOpticalSurfaceProperty4 );	
  */
	

  //  This is the old/original surface code, for posterity...
  /*
    G4String absorptiveOpticalSurfaceString = G4String( "absorptiveOpticalSurface" );	
    G4OpticalSurface *absorptiveOpticalSurface = new G4OpticalSurface( absorptiveOpticalSurfaceString );

    new G4LogicalBorderSurface( absorptiveOpticalSurfaceString, fTestScint_physicalVolume, fTestWater2_physicalVolume, absorptiveOpticalSurface);

    absorptiveOpticalSurface->SetType( dielectric_metal );
    absorptiveOpticalSurface->SetFinish( polished );
    absorptiveOpticalSurface->SetModel( glisur );

    G4double pp[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
    const G4int num = sizeof( pp ) / sizeof( G4double );
    G4double reflectivity[] = { 0.0, 0.0 };
    assert( sizeof( reflectivity ) == sizeof( pp ));
    G4double efficiency[] = { 1.0, 1.0 };
    assert( sizeof( efficiency ) == sizeof( pp ));

    G4MaterialPropertiesTable* absorptiveOpticalSurfaceProperty = new G4MaterialPropertiesTable();

    absorptiveOpticalSurfaceProperty->AddProperty( "REFLECTIVITY", pp, reflectivity, num );
    absorptiveOpticalSurfaceProperty->AddProperty( "EFFICIENCY", pp, efficiency, num );
    absorptiveOpticalSurface->SetMaterialPropertiesTable( absorptiveOpticalSurfaceProperty );	
  */

  //  Make 1 to mate to the far side and be "ground"/Lambertian 

  //  PFM-NOTE: Disabled during work to make new sim; will eventually need to be re-enabled 
  /* 
  
  G4String diffuseOpticalSurfaceString = G4String( "diffuseOpticalSurface" );	
  G4OpticalSurface *diffuseOpticalSurface = new G4OpticalSurface( diffuseOpticalSurfaceString );

  new G4LogicalBorderSurface( diffuseOpticalSurfaceString, fTestScint_physicalVolume, fTestWater5_physicalVolume, diffuseOpticalSurface);

  diffuseOpticalSurface->SetType( dielectric_metal );
  diffuseOpticalSurface->SetFinish( ground );
  diffuseOpticalSurface->SetModel( unified );

  G4double pp5[] = { 2.0*eV, 3.5*eV };  //  since we're working with 2.591eV photons these values are acceptable.
  const G4int num5 = sizeof( pp5 ) / sizeof( G4double );
  G4double reflectivity5[] = { 1.0, 1.0 };
  assert( sizeof( reflectivity5 ) == sizeof( pp5 ));
  G4double efficiency5[] = { 0.0, 0.0 };
  assert( sizeof( efficiency5 ) == sizeof( pp5 ));

  G4MaterialPropertiesTable* diffuseOpticalSurfaceProperty = new G4MaterialPropertiesTable();

  diffuseOpticalSurfaceProperty->AddProperty( "REFLECTIVITY", pp5, reflectivity5, num5 );
  diffuseOpticalSurfaceProperty->AddProperty( "EFFICIENCY", pp5, efficiency5, num5 );
  diffuseOpticalSurface->SetMaterialPropertiesTable( diffuseOpticalSurfaceProperty );	

  */

  //  BEGIN - code for funky SiPM placement 

  //G4float subArrayEdge = 26.84*mm;
  //G4float subArrayEdge = 18.25*mm; //  Old
  //G4float subArraySteps [ 6 ] = { -2.5, -1.5, -0.5, +0.5, +1.5, +2.5 };
  //G4float subArraySteps [4] = { -1.5, -0.5, +0.5, +1.5 };
  //G4float interSpace = ( subArrayEdge - ( 8 * 3.3 )) / 7.0;
  //G4float interSpace = ( subArrayEdge - ( 4 * 4.4 )) / 3.0; //  Old
  //G4float interSpace = ( subArrayEdge - ( 4 * housing->getDx() )) / 3.0 *mm;
  //G4float interSpace = 0.75 *mm;  
  //G4float xOffset = 3.0 + interSpace; 
  //G4float xOffset = 4.4 + interSpace;  //  Old 
  //G4float xOffset = housing->getDx() + interSpace; 
  //G4float yOffset = (51.0 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC

  // E 
  //G4float yOffset = ( 62.5 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC

  // C
  //G4float yOffset = ( 57.5 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC
  // B
  //G4float yOffset = ( 55.0 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC
  //  A
  //G4float yOffset = ( 52.5 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC

  //  Original
  //G4float yOffset = (60.0 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC
  //G4float yOffset = (40.0 + 0.15 )*mm;  // half the thickness of the scintillator plus half the thickness of the MPPC
  //G4float yOffset = (40.0 + 0.1 )*mm;
  //G4float zOffset = 3.0 + interSpace;
  //G4float zOffset = 4.4 + interSpace;  //  Old
  //G4float zOffset = housing->getDy() + interSpace;
  //G4float yOffset = (40.0 + 1.0 )*mm;  //  40 for the radius of the annulus, 1 for the fudge factor (I'll explain later...) 

  //  PFM-NOTE: Attemp #1 to replace SiPM place code went here
  //  It was a dumpster fire, so I'm goin back and restarting it 

  //  PFM-NOTE: modern way of placing SiPMs for A2/B(1) work #2 (START)
  //  I know where the CoGs are, can I just use those? 
  //  PFM-HACK: This is a copy/paste of the square vertices construction, come back and clean this up once I have something that works 

  pentagonIndex = 0;
  pentagonCount = 12;
  //pentagonIndex = 0;
  //pentagonCount = 12;

  //  PFM-HACK: The issues I'm seeing may be related to data structure reuse and not an underlying math bug
  //  This is super-hacky because its a test; come back and clean this up since it works I'll need to use it forthe hexagons 
  G4RotationMatrix *myRotMatrixPent[ 12 ][ 16 ];
  G4ThreeVector PB_CoG[ 12 ][ 16 ];
  G4ThreeVector PB_axis1[ 12 ][ 16 ];
  G4ThreeVector PB_axis2[ 12 ][ 16 ];
  G4ThreeVector zPosAxisOut[ 12 ][ 16 ];
  G4ThreeVector myCross1P[ 12 ][ 16 ];
  G4double angleDelta1P[ 12 ][ 16 ];
  G4RotationMatrix facetMatrix1[ 12 ][ 16 ];
  G4double pbHalf = 9.0 * mm;
  G4ThreeVector zPosAxis1[ 12 ][ 16 ];
  G4ThreeVector zPosAxis1New[ 12 ][ 16 ];
  G4double angleDelta2P[ 12 ][ 16 ];
  G4double dot2P[ 12 ][ 16 ];
  G4RotationMatrix facetMatrix2[ 12 ][ 16 ];
  G4float mySipmPitch = config.number("sipm-pitch-mm") * mm;  //  HAMA
  G4ThreeVector home_Vertex[ 12 ][ 16 ];
  int axis1Index;
  int axis2Index;
  int triggerMan;
  G4double rotationTwister;

  while ( pentagonIndex < pentagonCount ) {

	if ( true ) {
	//if (( pentagonIndex == 0 ) || ( pentagonIndex == 2 ) || ( pentagonIndex == 4 ) || ( pentagonIndex == 6 ) || ( pentagonIndex == 8 ) || ( pentagonIndex == 10 )) {

	triggerMan = 0;

	axis1Index = 0;
	axis2Index = 0;

	//while ( axis2Index < 1 ) {
	//	while ( axis1Index < 2 ) {
	while ( axis2Index < 4 ) {
		while ( axis1Index < 4 ) {

			//  PFM-NOTE: hosting all transformation math START
			PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = (( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 2 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale ) + ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 4 ]] * chapeau_scale )) / 5.0;
			PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 1 ]] * chapeau_scale ) - ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 0 ]] * chapeau_scale );
			PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ( chapeau_Vertices[ pentagonVertexIndices[ pentagonIndex ][ 3 ]] * chapeau_scale ) - PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ];

			//  PFM-RESUME: continue converting over to big-ol'-static-storage approach to fix memory issues; debugging build issues from conversion, but looking better  

			//  Convert to unit length
			PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();
			PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();  // local "X+" axis
			PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();  //  local "Y+" axis 

			//  Convert over the matrices to simplify combination 

			zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * mm );  //  counterintuitive; the face we want coupled to the scintillator is on the other side of the SiPM element so I'm flipping the Z sign 
			zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();

			//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: PB_CoG[ pentagonIndex ].dot( zPosAxisOut ): " << PB_CoG[ pentagonIndex ].dot( zPosAxisOut[ pentagonIndex ] ) << std::endl;

			if ( PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] )  < -0.9999 ) {

				//  It's effectively -1, just invert it and set the rotation to pi
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: hit " << std::endl;
				myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setX( 0.0 );
				myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setY( 1.0 );
				myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setZ( 0.0 );
				//myCross1P = G4ThreeVector( 0.0 * mm, 1.0 * mm, 0.0 * mm );
				myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();
				angleDelta1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = pi;
			}
			else {

				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: miss " << std::endl;
				myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].cross( zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
				angleDelta1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( zPosAxisOut[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
				//G4ThreeVector myCross1P = zPosAxisOut.cross( PB_CoG );
				//G4double angleDelta1P = zPosAxisOut.theta( PB_CoG );
			}

			
			facetMatrix1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( myCross1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ], 1.0 * angleDelta1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//G4RotationMatrix facetMatrix1 = G4RotationMatrix( myCross1P, -1.0 * angleDelta1P );

			//G4double angleDeltaDude = PB_axis2_mod.theta( zPosAxisUp );

			//  Matrix2

			//  Rotate the other frame vector to fix the orientation 
			zPosAxis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4ThreeVector( 0.0 * mm, -1.0 * mm, 0.0 * mm );
			//G4ThreeVector zPosAxis1 = G4ThreeVector( 0.0 * mm, 1.0 * mm, 0.0 * mm );
			zPosAxis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = zPosAxis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / zPosAxis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();

			zPosAxis1New[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = facetMatrix1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * zPosAxis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ];
			//G4ThreeVector zPosAxis1New = zPosAxis1.rotate( myCross1P, -1.0 * angleDelta1P );
			//G4ThreeVector zPosAxis1New = zPosAxis1.rotate( myCross1P, angleDelta1P );
			//G4ThreeVector myCross2 = zPosAxis1New.cross( PB_axis2 );
			//angleDelta2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = -1.0 * PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( zPosAxis1New[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			angleDelta2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( zPosAxis1New[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			//  What is the dot between CoG and the resulting cross of CoG's axis1 and the transformed zPosAxis1? it's sign shuold be the answer I'm looking for
			G4double hackDot = PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].cross( zPosAxis1New[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] ));

			if ( hackDot < 0.0 ) {
				rotationTwister = 1.0;
			}
			else {
				rotationTwister = -1.0;
			}

			dot2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( zPosAxis1New[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			//G4double angleDelta2P = zPosAxis1New.theta( PB_axis2 );

			//  Math checking: if I invert the transform on the facet coordinate axes, both of them should be in the XY plane.  If they're not, something's weird 
			if ( triggerMan == 0 ) {  
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: axis1Index, axis2Index, pentagonIndex: " << axis1Index << ", " << axis2Index << ", " << pentagonIndex << std::endl;
				G4RotationMatrix facetMatrix1Invert = facetMatrix1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ];
				facetMatrix1Invert.invert();
				G4ThreeVector PB_axis1_mod = facetMatrix1Invert * PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ]; 
				G4ThreeVector PB_axis2_mod = facetMatrix1Invert * PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ]; 
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: PB_axis1.x, .y, .z: " << PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].x() << ", " << PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].y() << ", " << PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: PB_axis2.x, .y, .z: " << PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].x() << ", " << PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].y() << ", " << PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ].z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: PB_axis1_mod.x, .y, .z: " << PB_axis1_mod.x() << ", " << PB_axis1_mod.y() << ", " << PB_axis1_mod.z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: PB_axis2_mod.x, .y, .z: " << PB_axis2_mod.x() << ", " << PB_axis2_mod.y() << ", " << PB_axis2_mod.z() << std::endl;
				if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: pentagonIndex, angleDelta1P (rad), angleDelta2P (rad), dot2P, hackDot: " << pentagonIndex << ", " << angleDelta1P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << angleDelta2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << dot2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << hackDot << std::endl;

				triggerMan = 1;
			}
			else {

				//  Please don't report anything, I only need it once

			}

			//G4RotationMatrix facetMatrix2 = G4RotationMatrix( PB_CoG, 45.0 * deg );

			facetMatrix2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ], rotationTwister * angleDelta2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//facetMatrix2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ], 1.0 * angleDelta2P[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//G4RotationMatrix facetMatrix2 = G4RotationMatrix( PB_CoG, -1.0 * angleDelta2P );  //  off by exactly 45 deg

		  	home_Vertex[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ( zero_Vertices[ 0 ] - ( PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * 1.5 * mySipmPitch ) - ( PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * 1.5 * mySipmPitch ));

			//  PFM-NOTE: hosting all transformation math END

			//myRotMatrixPent[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = new G4RotationMatrix( facetMatrix1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			myRotMatrixPent[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = new G4RotationMatrix( facetMatrix1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * facetMatrix2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			mHousings.at(( pentagonIndex * 16 ) + ( axis1Index + ( axis2Index * 4 )))->buildAndPlace(world_PV, G4ThreeVector((( home_Vertex[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] + ( PB_axis1[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * mySipmPitch * axis1Index ) + ( PB_axis2[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * mySipmPitch * axis2Index ))) + ( PB_CoG[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * sensorCenterR )), myRotMatrixPent[ pentagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			axis1Index = axis1Index + 1;
		}

		axis1Index = 0;
		axis2Index = axis2Index + 1;
	}
	}
	else {

		//  skip this pentagon
	}
	pentagonIndex = pentagonIndex + 1;
  }

  hexagonIndex = 0;
  hexagonCount = 20;
  //hexagonIndex = 0;
  //hexagonCount = 20;

  //  PFM-HACK: The issues I'm seeing may be related to data structure reuse and not an underlying math bug
  //  This is super-hacky because its a test; come back and clean this up since it works I'll need to use it forthe hexagons 
  G4RotationMatrix *myRotMatrixHex[ 20 ][ 16 ];
  G4ThreeVector HB_CoG[ 20 ][ 16 ];
  G4ThreeVector HB_axis1[ 20 ][ 16 ];
  G4ThreeVector HB_axis2[ 20 ][ 16 ];
  G4ThreeVector HB_zPosAxisOut[ 20 ][ 16 ];
  G4ThreeVector myCross1H[ 20 ][ 16 ];
  G4double angleDelta1H[ 20 ][ 16 ];
  G4RotationMatrix HB_facetMatrix1[ 20 ][ 16 ];
  G4ThreeVector HB_zPosAxis1[ 20 ][ 16 ];
  G4ThreeVector HB_zPosAxis1New[ 20 ][ 16 ];
  G4double angleDelta2H[ 20 ][ 16 ];
  G4double dot2H[ 20 ][ 16 ];
  G4RotationMatrix HB_facetMatrix2[ 20 ][ 16 ];
  G4ThreeVector HB_home_Vertex[ 20 ][ 16 ];

  while ( hexagonIndex < hexagonCount ) {

	if ( true ) {
	//if (( hexagonIndex == 0 ) || ( hexagonIndex == 2 ) || ( hexagonIndex == 4 ) || ( hexagonIndex == 6 ) || ( hexagonIndex == 8 ) || ( hexagonIndex == 10 ) || ( hexagonIndex == 12 ) || ( hexagonIndex == 14 ) || ( hexagonIndex == 16 ) || ( hexagonIndex == 18 ) || ( hexagonIndex == 11 ) || ( hexagonIndex == 13 ) || ( hexagonIndex == 15 ) || ( hexagonIndex == 17 ) || ( hexagonIndex == 19 )) {


	triggerMan = 0;

	axis1Index = 0;
	axis2Index = 0;

	//while ( axis2Index < 1 ) {
	//	while ( axis1Index < 2 ) {
	while ( axis2Index < 4 ) {
		while ( axis1Index < 4 ) {

			//  PFM-NOTE: hosting all transformation math START
			HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = (( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 0 ]]  * chapeau_scale ) + ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale )) / 2.0;
			HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 2 ]] * chapeau_scale ) - HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ];

			HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ((( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 3 ]] * chapeau_scale ) + ( chapeau_Vertices[ hexagonVertexIndices[ hexagonIndex ][ 4 ]] * chapeau_scale )) / 2.0 ) - HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ];

			//  PFM-RESUME: continue converting over to big-ol'-static-storage approach to fix memory issues; debugging build issues from conversion, but looking better  

			//  Convert to unit length
			HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();
			HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();  // local "X+" axis
			HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();  //  local "Y+" axis 

			//  Convert over the matrices to simplify combination 

			HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4ThreeVector( 0.0 * mm, 0.0 * mm, -1.0 * mm );  //  counterintuitive; the face we want coupled to the scintillator is on the other side of the SiPM element so I'm flipping the Z sign 
			HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();

			//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: HB_CoG[ hexagonIndex ].dot( HB_zPosAxisOut ): " << HB_CoG[ hexagonIndex ].dot( HB_zPosAxisOut[ hexagonIndex ] ) << std::endl;

			if ( HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] )  < -0.9999 ) {

				//  It's effectively -1, just invert it and set the rotation to pi
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: hit " << std::endl;
				myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setX( 0.0 );
				myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setY( 1.0 );
				myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].setZ( 0.0 );
				//myCross1H = G4ThreeVector( 0.0 * mm, 1.0 * mm, 0.0 * mm );
				myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();
				angleDelta1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = pi;
			}
			else {

				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: miss " << std::endl;
				myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].cross( HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
				angleDelta1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( HB_zPosAxisOut[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
				//G4ThreeVector myCross1H = HB_zPosAxisOut.cross( HB_CoG );
				//G4double angleDelta1H = HB_zPosAxisOut.theta( HB_CoG );
			}

			
			HB_facetMatrix1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( myCross1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ], 1.0 * angleDelta1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//G4RotationMatrix HB_facetMatrix1 = G4RotationMatrix( myCross1H, -1.0 * angleDelta1H );

			//G4double angleDeltaDude = HB_axis2_mod.theta( zPosAxisUp );

			//  Matrix2

			//  Rotate the other frame vector to fix the orientation 
			HB_zPosAxis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4ThreeVector( 0.0 * mm, -1.0 * mm, 0.0 * mm );
			//G4ThreeVector HB_zPosAxis1 = G4ThreeVector( 0.0 * mm, 1.0 * mm, 0.0 * mm );
			HB_zPosAxis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_zPosAxis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] / HB_zPosAxis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].mag();

			HB_zPosAxis1New[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_facetMatrix1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * HB_zPosAxis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ];
			//G4ThreeVector HB_zPosAxis1New = HB_zPosAxis1.rotate( myCross1H, -1.0 * angleDelta1H );
			//G4ThreeVector HB_zPosAxis1New = HB_zPosAxis1.rotate( myCross1H, angleDelta1H );
			//G4ThreeVector myCross2 = HB_zPosAxis1New.cross( HB_axis2 );
			//angleDelta2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = -1.0 * HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( HB_zPosAxis1New[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			angleDelta2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].theta( HB_zPosAxis1New[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			//  What is the dot between CoG and the resulting cross of CoG's axis1 and the transformed HB_zPosAxis1? it's sign shuold be the answer I'm looking for
			G4double hackDot = HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].cross( HB_zPosAxis1New[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] ));

			//if ( hackDot > 0.0 ) {

			//  PFM-HACK: This workaround is inelegant, do better general-case 
			//  Don't kow why it isn't the same in the pentagons, fun math time later 
			if ( hexagonIndex < 10 ) {
				if ( hackDot < 0.0 ) {
					rotationTwister = -1.0;
				}
				else {
					rotationTwister = 1.0;
				}
			}
			else {
				if ( hackDot < 0.0 ) {
					rotationTwister = 1.0;
				}
				else {
					rotationTwister = -1.0;
				}
			}


			//if ( hackDot > 0.0 ) {
/*			if ( hackDot < 0.0 ) {
			//if (( hackDot < 0.0 ) && ( hexagonIndex < 6 )) {
				rotationTwister = 1.0;
			}
			else {
				rotationTwister = -1.0;
			}
*/
			dot2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].dot( HB_zPosAxis1New[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			//G4double angleDelta2H = HB_zPosAxis1New.theta( HB_axis2 );

			//  Math checking: if I invert the transform on the facet coordinate axes, both of them should be in the XY plane.  If they're not, something's weird 
			if ( triggerMan == 0 ) {  
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: axis1Index, axis2Index, hexagonIndex: " << axis1Index << ", " << axis2Index << ", " << hexagonIndex << std::endl;
				G4RotationMatrix HB_facetMatrix1Invert = HB_facetMatrix1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ];
				HB_facetMatrix1Invert.invert();
				G4ThreeVector HB_axis1_mod = HB_facetMatrix1Invert * HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ]; 
				G4ThreeVector HB_axis2_mod = HB_facetMatrix1Invert * HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ]; 
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: HB_axis1.x, .y, .z: " << HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].x() << ", " << HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].y() << ", " << HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: HB_axis2.x, .y, .z: " << HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].x() << ", " << HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].y() << ", " << HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ].z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: HB_axis1_mod.x, .y, .z: " << HB_axis1_mod.x() << ", " << HB_axis1_mod.y() << ", " << HB_axis1_mod.z() << std::endl;
				//if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: HB_axis2_mod.x, .y, .z: " << HB_axis2_mod.x() << ", " << HB_axis2_mod.y() << ", " << HB_axis2_mod.z() << std::endl;
				if (config.integer("verbose") > 0) std::cout << "DetCon::DetCon: hexagonIndex, angleDelta1H (rad), angleDelta2H (rad), dot2H, hackDot: " << hexagonIndex << ", " << angleDelta1H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << angleDelta2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << dot2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] << ", " << hackDot << std::endl;

				triggerMan = 1;
			}
			else {

				//  Please don't report anything, I only need it once

			}

			//G4RotationMatrix HB_facetMatrix2 = G4RotationMatrix( HB_CoG, 45.0 * deg );

			HB_facetMatrix2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ], rotationTwister * angleDelta2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//HB_facetMatrix2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = G4RotationMatrix( HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ], 1.0 * angleDelta2H[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			//G4RotationMatrix HB_facetMatrix2 = G4RotationMatrix( HB_CoG, -1.0 * angleDelta2H );  //  off by exactly 45 deg

		  	HB_home_Vertex[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = ( zero_Vertices[ 0 ] - ( HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * 1.5 * mySipmPitch ) - ( HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * 1.5 * mySipmPitch ));

			//  PFM-NOTE: hosting all transformation math END

			//myRotMatrixHex[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = new G4RotationMatrix( HB_facetMatrix1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			myRotMatrixHex[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] = new G4RotationMatrix( HB_facetMatrix1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * HB_facetMatrix2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );

			mHousings.at( (12 * 16 ) + ( hexagonIndex * 16 ) + ( axis1Index + ( axis2Index * 4 )))->buildAndPlace(world_PV, G4ThreeVector((( HB_home_Vertex[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] + ( HB_axis1[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * mySipmPitch * axis1Index ) + ( HB_axis2[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * mySipmPitch * axis2Index ))) + ( HB_CoG[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] * sensorCenterR )), myRotMatrixHex[ hexagonIndex ][ axis1Index + ( axis2Index * 4 ) ] );
			axis1Index = axis1Index + 1;
		}

		axis1Index = 0;
		axis2Index = axis2Index + 1;
	}
	}
	else {

		//  skip this hexagon
	}
	hexagonIndex = hexagonIndex + 1;
  }
  //  PFM-NOTE: modern way of placing SiPMs for A2/B(1) work #2 (END)


  G4int facetIndex = 7;
  //G4int facetIndex = 5;
  //G4int facetIndex = 0;
  //G4int facetCount = 8;
  //G4int facetCount = 10;
  //G4int facetCount = 14;

  int housingIndex = 0;

  //  PFM-NOTE: New array construction after this (until hitting old code) 

  G4float sipmPitch = 4.4 * mm;  //  HAMA
  //G4float sipmPitch = 3.2 * mm;  //  KETEK
  G4float sipmXStart = -6.6 * mm;  //  HAMA
  G4float sipmZStart = -6.6 * mm;  //  HAMA
  G4float mpccThickness = 0.10 * mm;
  //G4float mpccThickness = 0.15 * mm;
  //G4float sipmXStart = -11.2 * mm;  //  KETEK
  //G4float sipmZStart = -11.2 * mm;  //  KETEK
  //G4float sipmXStart = -12.0 * mm;
  //G4float sipmZStart = -12.0 * mm;
  //G4float sipmPitch = 3.3 * mm;
  //G4float sipmXStart = -11.65 * mm;
  //G4float sipmZStart = -11.65 * mm;

  //  siPM_Hex_A_

  G4float rotationAngle = -1.0 * ( facetIndex * ( 360.0 / 14.0 )) * deg; 
  G4RotationMatrix* rm_xMoo = new G4RotationMatrix();

  //  Additional rotations to get the SiPMs into their proper locations
  //rm_xMoo->rotateZ( -1 * rotationAngle );

  //rm_xMoo->rotateZ(-1 * 45.0*deg);
  //rm_xMoo->rotateX(-90*deg);

  G4RotationMatrix* rm_Position = new G4RotationMatrix();
  rm_Position->rotateZ( -1 * rotationAngle );

  int sipmX = 0;
  int sipmZ = 0;
  int sipmArrayIndex = 0;
  int arraySize = 16;  //  HAMA
  //int arraySize = 64;  //  KETEK
  int hexArrayIndex = 0;

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 1 ) {

  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 2 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1
      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 

      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 0, 1 ));

      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

      myVec.rotate( HexRing1Angle * deg, G4ThreeVector( 0, -1, 0 ));

      //myVec.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));

      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));

	G4RotationMatrix *rotMatrixH1 = new G4RotationMatrix();
	rotMatrixH1->rotate(( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	rotMatrixH1->rotate( HexRing1Angle * deg, axisOfRotation2 );
	rotMatrixH1->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrixH1 );  //  Test: ... 

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrix2 );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }

  sipmX = 0;
  sipmZ = 0;
  sipmArrayIndex = 1;
  hexArrayIndex = 1;

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 1 ) {
  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 2 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1

      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //  reversing the order of the rotations
      myVec.rotate( HexRing1Angle * deg, G4ThreeVector( 0, -1, 0 ));
      myVec.rotate(( hexArrayIndex * 72.0 ) * deg, G4ThreeVector( 0, 0, 1 ));
      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));


      //myVec.rotate( -1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

/*
      myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( -90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));

      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      myVec.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
*/
      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));


	G4RotationMatrix *rotMatrixH2 = new G4RotationMatrix();
	rotMatrixH2->rotate( - 1.0 * ( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	rotMatrixH2->rotate( HexRing1Angle * deg, axisOfRotation2 );
	rotMatrixH2->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around


	//G4RotationMatrix *rotMatrixH2 = new G4RotationMatrix();
	//rotMatrixH2->rotate( -1.0 + ( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	//rotMatrixH2->rotate( 1.0 * HexRing1Angle * deg, axisOfRotation2 );
	//rotMatrixH2->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrixH2 );  //  Test: ... 

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, 0 );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrix3 );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }
  sipmX = 0;
  sipmZ = 0;
  sipmArrayIndex = 2;
  hexArrayIndex = 2;

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1
      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //  reversing the order of the rotations
      myVec.rotate( HexRing1Angle * deg, G4ThreeVector( 0, -1, 0 ));
      myVec.rotate(( hexArrayIndex * 72.0 ) * deg, G4ThreeVector( 0, 0, 1 ));
      //myVec.rotate( -1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

/*
      myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( -90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));

      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      myVec.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
*/
      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));

	G4RotationMatrix *rotMatrixH3 = new G4RotationMatrix();
	rotMatrixH3->rotate( - 1.0 * ( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	rotMatrixH3->rotate( HexRing1Angle * deg, axisOfRotation2 );
	rotMatrixH3->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrixH3 );  //  Test: ... 

      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }

  sipmX = 0;
  sipmZ = 0;
  sipmArrayIndex = 3;
  hexArrayIndex = 3;

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1
      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //  reversing the order of the rotations
      myVec.rotate( HexRing1Angle * deg, G4ThreeVector( 0, -1, 0 ));
      myVec.rotate(( hexArrayIndex * 72.0 ) * deg, G4ThreeVector( 0, 0, 1 ));
      //myVec.rotate( -1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

/*
      myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( -90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));

      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      myVec.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
*/
      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));

	G4RotationMatrix *rotMatrixH4 = new G4RotationMatrix();
	rotMatrixH4->rotate( - 1.0 * ( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	rotMatrixH4->rotate( HexRing1Angle * deg, axisOfRotation2 );
	rotMatrixH4->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrixH4 );  //  Test: ... 

      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }

  sipmX = 0;
  sipmZ = 0;
  sipmArrayIndex = 4;
  hexArrayIndex = 4;

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1
      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //  reversing the order of the rotations
      myVec.rotate( HexRing1Angle * deg, G4ThreeVector( 0, -1, 0 ));
      myVec.rotate(( hexArrayIndex * 72.0 ) * deg, G4ThreeVector( 0, 0, 1 ));
      //myVec.rotate( -1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

/*
      myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( 90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));
      //myVec.rotate((( -90.0 / 180.0 ) + ( hexArrayIndex * 72.0 )) * M_PI, G4ThreeVector( 0, 1, 0 ));

      //myVec.rotate( ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));
      myVec.rotate((( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 0, 0, 1 ));
*/
      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));

	G4RotationMatrix *rotMatrixH5 = new G4RotationMatrix();
	rotMatrixH5->rotate( - 1.0 * ( HexRing1Step * hexArrayIndex ) * deg, axisOfRotation3 );
	rotMatrixH5->rotate( HexRing1Angle * deg, axisOfRotation2 );
	rotMatrixH5->rotate( 180.0 * deg, axisOfRotation2 );  //  need to turn it around

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rotMatrixH5 );  //  Test: ... 

      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }

  //  PFM-NOTE: This is for placement of the P1 array, replacement in progress...
/*
  sipmX = 0;
  sipmZ = 0;
  sipmArrayIndex = 5;
  rm_xMoo->rotateX( 180.0 * deg );
  //rm_xMoo->rotateX( 90 * deg );

  while ( sipmX < 4 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 9 ) {
  //while ( sipmX < 1 ) {
  //while ( sipmX < 8 ) {

    while ( sipmZ < 4 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 9 ) {
    //while ( sipmZ < 2 ) {
    //while ( sipmZ < 8 ) {

	//  PFM-TWEAK1

      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB ) + mpccThickness ); 

      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, 1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( hemisphereRadiusOuterScaled + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( 50.0 + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -50.0 * mm ); 

      //  reversing the order of the rotations
      myVec.rotate( 0.0, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( -1.0 * ( 90.0 / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( -1.0 * (( 90.0 - HexRing1Angle ) / 180.0 ) * M_PI, G4ThreeVector( 1, 0, 0 ));
      //myVec.rotate( (( 90.0 + ( hexArrayIndex * 72.0 )) / 180.0 ) * M_PI, G4ThreeVector( 0, 1, 0 ));

      //G4ThreeVector myVec = G4ThreeVector( 0.0 * mm, 0.0 * mm, -50.0 * mm );    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), sipmZStart + ( sipmZ * sipmPitch, yOffset ));    
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ), yOffset, sipmZStart + ( sipmZ * sipmPitch ));    
      //myVec.rotate( rotationAngle, G4ThreeVector( 0, 0, 1 ));

      //mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, 0 );  //  Test: ... 
      mHousings.at( housingIndex + ( sipmArrayIndex * arraySize ) + ( sipmZ + ( sipmX * 4 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec2, rotMatrix2 );
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 9 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->setPosition(G4ThreeVector());
      //mHousings.at( housingIndex + ( sipmZ + ( sipmX * 8 )))->buildAndPlace(world_PV, myVec, rm_xMoo );

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }
*/
  //  END - code for funky SiPM placement 

//  BEGIN - code for traditional placement of SiPM elements 
/*
  int sipmX = 0;
  int sipmZ = 0;
  G4float sipmPitch = 3.0 * mm;
  G4float sipmXStart = -12.0 * mm;
  G4float sipmZStart = -12.0 * mm;
  //G4float sipmPitch = 3.2 * mm;
  //G4float sipmXStart = ( -6.5 + ( 0.2 + ( 3.0 / 2.0 ))) * mm;
  //G4float sipmZStart = ( -6.5 + ( 0.2 + ( 3.0 / 2.0 ))) * mm;

  G4float rotationAngle = 90.0 * deg; 
  G4RotationMatrix* rm_xMoo = new G4RotationMatrix();
  rm_xMoo->rotateX( -1.0 * rotationAngle );

//  while ( sipmX < 9 ) {
  //  while ( sipmZ < 9 ) {
  while ( sipmX < 1 ) {
    while ( sipmZ < 1 ) {

// lightguideThicknessAB
// scint_outer_radius 

      G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -1.0 * ( scint_outer_radius + lightguideThicknessAB )); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, ( -51.327 - 2.5 ) * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, ( -51.327 - 10.0 ) * mm ); 
      //G4ThreeVector myVec = G4ThreeVector( sipmXStart + ( sipmX * sipmPitch ) * mm, sipmZStart + ( sipmZ * sipmPitch ) * mm, -60.0 * mm ); 
      myVec.rotate( -90.0 * deg, G4ThreeVector( 1, 0, 0 ));
      mHousings.at( sipmZ + ( sipmX * 9 ))->buildAndPlace(world_PV, myVec, rm_xMoo );  //  Test: ... 

      sipmZ = sipmZ + 1;
    }

    sipmZ = 0;
    sipmX = sipmX + 1;
  }
*/
//  END - code for traditional placement of SiPM elements
 
  return world_PV;
}

G4SipmModel* DetectorConstruction::getSipmModel() const {
  return housing->getSipm()->getModel();
}

G4SipmHousing* DetectorConstruction::getSipmHousing() const {
  return housing;
}

