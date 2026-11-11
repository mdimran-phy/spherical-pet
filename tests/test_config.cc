// SPDX-License-Identifier: GPL-3.0-only
#include "Config.hh"
#include <iostream>
#include <stdexcept>
#include <vector>

Config parse(std::initializer_list<std::string> args) {
    std::vector<std::string> strings = {"spherical-pet"};
    strings.insert(strings.end(), args.begin(), args.end());
    std::vector<char*> argv;
    for (auto& s : strings)
        argv.push_back(s.data());
    Config c;
    c.parse(argv.size(), argv.data());
    return c;
}
void require(bool condition, const std::string& message) {
    if (!condition)
        throw std::runtime_error(message);
}
void rejects(std::initializer_list<std::string> args) {
    try {
        parse(args);
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error("Invalid settings were accepted");
}
int main() {
    try {
        auto c = parse({"-r", "1", "-e", "3", "-E", "200", "-theta", "45", "--lyso-yield", "0"});
        require(c.text("mode") == "geometry", "Mode alias");
        require(c.integer("events") == 3 && c.number("energy-kev") == 200, "Short options");
        require(c.spectrum().size() == 3, "Default spectrum");
        require(parse({"--help"}).helpRequested, "Help");
        rejects({"-e", "1.5"});
        rejects({"--seed", "0"});
        rejects({"--lyso-index", "nan"});
        rejects({"--sipm-pitch-mm", "0"});
        rejects({"--shell-thickness-mm", "34.5"});
        rejects({"--theta", "181"});
        rejects({"--scint-energy-ev", "0.1"});
        rejects({"--pde-filter", "2"});
        rejects({"--unknown", "1"});
        rejects({"--events"});
        rejects({"--angular", "wrong"});
        rejects({"--vis-photons", "-1"});
        rejects({"--vis-photons", "1.5"});
        require(parse({"--vis-photons", "0"}).integer("vis-photons") == 0,
                "Can hide optical paths without disabling transport");
        require(parse({}).text("guide-shape") == "pentagon-hexagon", "Original shape default");
        require(parse({"--guide-shape", "pentagon-octagon"}).text("guide-shape") ==
                    "pentagon-octagon",
                "Octagon selector");
        rejects({"--guide-shape", "octagon"});
        rejects({"--guide-bevel-fraction", "0"});
        rejects({"--guide-bevel-fraction", "0.5"});
        rejects({"--guide-bevel-fraction", "nan"});
        std::cout << "PASS: standalone configuration tests\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
