/*
 * DetectorConstruction.hh
 *
 * @date Sep 12, 2011
 * @author Tim Niggemann, III Phys. Inst. A, RWTH Aachen University
 * @copyright GNU General Public License v3.0
 */

#ifndef DETECTORCONSTRUCTION_HH_
#define DETECTORCONSTRUCTION_HH_

#include "Config.hh"
#include <G4ThreeVector.hh>
#include <G4RotationMatrix.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

#include "G4Material.hh"

#include "G4Box.hh"

#include "G4Sipm.hh"
#include "housing/G4SipmHousing.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4MaterialPropertiesTable.hh"

#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4Polyhedra.hh"
#include "G4Sphere.hh"
#include "G4Tet.hh"
#include "G4SubtractionSolid.hh"
#include "G4Orb.hh"
#include "G4TessellatedSolid.hh"  
#include "G4QuadrangularFacet.hh"
#include "G4TriangularFacet.hh"
#include "G4UnionSolid.hh"
#include "G4MultiUnion.hh"

#include "G4Transform3D.hh"
//#include "Point3D.h"

/**
 * Construct the supplied spherical LYSO detector, faceted guide, and SiPM arrays.
 */
class DetectorConstruction: public G4VUserDetectorConstruction {
private:
  const Config& config;
  G4SipmHousing* housing;
  G4SipmHousing* housing2;  
  G4SipmHousing* housing3;  
  G4SipmHousing* housing4;  

  G4SipmHousing* housing5;
  G4SipmHousing* housing6;  
  G4SipmHousing* housing7;  
  G4SipmHousing* housing8;  

  G4SipmHousing* housing9;
  G4SipmHousing* housing10;  
  G4SipmHousing* housing11;  
  G4SipmHousing* housing12;  

  G4SipmHousing* housing13;
  G4SipmHousing* housing14;  
  G4SipmHousing* housing15;  
  G4SipmHousing* housing16;  

  std::vector<G4SipmHousing*> mHousings;

  G4Element* fLu;
  G4Element* fY;
  G4Element* fSi;
  G4Element* fO;
  G4Material* fLYSO;
  G4MaterialPropertiesTable* fLYSO_mt;

  G4Box *fTestScint_box;
  G4LogicalVolume* fTestScint_LV;
  G4VPhysicalVolume *fTestScint_PV; 

  G4Box *fTestWater1_box;
  G4LogicalVolume *fTestWater1_LV;
  G4VPhysicalVolume *fTestWater1_PV; 
  G4Box *fTestWater2_box;
  G4LogicalVolume *fTestWater2_LV;
  G4VPhysicalVolume *fTestWater2_PV; 
  G4Box *fTestWater3_box;
  G4LogicalVolume *fTestWater3_LV;
  G4VPhysicalVolume *fTestWater3_PV; 
  G4Box *fTestWater4_box;
  G4LogicalVolume *fTestWater4_LV;
  G4VPhysicalVolume *fTestWater4_PV; 
  G4Box *fTestWater5_box;
  G4LogicalVolume *fTestWater5_LV;
  G4VPhysicalVolume *fTestWater5_PV; 

  G4LogicalVolume* fTestWater_LV;
  G4VPhysicalVolume *fTestWaterEndcap1_PV; 
  G4VPhysicalVolume *fTestWaterEndcap2_PV; 

  G4Polyhedra* scint_poly; 
  G4Tubs* scint_tube;
  G4Tubs* scint_tube_donut_hole;
  G4UnionSolid* union_solid_scint;

  G4Sphere* scint_hemisphere;
  G4Sphere* scint_hemisphere_virgin;

  G4SubtractionSolid *hemisphereNorthScint;
  G4LogicalVolume *hemisphereNorthScint_LV;
  G4VPhysicalVolume *hemisphereNorthScint_PV;
  G4SubtractionSolid *hemisphereNorthGasket;
  G4LogicalVolume *hemisphereNorthGasket_LV;
  G4VPhysicalVolume *hemisphereNorthGasket_PV;

