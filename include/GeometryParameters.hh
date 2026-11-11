// SPDX-License-Identifier: GPL-3.0-only
#pragma once

// Fixed dimensions of the supplied geometry, expressed in mm without a Geant4
// dependency so configuration validation uses the same limits. Runtime study
// parameters (shell thickness, pitch and guide bevel) remain in Config.
namespace GeometryParameters {
inline constexpr int pentagonCount = 12;
inline constexpr int hexagonCount = 20;
inline constexpr int sensorsPerSide = 4;
inline constexpr int sensorsPerArray = sensorsPerSide * sensorsPerSide;
inline constexpr int sensorCount = (pentagonCount + hexagonCount) * sensorsPerArray;
inline constexpr double worldHalfLengthMm = 100.0;
inline constexpr double shellOuterRadiusMm = 34.5;
inline constexpr double guideOutletRadiusMm = 39.5;
inline constexpr double guideOutletHalfWidthMm = 9.0;
// Each hemisphere supplies half of the 0.1 mm equatorial epoxy gasket.
inline constexpr double halfGasketThicknessMm = 0.05;
inline constexpr double innerLinerThicknessMm = 1.0;
inline constexpr double maskExtensionScale = 2.0;
static_assert(sensorCount > 0, "Detector must contain SiPMs");
} // namespace GeometryParameters
