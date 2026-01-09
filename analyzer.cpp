#include "analyzer.h"

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

static std::string trim_copy(const std::string& s) {
    size_t l = 0, r = s.size();
    while (l < r && std::isspace(static_cast<unsigned char>(s[l]))) ++l;
    while (r > l && std::isspace(static_cast<unsigned char>(s[r - 1]))) --r;
    return s.substr(l, r - l);
}

static void split_csv_line(const std::string& line, std::vector<std::string>& out) {
    out.clear();
    size_t i = 0;
    while (i <= line.size()) {
        size_t j = line.find(',', i);
        if (j == std::string::npos) j = line.size();
        out.push_back(trim_copy(line.substr(i, j - i)));
        i = j + 1;
        if (j == line.size()) break;
    }
}

static bool extract_hour(const std::string& ts, int& hour) {
    if (ts.size() < 13) return false;
    if (ts[10] != ' ') return false;
    char a = ts[11], b = ts[12];
    if (!std::isdigit(a) || !std::isdigit(b)) return false;
    hour = (a - '0') * 10 + (b - '0');
    if (hour < 0 || hour > 23) return false;
    return ts.find(':', 11) != std::string::npos;
}

size_t TripAnalyzer::SlotKeyHash::operator()(const std::pair<std::string, int>& k) const noexcept {
    size_t h = std::hash<std::string>{}(k.first);
    h = h * 31 + std::hash<int>{}(k.second);
    return h;
}

bool TripAnalyzer::SlotKeyEq::operator()(const std::pair<std::string, int>& x,
                                        const std::pair<std::string, int>& y) const noexcept {
    return x.first == y.first && x.second == y.second;
}

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    zoneCounts_.clear();
    slotCounts_.clear();

    std::ifstream file(csvPath);
    if (!file) return;

    std::string row;
    if (!std::getline(file, row)) return;

    std::vector<std::string> cols;

    while (std::getline(file, row)) {
        if (row.empty()) continue;
        split_csv_line(row, cols);
        if (cols.size() < 6) continue;

        const std::string& zone = cols[1];
        const std::string& time = cols[3];
        if (zone.empty() || time.empty()) continue;

        int hour;
        if (!extract_hour(time, hour)) continue;

        zoneCounts_[zone]++;
        slotCounts_[{zone, hour}]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> res;
    res.reserve(zoneCounts_.size());

    for (const auto& e : zoneCounts_)
        res.push_back({e.first, e.second});

    std::sort(res.begin(), res.end(),
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.zone < b.zone;
        });

    if (k < (int)res.size()) res.resize(k);
    return res;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> res;
    res.reserve(slotCounts_.size());

    for (const auto& e : slotCounts_)
        res.push_back({e.first.first, e.first.second, e.second});

    std::sort(res.begin(), res.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) return a.count > b.count;
            if (a.zone != b.zone) return a.zone < b.zone;
            return a.hour < b.hour;
        });

    if (k < (int)res.size()) res.resize(k);
    return res;
}
