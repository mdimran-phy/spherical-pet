// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <G4String.hh>
#include <G4ThreeVector.hh>
#include <vector>
class G4TessellatedSolid;
// An ordered hexagon becomes an irregular octagon by cutting corners 0 and 3.
std::vector<G4ThreeVector> bevelGuideHexagon(const std::vector<G4ThreeVector>& ring,
                                             double fraction);
// Local corner bevel, tapering to zero inside the shell and at an outlet corner.
G4TessellatedSolid* guideCornerCut(const G4String& name, const G4ThreeVector& corner,
                                   const G4ThreeVector& before, const G4ThreeVector& after,
                                   const G4ThreeVector* outlet, double fraction);
