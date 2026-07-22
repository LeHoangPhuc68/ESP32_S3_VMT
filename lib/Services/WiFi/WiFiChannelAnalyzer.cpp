#include "WiFiChannelAnalyzer.h"

namespace
{
    constexpr std::uint8_t MaximumOverlapDistance = 4;

    constexpr std::uint16_t PreferredScoreToleranceMinimum = 20;

    constexpr std::uint16_t PreferredScoreTolerancePercent = 15;

    constexpr std::uint8_t PreferredChannels[] =
    {
        1,
        6,
        11
    };
}

bool WiFiChannelAnalyzer::analyze(
    const WiFiScanSnapshot &snapshot)
{
    if (!snapshot.hasSuccessfulScan())
    {
        return false;
    }

    if (
        hasAnalysis_ &&
        generation_ == snapshot.generation())
    {
        return false;
    }

    std::int32_t rssiSums[ChannelCount] = {};

    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        channels_[index] = ChannelMetrics();
    }

    for (std::uint8_t entryIndex = 0;
         entryIndex < snapshot.count();
         ++entryIndex)
    {
        const WiFiScanEntry *entry =
            snapshot.entry(entryIndex);

        if (
            entry == nullptr ||
            entry->channel < FirstChannel ||
            entry->channel > LastChannel)
        {
            continue;
        }

        const std::uint8_t sourceChannel =
            static_cast<std::uint8_t>(
                entry->channel);

        ChannelMetrics &sourceMetrics =
            channels_[sourceChannel - FirstChannel];

        if (sourceMetrics.apCount < UINT8_MAX)
        {
            ++sourceMetrics.apCount;
        }

        std::int32_t storedRssi = entry->rssi;

        if (storedRssi < NoSignalRssi)
        {
            storedRssi = NoSignalRssi;
        }
        else if (storedRssi > 0)
        {
            storedRssi = 0;
        }

        if (storedRssi > sourceMetrics.strongestRssi)
        {
            sourceMetrics.strongestRssi =
                static_cast<std::int16_t>(storedRssi);
        }

        rssiSums[sourceChannel - FirstChannel] +=
            storedRssi;

        const std::uint16_t impact =
            signalImpact(entry->rssi);

        for (std::uint8_t candidate = FirstChannel;
             candidate <= LastChannel;
             ++candidate)
        {
            const std::uint8_t distance =
                candidate > sourceChannel
                    ? candidate - sourceChannel
                    : sourceChannel - candidate;

            const std::uint8_t weight =
                overlapWeight(distance);

            if (weight == 0)
            {
                continue;
            }

            ChannelMetrics &candidateMetrics =
                channels_[candidate - FirstChannel];

            candidateMetrics.interferenceScore +=
                static_cast<std::uint16_t>(
                    weight * impact);
        }
    }

    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        if (channels_[index].apCount > 0)
        {
            channels_[index].averageRssi =
                static_cast<std::int16_t>(
                    rssiSums[index] /
                    channels_[index].apCount);
        }
    }

    std::uint8_t bestChannel = FirstChannel;
    std::uint16_t bestScore =
        channels_[0].interferenceScore;

    for (std::uint8_t channel = FirstChannel + 1;
         channel <= LastChannel;
         ++channel)
    {
        const std::uint16_t score =
            channels_[channel - FirstChannel].
                interferenceScore;

        if (score < bestScore)
        {
            bestScore = score;
            bestChannel = channel;
        }
    }

    std::uint16_t tolerance =
        static_cast<std::uint16_t>(
            (static_cast<std::uint32_t>(bestScore) *
             PreferredScoreTolerancePercent) /
            100U);

    if (tolerance < PreferredScoreToleranceMinimum)
    {
        tolerance = PreferredScoreToleranceMinimum;
    }

    const std::uint32_t preferredLimit =
        static_cast<std::uint32_t>(bestScore) +
        tolerance;

    std::uint8_t preferredChannel = 0;
    std::uint16_t preferredScore = UINT16_MAX;

    for (const std::uint8_t channel : PreferredChannels)
    {
        const std::uint16_t score =
            channels_[channel - FirstChannel].
                interferenceScore;

        if (
            score <= preferredLimit &&
            score < preferredScore)
        {
            preferredChannel = channel;
            preferredScore = score;
        }
    }

    recommendedChannel_ =
        preferredChannel != 0
            ? preferredChannel
            : bestChannel;

    channels_[recommendedChannel_ - FirstChannel].
        recommended = true;

    generation_ = snapshot.generation();
    completedAtMs_ = snapshot.completedAtMs();
    hasAnalysis_ = true;

    return true;
}

void WiFiChannelAnalyzer::clear()
{
    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        channels_[index] = ChannelMetrics();
    }

    hasAnalysis_ = false;
    recommendedChannel_ = 0;
    generation_ = 0;
    completedAtMs_ = 0;
}

bool WiFiChannelAnalyzer::hasAnalysis() const
{
    return hasAnalysis_;
}

const WiFiChannelAnalyzer::ChannelMetrics *
WiFiChannelAnalyzer::metrics(
    const std::uint8_t channel) const
{
    if (
        !hasAnalysis_ ||
        channel < FirstChannel ||
        channel > LastChannel)
    {
        return nullptr;
    }

    return &channels_[channel - FirstChannel];
}

std::uint8_t
WiFiChannelAnalyzer::recommendedChannel() const
{
    return hasAnalysis_
        ? recommendedChannel_
        : 0;
}

std::uint32_t
WiFiChannelAnalyzer::generation() const
{
    return generation_;
}

std::uint32_t
WiFiChannelAnalyzer::completedAtMs() const
{
    return completedAtMs_;
}

std::uint8_t WiFiChannelAnalyzer::overlapWeight(
    const std::uint8_t distance)
{
    if (distance > MaximumOverlapDistance)
    {
        return 0;
    }

    return static_cast<std::uint8_t>(
        MaximumOverlapDistance + 1 - distance);
}

std::uint16_t WiFiChannelAnalyzer::signalImpact(
    std::int32_t rssi)
{
    if (rssi < -100)
    {
        rssi = -100;
    }
    else if (rssi > -30)
    {
        rssi = -30;
    }

    return static_cast<std::uint16_t>(
        10 + rssi + 100);
}
