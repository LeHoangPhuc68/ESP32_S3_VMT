#include "WiFiScanSnapshot.h"

WiFiScanSnapshot::Status
WiFiScanSnapshot::status() const
{
    return status_;
}

bool WiFiScanSnapshot::hasSuccessfulScan() const
{
    return status_ == Status::Success;
}

std::uint8_t WiFiScanSnapshot::count() const
{
    return count_;
}

std::uint32_t WiFiScanSnapshot::generation() const
{
    return generation_;
}

std::uint32_t WiFiScanSnapshot::completedAtMs() const
{
    return completedAtMs_;
}

const WiFiScanEntry *WiFiScanSnapshot::entry(
    const std::uint8_t index) const
{
    if (
        status_ != Status::Success ||
        index >= count_)
    {
        return nullptr;
    }

    return &entries_[index];
}

WiFiScanEntry &WiFiScanSnapshot::entryForPublication(
    const std::uint8_t index)
{
    return entries_[index];
}

void WiFiScanSnapshot::publish(
    const std::uint8_t count,
    const std::uint32_t completedAtMs)
{
    count_ = count <= Capacity ? count : Capacity;
    completedAtMs_ = completedAtMs;
    ++generation_;
    status_ = Status::Success;
}

void WiFiScanSnapshot::clear()
{
    for (std::uint8_t index = 0; index < Capacity; ++index)
    {
        entries_[index] = WiFiScanEntry();
    }

    status_ = Status::Empty;
    count_ = 0;
    completedAtMs_ = 0;
}
