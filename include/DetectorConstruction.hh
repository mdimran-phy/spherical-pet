/*
 * DetectorConstruction.cc
 *
 * @date Sep 12, 2011
 * @author Tim Niggemann, III Phys. Inst. A, RWTH Aachen University
 * @copyright GNU General Public License v3.0
 */

#pragma once
#include "Config.hh"
#include "G4RotationMatrix.hh"
#include "G4VUserDetectorConstruction.hh"
#include <memory>
#include <string>
#include <vector>

class G4Sipm;
class G4SipmModel;
class G4SipmHousing;

// Spherical LYSO detector, faceted guide, and 32 arrays of 4 x 4 SiPMs.
class DetectorConstruction : public G4VUserDetectorConstruction {
  public:
    explicit DetectorConstruction(const Config& settings);
    ~DetectorConstruction() override;
    G4VPhysicalVolume* Construct() override;
    G4SipmModel* getSipmModel() const;
    G4SipmHousing* getSipmHousing() const;

  private:
    const Config& config;
    // Destruction is in reverse declaration order: housings before shared model.
    std::unique_ptr<G4SipmModel> model;
    std::vector<std::unique_ptr<G4RotationMatrix>> rotations;
    std::vector<std::unique_ptr<G4SipmHousing>> mHousings;
    G4SipmHousing* housing = nullptr; // Non-owning alias of the first housing.

    G4SipmModel* createSipmModel(std::string name) const;
    std::unique_ptr<G4SipmHousing> createHousing(const std::string& name,
                                                 std::unique_ptr<G4Sipm> sipm) const;
    G4RotationMatrix* keepRotation(const G4RotationMatrix& rotation);
};
