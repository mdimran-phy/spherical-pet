// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct Option {
    std::string name, alias, initial, description;
};
class Config {
  public:
    Config();
    static const std::vector<Option>& options();
    void parse(int argc, char** argv);
    void validate() const;
    void help(std::ostream& out) const;
    void write(std::ostream& out) const;
    const std::string& text(const std::string& key) const;
    double number(const std::string& key) const;
    long integer(const std::string& key) const;
    bool flag(const std::string& key) const;
    std::vector<std::pair<double, double>> spectrum() const;
    bool helpRequested = false, versionRequested = false, printRequested = false;

  private:
    std::map<std::string, std::string> values;
    void set(const std::string& key, const std::string& value);
    void read(const std::string& filename);
};
