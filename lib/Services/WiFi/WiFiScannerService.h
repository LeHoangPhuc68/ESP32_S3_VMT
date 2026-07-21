#pragma once

#include <Arduino.h>
#include <cstdint>

class WiFiScannerService final
{
public:
    static constexpr std::uint8_t MaxNetworks = 16;

    enum class State : std::uint8_t
    {
        Idle = 0,
        Preparing,
        Scanning,
        Complete,
        Failed
    };

    struct Network
    {
        String ssid;
        String bssid;

        std::int32_t rssi = 0;
        std::int32_t channel = 0;

        std::uint8_t encryptionType = 0;

        bool hidden = false;
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

    static void collectResults(
        std::int16_t resultCount);

    static void sortBySignalStrength();

    static void clearNetworks();

    static bool initialized_;

    static State state_;

    static Network networks_[MaxNetworks];

    static std::uint8_t networkCount_;

    static std::int16_t lastScanCode_;
};
