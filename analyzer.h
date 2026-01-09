#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstddef>

struct ZoneCount
{
    std::string zone;
    long long count;
};

struct SlotCount
{
    std::string zone;
    int hour;
    long long count;
};

class TripAnalyzer
{
public:
    void ingestFile(const std::string& csvPath);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;

private:
    struct SlotKeyHash
    {
        std::size_t operator()(const std::pair<std::string, int>& p) const noexcept;
    };

    struct SlotKeyEq
    {
        bool operator()(
            const std::pair<std::string, int>& a,
            const std::pair<std::string, int>& b
        ) const noexcept;
    };

    std::unordered_map<std::string, long long> zoneCounts_;
    std::unordered_map<
        std::pair<std::string, int>,
        long long,
        SlotKeyHash,
        SlotKeyEq
    > slotCounts_;
};
