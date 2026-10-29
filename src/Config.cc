// SPDX-License-Identifier: GPL-3.0-only
#include "Config.hh"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
std::string trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
} // namespace
const std::vector<Option>& Config::options() {
    static const std::vector<Option> list = {
        {"mode", "-r", "simulation",
         "simulation (0) or geometry (1); geometry performs overlap checks"},
        {"events", "-e", "10", "Number of primary events"},
        {"output", "-o", "results/run",
         "New output directory; existing paths are never overwritten"},
        {"seed", "-seed", "20261029", "Positive random seed, at most 2147483646"},
        {"particle", "-x", "gamma", "Primary: gamma, e-, or e+"},
        {"energy-kev", "-E", "511", "Primary kinetic energy [keV], including for positrons"},
        {"theta", "-theta", "0", "Polar direction from +z [degrees, 0..180]"},
        {"phi", "-phi", "0", "Azimuth from +x toward +y [degrees]"},
        {"angular", "", "fixed", "fixed or isotropic directions"},
        {"source-x-mm", "-gx", "0", "Source x position [mm]"},
        {"source-y-mm", "-gy", "0", "Source y position [mm]"},
        {"source-z-mm", "-gz", "0", "Source z position [mm]"},
        {"shell-thickness-mm", "", "10", "LYSO shell thickness [mm]; outer radius remains 34.5 mm"},
        {"guide-shape", "", "pentagon-hexagon",
         "pentagon-hexagon (supplied) or pentagon-octagon (beveled variant)"},
        {"guide-bevel-fraction", "", "0.25",
         "Octagon corner cut as fraction of adjacent hexagon edges (0 < f < 0.5)"},
        {"sipm-pitch-mm", "", "4.4", "Sensor center pitch in each 4x4 array [mm]"},
        {"sipm-model", "", "HamamatsuS1036233100",
         "Built-in G4SiPM model name; package must fit array pitch"},
        {"lyso-yield", "", "32", "Scintillation photons per keV"},
        {"lyso-decay-ns", "", "41", "Scintillation decay time [ns]"},
        {"lyso-index", "", "1.81", "Constant LYSO refractive index"},
        {"lyso-absorption-mm", "", "413", "Constant LYSO optical absorption length [mm]"},
        {"scint-energy-ev", "", "2.8", "Center of provisional flat scintillation spectrum [eV]"},
        {"scint-width-ev", "", "0.02", "Full width of provisional flat spectrum [eV]"},
        {"spectrum", "", "", "Optional CSV: energy_eV,intensity; overrides flat spectrum"},
        {"epoxy-index", "", "1.5",
         "Shared epoxy index: guide, gaskets and reference sensor window"},
        {"epoxy-absorption-mm", "", "0",
         "Shared epoxy absorption [mm]; 0 leaves bulk absorption unset"},
        {"pde-filter", "", "1", "Enable upstream SiPM photon-detection-efficiency filter [0/1]"},
        {"cell-filter", "", "1", "Enable SiPM cell-geometry filter [0/1]"},
        {"range-cut-mm", "", "0.1", "Electromagnetic production range cut [mm]"},
        {"max-steps", "", "10000",
         "Per-track guard; killed tracks are reported and cause exit code 1"},
        {"track-photons", "", "0", "Save full steps for first N optical tracks/event (0 disables)"},
        {"export-geometry", "", "1", "Export geometry OBJ, inventory and sensor centers [0/1]"},
        {"mesh-steps", "", "96", "Angular tessellation steps for display only [12..192]"},
        {"check-overlaps", "", "0", "Check placed volumes before simulation [0/1]"},
        {"overlap-samples", "", "1000", "Sampled points per volume for overlap check"},
        {"overlap-tolerance-mm", "", "0.001", "Overlap-check tolerance [mm]"},
        {"macro", "-m", "", "Optional source macro; /gps/* commands only"},
        {"batch", "-b", "1", "1 batch; 0 opens Geant4 UI (requires WITH_GEANT4_UIVIS=ON)"},
        {"verbose", "-v", "0", "Application and SiPM verbosity [0..2]"}};
    return list;
}
Config::Config() {
    for (const auto& o : options())
        values[o.name] = o.initial;
}
const std::string& Config::text(const std::string& key) const { return values.at(key); }
double Config::number(const std::string& key) const {
    size_t n = 0;
    double value;
    try {
        value = std::stod(text(key), &n);
    } catch (...) {
        throw std::runtime_error("Invalid number for --" + key + ": " + text(key));
    }
    if (n != text(key).size() || !std::isfinite(value))
        throw std::runtime_error("Invalid finite number for --" + key);
    return value;
}
long Config::integer(const std::string& key) const {
    size_t n = 0;
    long value;
    try {
        value = std::stol(text(key), &n);
    } catch (...) {
        throw std::runtime_error("Invalid integer for --" + key + ": " + text(key));
    }
    if (n != text(key).size())
        throw std::runtime_error("Invalid integer for --" + key);
    return value;
}
bool Config::flag(const std::string& key) const {
    auto n = integer(key);
    if (n != 0 && n != 1)
        throw std::runtime_error("--" + key + " must be 0 or 1");
    return n == 1;
}
void Config::set(const std::string& key, const std::string& value) {
    if (!values.count(key))
        throw std::runtime_error("Unknown parameter: " + key);
    if (value.find_first_of("\r\n") != std::string::npos)
        throw std::runtime_error("Newlines are not allowed in parameter values");
    values[key] = value;
}
void Config::read(const std::string& filename) {
    std::ifstream f(filename);
    if (!f)
        throw std::runtime_error("Cannot open config: " + filename);
    std::string line;
    int n = 0;
    while (std::getline(f, line)) {
        ++n;
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        auto pos = line.find('=');
        if (pos == std::string::npos)
            throw std::runtime_error(filename + ":" + std::to_string(n) + ": expected key = value");
        auto key = trim(line.substr(0, pos)), value = trim(line.substr(pos + 1));
        // Input-file paths are relative to the config file. Output is relative to cwd.
        if ((key == "spectrum" || key == "macro") && !value.empty()) {
            auto p = std::filesystem::path(value);
            if (p.is_relative())
                value = (std::filesystem::absolute(filename).parent_path() / p)
                            .lexically_normal()
                            .string();
        }
        set(key, value);
    }
}
void Config::parse(int argc, char** argv) {
    // Read configs first: explicit command-line options always take precedence.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            helpRequested = true;
            return;
        }
        if (arg == "--version") {
            versionRequested = true;
            return;
        }
        if (arg == "--config" || arg == "-c") {
            if (++i == argc)
                throw std::runtime_error("Missing config filename");
            read(argv[i]);
        }
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" || arg == "-c") {
            ++i;
            continue;
        }
        if (arg == "--print-config") {
            printRequested = true;
            continue;
        }
        std::string key;
        for (const auto& o : options())
            if (arg == "--" + o.name || (!o.alias.empty() && arg == o.alias)) {
                key = o.name;
                break;
            }
        if (key.empty())
            throw std::runtime_error("Unknown option: " + arg + " (use --help)");
        if (++i == argc)
            throw std::runtime_error("Missing value for " + arg);
        set(key, argv[i]);
    }
    if (text("mode") == "0")
        set("mode", "simulation");
    if (text("mode") == "1")
        set("mode", "geometry");
    validate();
}
void Config::validate() const {
    if (text("mode") != "simulation" && text("mode") != "geometry")
        throw std::runtime_error("--mode must be simulation or geometry");
    if (integer("events") < 1 || integer("events") > 1000000000)
        throw std::runtime_error("--events must be 1..1000000000");
    if (integer("seed") < 1 || integer("seed") > 2147483646L)
        throw std::runtime_error("--seed must be 1..2147483646");
    if (text("particle") != "gamma" && text("particle") != "e-" && text("particle") != "e+")
        throw std::runtime_error("Supported primaries: gamma, e-, e+");
    if (text("guide-shape") != "pentagon-hexagon" && text("guide-shape") != "pentagon-octagon")
        throw std::runtime_error("--guide-shape must be pentagon-hexagon or pentagon-octagon");
    if (number("guide-bevel-fraction") <= 0 || number("guide-bevel-fraction") >= 0.5)
        throw std::runtime_error("--guide-bevel-fraction must be strictly between 0 and 0.5");
    if (text("angular") != "fixed" && text("angular") != "isotropic")
        throw std::runtime_error("--angular must be fixed or isotropic");
    for (const auto& key : {"energy-kev", "shell-thickness-mm", "sipm-pitch-mm", "lyso-decay-ns",
                            "lyso-absorption-mm", "scint-width-ev", "range-cut-mm"})
        if (number(key) <= 0)
            throw std::runtime_error(std::string("--") + key + " must be positive");
    if (number("shell-thickness-mm") >= 33.5)
        throw std::runtime_error(
            "Shell thickness must be below 33.5 mm to leave room for the 1 mm inner liner");
    if (number("lyso-yield") < 0 || number("epoxy-absorption-mm") < 0)
        throw std::runtime_error("Yield and epoxy absorption cannot be negative");
    if (number("lyso-index") < 1 || number("epoxy-index") < 1)
        throw std::runtime_error("Refractive indices must be at least 1");
    if (number("theta") < 0 || number("theta") > 180)
        throw std::runtime_error("--theta must be 0..180 degrees");
    number("phi");
    for (const auto& key : {"source-x-mm", "source-y-mm", "source-z-mm"})
        if (std::abs(number(key)) >= 99)
            throw std::runtime_error("Source must lie within the world (|coordinate| < 99 mm)");
    for (const auto& key :
         {"batch", "pde-filter", "cell-filter", "export-geometry", "check-overlaps"})
        flag(key);
    if (integer("max-steps") < 1 || integer("track-photons") < 0 ||
        integer("overlap-samples") < 1 || number("overlap-tolerance-mm") < 0)
        throw std::runtime_error("Invalid track or overlap limit");
    if (integer("mesh-steps") < 12 || integer("mesh-steps") > 192)
        throw std::runtime_error("--mesh-steps must be 12..192");
    if (integer("verbose") < 0 || integer("verbose") > 2)
        throw std::runtime_error("--verbose must be 0..2");
    if (text("output").empty())
        throw std::runtime_error("Output path cannot be empty");
    if (!text("macro").empty() && !std::filesystem::is_regular_file(text("macro")))
        throw std::runtime_error("Source macro does not exist: " + text("macro"));
    spectrum();
}
std::vector<std::pair<double, double>> Config::spectrum() const {
    std::vector<std::pair<double, double>> result;
    if (text("spectrum").empty()) {
        const double c = number("scint-energy-ev"), w = number("scint-width-ev");
        result = {{c - w / 2, 1}, {c, 1}, {c + w / 2, 1}};
    } else {
        std::ifstream f(text("spectrum"));
        if (!f)
            throw std::runtime_error("Cannot read spectrum: " + text("spectrum"));
        std::string line;
        while (std::getline(f, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#' || line == "energy_eV,intensity")
                continue;
            for (auto& c : line)
                if (c == ',')
                    c = ' ';
            std::istringstream in(line);
            double energy, intensity;
            std::string extra;
            if (!(in >> energy >> intensity) || (in >> extra))
                throw std::runtime_error(
                    "Spectrum requires two numeric columns: energy_eV,intensity");
            result.emplace_back(energy, intensity);
        }
    }
    if (result.size() < 2)
        throw std::runtime_error("Spectrum needs at least two points");
    double last = 0, sum = 0;
    for (auto [e, i] : result) {
        if (!std::isfinite(e) || !std::isfinite(i) || e <= last || e < 1 || e > 6 || i < 0)
            throw std::runtime_error("Spectrum energies must increase within 1..6 eV; intensities "
                                     "must be finite and nonnegative");
        last = e;
        sum += i;
    }
    if (sum <= 0)
        throw std::runtime_error("Spectrum has no positive intensity");
    return result;
}
void Config::write(std::ostream& out) const {
    out << "# Resolved spherical-pet parameters; input paths are absolute.\n";
    for (const auto& o : options()) {
        auto v = text(o.name);
        if ((o.name == "macro" || o.name == "spectrum") && !v.empty())
            v = std::filesystem::absolute(v).string();
        out << o.name << " = " << v << "\n";
    }
}
void Config::help(std::ostream& out) const {
    out << "spherical-pet 0.1.0 — Geant4 spherical PET optical simulation\n\n"
        << "Usage: spherical-pet [--config FILE] [OPTION VALUE ...]\n"
        << "Example: spherical-pet -r 0 -e 10 -x gamma -E 511 -theta 0 -seed 42 -o "
           "results/run42\n\n"
        << "  -h, --help            Show this help\n  --version             Show version\n"
        << "  -c, --config FILE     Read key=value config; CLI overrides config\n"
        << "  --print-config        Print resolved settings and exit without Geant4 "
           "initialization\n\n";
    for (const auto& o : options())
        out << "  " << std::left << std::setw(31)
            << ("--" + o.name + (o.alias.empty() ? "" : " (" + o.alias + ")")) << o.description
            << " [default: " << (o.initial.empty() ? "none" : o.initial) << "]\n";
    out << "\nFull-sphere geometry; 512 SiPMs; 12 pentagonal + 20 hexagonal guide "
           "sections.\nSingle gamma by default; no coincidence selection or PET image "
           "reconstruction.\n";
}