  G4SubtractionSolid *hemisphereSouthScint;
  G4LogicalVolume *hemisphereSouthScint_LV;
  G4VPhysicalVolume *hemisphereSouthScint_PV;
  G4SubtractionSolid *hemisphereSouthGasket;
  G4LogicalVolume *hemisphereSouthGasket_LV;
  G4VPhysicalVolume *hemisphereSouthGasket_PV;

  G4SubtractionSolid *gasketSubtraction;
  G4LogicalVolume *gasketSubtraction_LV;
  G4VPhysicalVolume *gasketSubtraction_PV;
  G4Box *gasketBox;
  G4LogicalVolume *gasketBox_LV;
  G4VPhysicalVolume *gasketBox_PV; 
  G4Box *gasketBox2;
  G4LogicalVolume *gasketBox2_LV;
  G4VPhysicalVolume *gasketBox2_PV; 
  G4Polyhedra* hexagon_A;
  G4LogicalVolume *hexagon_A_LV;
  G4LogicalVolume *hexagon_A_Fake_LV;
  G4VPhysicalVolume *hexagon_A_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexA_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexB_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexC_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexD_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexE_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexF_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexG_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexH_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexI_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexJ_PV;
  G4VPhysicalVolume *hexagon_A_DummyHexK_PV;
  G4Polyhedra* pentagon_A;
  G4LogicalVolume *pentagon_A_LV;
  G4VPhysicalVolume *pentagon_A_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentA_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentB_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentC_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentD_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentE_PV;
  G4VPhysicalVolume *pentagon_A_DummyPentF_PV;

  G4TessellatedSolid *pentagon_B;
  G4TriangularFacet *pentagon_B_Facets[ 16 ];
  G4LogicalVolume *pentagon_B_LV;
  G4VPhysicalVolume *pentagon_B_PV; 

  G4Sphere* innermost_hemisphere;
  G4LogicalVolume *innermost_hemisphere_LV;
  G4VPhysicalVolume *innermost_hemisphere_PV; 

  G4Orb* lg_orb_void;
  G4SubtractionSolid *hexagon_A_MinusOrb;
  G4SubtractionSolid *pentagon_A_MinusOrb;
  G4TessellatedSolid *hexagon_B;
  G4TriangularFacet *hexagon_B_Facets[ 16 ];
  //G4TriangularFacet *hexagon_B_Facets[ 14 ];
  G4LogicalVolume *hexagon_B_LV;
  G4VPhysicalVolume *hexagon_B_A_PV; 
  G4VPhysicalVolume *hexagon_B_B_PV; 
  G4VPhysicalVolume *hexagon_B_C_PV; 
  G4VPhysicalVolume *hexagon_B_D_PV; 
  G4VPhysicalVolume *hexagon_B_E_PV; 

  G4TessellatedSolid *testSolidCube;
  G4TriangularFacet *cubeTestFacets[ 12 ];
  G4LogicalVolume *cubeTest_LV;
  G4VPhysicalVolume *cubeTest_PV; 

  G4Tubs *lightguideBTubs;
  G4SubtractionSolid *lightguideShellTubMinusB;
  G4LogicalVolume *lightguideShellTubMinusB_LV; 
  G4VPhysicalVolume *lightguideShellTubMinusB_PV;

  G4Box *lightguideBBox;
  G4Box *pentagon_B_BBox;
  G4Box *hexagon_B_BBox1;
  G4Box *hexagon_B_BBox2;
  G4Box *hexagon_B_BBox3;
  G4Box *hexagon_B_BBox4;
  G4Box *hexagon_B_BBox5;

  G4SubtractionSolid *lightguideShellBoxMinusB;

