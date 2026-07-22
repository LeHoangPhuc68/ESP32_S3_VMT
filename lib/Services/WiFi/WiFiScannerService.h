#pragma once

#include <Arduino.h>
#include <cstdint>

#include "WiFiScanSnapshot.h"

class WiFiScannerService final
{
public:
    using Network = WiFiScanEntry;

    static constexpr std::uint8_t MaxNetworks =
        WiFiScanSnapshot::Capacity;

    enum class State : std::uint8_t
    {
        Idle = 0,
        Preparing,
        Scanning,
        Complete,
        Failed
    };

    /*
     * Khởi tạo Wi-Fi radio ở Station mode.
     */
    static bool begin();

    /*
     * Bắt đầu scan bất đồng bộ.
     */
    static bool startScan();

    /*
     * Phải gọi liên tục trong loop.
     */
    static void update();

    static void cancelScan();

    /*
     * Dọn kết quả hiện tại.
     */
    static void clear();

    static State state();

    static bool isScanning();

    static std::uint8_t networkCount();

    static const Network *network(
        std::uint8_t index);

    static const WiFiScanSnapshot &snapshot();

    static const char *stateText();

    static const char *securityText(
        std::uint8_t encryptionType);

    /*
     * Mã trạng thái cuối cùng trả về từ WiFi API.
     *
     * Dùng để debug qua Serial.
     */
    static std::int16_t lastScanCode();

private:
    static constexpr std::int16_t InitializationError =
        -100;

    static constexpr std::int16_t OwnershipUnavailableError =
        -110;

    static constexpr std::int16_t OwnershipLostError =
        -111;

    static constexpr std::int16_t ScanRequestError =
        -112;

    static void failScan(
        std::int16_t errorCode);

    static void stageResults(
        std::int16_t resultCount);

    static void publishStagedResults();

    static void sortBySignalStrength(
        std::uint8_t count);

    static bool initialized_;

    static State state_;

    static WiFiScanSnapshot snapshot_;

    static Network stagedNetworks_[MaxNetworks];

    static std::uint8_t stagedNetworkCount_;

    static std::int16_t lastScanCode_;
};
