// SPDX-License-Identifier: GPL-3.0-only
#include "GuideShape.hh"
#include <G4TessellatedSolid.hh>
#include <G4TriangularFacet.hh>
#include <algorithm>
#include <cmath>
#include <stdexcept>

std::vector<G4ThreeVector> bevelGuideHexagon(const std::vector<G4ThreeVector>& ring, double f) {
    if (ring.size() != 6 || !std::isfinite(f) || f <= 0 || f >= 0.5)
        throw std::runtime_error("Invalid hexagon bevel");
    std::vector<G4ThreeVector> result;
    for (int i = 0; i < 6; ++i) {
        if (i == 0 || i == 3) {
            result.push_back(ring[i] + f * (ring[(i + 5) % 6] - ring[i]));
            result.push_back(ring[i] + f * (ring[(i + 1) % 6] - ring[i]));
        } else
            result.push_back(ring[i]);
    }
    return result;
}

G4TessellatedSolid* guideCornerCut(const G4String& name, const G4ThreeVector& corner,
                                   const G4ThreeVector& before, const G4ThreeVector& after,
                                   const G4ThreeVector* outlet, double fraction) {
    const G4ThreeVector triangle[] = {corner, corner + fraction * (before - corner),
                                      corner + fraction * (after - corner)};
    auto nearest = std::min_element(outlet, outlet + 4, [&](const auto& a, const auto& b) {
        return (a - corner).mag2() < (b - corner).mag2();
    });
    const G4ThreeVector tips[] = {0.85 * corner, *nearest};
    auto* solid = new G4TessellatedSolid(name);
    // Two tetrahedra joined at their triangular bases, represented by one closed
    // six-triangle surface. The lower tip is inside the scintillator's outer orb;
    // the upper tip touches (and removes no finite area from) the square outlet.
    for (const auto& tip : tips) {
        auto inside = (triangle[0] + triangle[1] + triangle[2] + tip) / 4.0;
        for (int i = 0; i < 3; ++i) {
            auto a = triangle[i], b = triangle[(i + 1) % 3];
            if ((b - a).cross(tip - a).dot(inside - a) > 0)
                std::swap(a, b);
            if (!solid->AddFacet(new G4TriangularFacet(a, b, tip, ABSOLUTE)))
                throw std::runtime_error("Cannot add corner-cut facet");
        }
    }
    solid->SetSolidClosed(true);
    if (!solid->GetSolidClosed())
        throw std::runtime_error("Guide corner cut is not closed");
    return solid;
}