  G4SubtractionSolid *pentagon_B_BBox_Minus_pentagon_B;
  G4SubtractionSolid *hexagon_B_BBox1_Minus_hexagon_B;
  G4SubtractionSolid *hexagon_B_BBox2_Minus_hexagon_B;
  G4SubtractionSolid *hexagon_B_BBox3_Minus_hexagon_B;
  G4SubtractionSolid *hexagon_B_BBox4_Minus_hexagon_B;
  G4SubtractionSolid *hexagon_B_BBox5_Minus_hexagon_B;

  G4LogicalVolume *pentagon_B_BBox_Minus_pentagon_B_LV; 
  G4LogicalVolume *hexagon_B_BBox1_Minus_hexagon_B_LV; 
  G4LogicalVolume *hexagon_B_BBox2_Minus_hexagon_B_LV; 
  G4LogicalVolume *hexagon_B_BBox3_Minus_hexagon_B_LV; 
  G4LogicalVolume *hexagon_B_BBox4_Minus_hexagon_B_LV; 
  G4LogicalVolume *hexagon_B_BBox5_Minus_hexagon_B_LV; 

  G4VPhysicalVolume *pentagon_B_BBox_Minus_pentagon_B_PV;
  G4VPhysicalVolume *hexagon_B_BBox1_Minus_hexagon_B_PV;
  G4VPhysicalVolume *hexagon_B_BBox2_Minus_hexagon_B_PV;
  G4VPhysicalVolume *hexagon_B_BBox3_Minus_hexagon_B_PV;
  G4VPhysicalVolume *hexagon_B_BBox4_Minus_hexagon_B_PV;
  G4VPhysicalVolume *hexagon_B_BBox5_Minus_hexagon_B_PV;

  G4LogicalVolume *lightguideShellBoxMinusB_LV; 
  G4VPhysicalVolume *lightguideShellBoxMinusB_PV;

  G4Box *placement_test_Box;
  G4LogicalVolume *placement_test_Box_LV;
  G4VPhysicalVolume *placement_test_Box_PV;

  G4Orb* orb_surface;
  G4LogicalVolume *orb_surface_LV;
  G4VPhysicalVolume *orb_surface_PV; 

  G4Box *tinkerBox1;
  G4Box *tinkerBox2;
  G4Box *tinkerBox3;
  G4SubtractionSolid *tinkerBox1MinusTinkerBox2;
  G4SubtractionSolid *tinkerBox1MinusTinkerBox2MinusTinkerBox3;
  G4LogicalVolume *tinkerBox1MinusTinkerBox2_LV;
  G4VPhysicalVolume *tinkerBox1MinusTinkerBox2_PV;

  G4Box *hackGeo1;
  G4SubtractionSolid *hemisphereMinusHack;

  G4Box *hackScint1;
  G4LogicalVolume *hackScint1_LV;
  G4VPhysicalVolume *hackScint1_PV;
  G4Box *hackScint1of4;
  G4LogicalVolume *hackScint1of4_LV;
  G4VPhysicalVolume *hackScint1of4_PV;
  G4Box *hackScint2of4;
  G4LogicalVolume *hackScint2of4_LV;
  G4VPhysicalVolume *hackScint2of4_PV;
  G4Box *hackScint3of4;
  G4LogicalVolume *hackScint3of4_LV;
  G4VPhysicalVolume *hackScint3of4_PV;
  G4Box *hackScint4of4;
  G4LogicalVolume *hackScint4of4_LV;
  G4VPhysicalVolume *hackScint4of4_PV;



  G4Tubs *waterTubs;
  G4SubtractionSolid *waterTubsMinusScintBox;
  G4LogicalVolume *waterTubsMinusScintBox_LV;
  G4VPhysicalVolume *waterTubsMinusScintBox_PV; 

  G4Polyhedra* pentagonPyramid;
  G4LogicalVolume *pentagonPyramid_LV;
  G4VPhysicalVolume *pentagonPyramid_PV; 

  G4Polyhedra* hexagonPyramid;
  G4LogicalVolume *hexagonPyramid_LV;
  G4VPhysicalVolume *hexagonPyramid_PV; 

