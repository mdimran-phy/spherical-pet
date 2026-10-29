// SPDX-License-Identifier: GPL-3.0-only
#include "GuideShape.hh"
#include <G4TessellatedSolid.hh>
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool ok, const char* message) {
    if (!ok)
        throw std::runtime_error(message);
}
int main() {
    try {
        std::vector<G4ThreeVector> hexagon;
        for (int i = 0; i < 6; ++i) {
            const double angle = i * std::acos(-1.) / 3.;
            hexagon.emplace_back(12. * std::cos(angle), 12. * std::sin(angle), 34.5);
        }
        const G4ThreeVector outlet[] = {{-9, -9, 39.5}, {9, -9, 39.5}, {9, 9, 39.5}, {-9, 9, 39.5}};
        for (double fraction : {0.01, 0.25, 0.49}) {
            auto octagon = bevelGuideHexagon(hexagon, fraction);
            require(octagon.size() == 8, "Expected eight shoulder edges");
            for (size_t i = 0; i < octagon.size(); ++i)
                require((octagon[(i + 1) % 8] - octagon[i])
                                .cross(octagon[(i + 2) % 8] - octagon[(i + 1) % 8])
                                .mag() > 1e-6,
                        "Degenerate octagon edge");
            for (int c : {0, 3}) {
                const auto corner = hexagon[c];
                const auto a = corner + fraction * (hexagon[(c + 5) % 6] - corner);
                const auto b = corner + fraction * (hexagon[(c + 1) % 6] - corner);
                auto* cut = guideCornerCut("testCut", corner, hexagon[(c + 5) % 6],
                                           hexagon[(c + 1) % 6], outlet, fraction);
                require(cut->GetPolyhedron() != nullptr, "Missing cut display mesh");
                require(cut->GetSolidClosed(), "Cut must be closed");
                require(cut->GetNumberOfFacets() == 6, "Cut must have six boundary triangles");
                auto bottom = (corner + a + b + 0.85 * corner) / 4.;
                require(cut->Inside(bottom) == kInside, "Lower cut tetrahedron not navigable");
                G4ThreeVector top = outlet[0];
                for (auto candidate : outlet)
                    if ((candidate - corner).mag2() < (top - corner).mag2())
                        top = candidate;
                require(cut->Inside((corner + a + b + top) / 4.) == kInside,
                        "Upper cut tetrahedron not navigable");
                require(cut->Inside(G4ThreeVector(0, 0, 34.5)) == kOutside,
                        "Bevel reaches guide center");
                require(cut->Inside(top) == kSurface, "Outlet contact is not the bevel tip");
                const auto direction = G4ThreeVector(0, 0, 34.5) - bottom;
                require(cut->DistanceToOut(bottom, direction.unit(), false, nullptr, nullptr) > 0,
                        "Invalid cut navigation distance");
                delete cut;
            }
        }
        std::cout << "PASS: octagonal shoulder and navigable bevel solids\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