  G4UnionSolid *pentUnionHex;
  G4MultiUnion *chapeauMultiUnion;
  G4LogicalVolume *chapeauMultiUnion_LV;
  G4VPhysicalVolume *chapeauMultiUnion_PV; 

  G4TessellatedSolid *chapeauPoof;
  G4TriangularFacet *chapeauPoof_H1A_Facets[ 10 ];  
  G4TriangularFacet *chapeauPoof_P1A_Facets[ 8 ];  
  G4TriangularFacet *chapeauPoof_H2A_Facets[ 10 ];  
  G4TriangularFacet *chapeauPoof_H3A_Facets[ 10 ];  
  G4TriangularFacet *chapeauPoof_H4A_Facets[ 10 ];  
  G4TriangularFacet *chapeauPoof_H5A_Facets[ 10 ];  
  G4TriangularFacet *chapeauPoof_H1B_Facets[ 22 ];  
  G4TriangularFacet *chapeauPoof_H2B_Facets[ 22 ];  
  G4TriangularFacet *chapeauPoof_H3B_Facets[ 22 ];  
  G4TriangularFacet *chapeauPoof_H4B_Facets[ 22 ];  
  G4TriangularFacet *chapeauPoof_H5B_Facets[ 22 ];  
  G4TriangularFacet *chapeauPoof_P1B_Facets[ 21 ]; 

  G4TessellatedSolid *chapeauPoof_A2;
  G4TriangularFacet *chapeauPoof_A2_PentagonFacets[ 36 ]; 
  G4TriangularFacet *chapeauPoof_A2_HexagonFacets[ 80 ]; 
  G4TriangularFacet *chapeauPoof_A2_SkirtFacets[ 20 ]; 
  G4TriangularFacet *chapeauPoof_A2_SquareHexagonFacets[ 2 * 20 ]; 
  G4TriangularFacet *chapeauPoof_A2_SquarePentagonFacets[ 2 * 12 ]; 
  G4TriangularFacet *chapeauPoof_A2_SquareHexagonSkirtFacets[ 10 * 20 ]; 
  G4TriangularFacet *chapeauPoof_A2_SquarePentagonSkirtFacets[ 9 * 12 ]; 

  G4TessellatedSolid *chapeauCasting_A2;
  G4TriangularFacet *chapeauCasting_A2_PentagonFacets[ 36 ]; 
  G4TriangularFacet *chapeauCasting_A2_HexagonFacets[ 80 ]; 
  G4TriangularFacet *chapeauCasting_A2_SkirtFacets[ 20 ]; 
  G4TriangularFacet *chapeauCasting_A2_SquareHexagonFacets[ 2 * 20 ]; 
  G4TriangularFacet *chapeauCasting_A2_SquarePentagonFacets[ 2 * 12 ]; 
  G4TriangularFacet *chapeauCasting_A2_SquareHexagonSkirtFacets[ 10 * 20 ]; 
  G4TriangularFacet *chapeauCasting_A2_SquarePentagonSkirtFacets[ 9 * 12 ]; 

  G4TriangularFacet *chapeauCasting_A2_SquareHexagonExtensionFacets[ 10 * 20 ]; 
  G4TriangularFacet *chapeauCasting_A2_SquarePentagonExtensionFacets[ 10 * 12 ]; 
 
  //G4TriangularFacet *chapeau_Facets[ 200 ];  
  //G4TriangularFacet *chapeauPoof_Facets[ 200 ];  
  G4LogicalVolume *chapeauPoof_LV;
  G4VPhysicalVolume *chapeauPoof_PV; 
  G4LogicalVolume *chapeauPoof_A2_LV;
  G4VPhysicalVolume *chapeauPoof_A2_PV; 

  G4Sphere *homeSphere;
  G4LogicalVolume *homeSphere_LV;
  G4VPhysicalVolume *homeSphere_PV; 
  G4SubtractionSolid *chapeauPoofMinusHomeOrb;
  G4LogicalVolume *chapeauPoofMinusHomeOrb_LV;
  G4VPhysicalVolume *chapeauPoofMinusHomeOrb_PV; 
  G4SubtractionSolid *homeSphereMinusChapeauPoof;
  G4LogicalVolume *homeSphereMinusChapeauPoof_LV;
  G4VPhysicalVolume *homeSphereMinusChapeauPoof_PV; 
  G4SubtractionSolid *homeSphereMinusChapeauCasting;
  G4LogicalVolume *homeSphereMinusChapeauCasting_LV;
  G4VPhysicalVolume *homeSphereMinusChapeauCasting_PV; 

  G4Sphere* scintShellCollar;
  G4LogicalVolume *scintShellCollar_LV;
  G4VPhysicalVolume *scintShellCollar_PV; 
  G4SubtractionSolid *scintShellCollarMinusChapeauPoof;
  G4LogicalVolume *scintShellCollarMinusChapeauPoof_LV;
  G4VPhysicalVolume *scintShellCollarMinusChapeauPoof_PV; 

  G4TessellatedSolid *chapeauJig;
  G4TriangularFacet *chapeauJig_Facets[ 109 ];  
  G4LogicalVolume *chapeauJig_LV;
  G4VPhysicalVolume *chapeauJig_PV; 
  G4SubtractionSolid *chapeauJigMinusChapeauPoof;
  G4SubtractionSolid *chapeauJigMinusHomeOrb;
  G4LogicalVolume *chapeauJigMinusHomeOrb_LV;
  G4VPhysicalVolume *chapeauJigMinusHomeOrb_PV; 

  G4TessellatedSolid *chapeauCasting;
  G4TriangularFacet *chapeauCasting_H1A_Facets[ 10 ];  
  G4TriangularFacet *chapeauCasting_P1A_Facets[ 8 ];  
  G4TriangularFacet *chapeauCasting_H2A_Facets[ 10 ];  
  G4TriangularFacet *chapeauCasting_H3A_Facets[ 10 ];  
  G4TriangularFacet *chapeauCasting_H4A_Facets[ 10 ];  
  G4TriangularFacet *chapeauCasting_H5A_Facets[ 10 ];  
  G4TriangularFacet *chapeauCasting_H1B_Facets[ 22 ];  
  G4TriangularFacet *chapeauCasting_H2B_Facets[ 22 ];  
  G4TriangularFacet *chapeauCasting_H3B_Facets[ 22 ];  
  G4TriangularFacet *chapeauCasting_H4B_Facets[ 22 ];  
  G4TriangularFacet *chapeauCasting_H5B_Facets[ 22 ];  
  G4TriangularFacet *chapeauCasting_P1B_Facets[ 21 ];  
  G4LogicalVolume *chapeauCasting_LV;
  G4VPhysicalVolume *chapeauCasting_PV; 
  //G4TriangularFacet *chapeauCasting_Facets[ 200 ];  

  /**
   * @param name - the name of the model.
   * @return G4SipmModel - the new instance.
   */
  G4SipmModel* createSipmModel(std::string name) const;
  /**
   * @param name - the name of the housing.
   * @param sipm - the SiPM instance.
   * @return G4SipmHousing - the new instance.
   */
  G4SipmHousing* createHousing(std::string name, G4Sipm* sipm) const;

public:
  /**
   * Constructor.
   *
   * @param modelName - the model name (see G4SipmModelFactory).
   * @param housingName - the name of the housing ("ceramic", "smd", "plain").
   */
  explicit DetectorConstruction(const Config& settings);
  virtual ~DetectorConstruction();

  G4VPhysicalVolume* Construct();

  /**
   * @return G4SipmModel - the SiPM model.
   */
  G4SipmModel* getSipmModel() const;
  /**
   * @return G4SipmHousing - the housing instance.
   */
  G4SipmHousing* getSipmHousing() const;

};

#endif /* G4SIPMDETECTORCONSTRUCTION_HH_ */

