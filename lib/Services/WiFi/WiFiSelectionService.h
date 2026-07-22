#pragma once

#include "WiFiScanSnapshot.h"

class WiFiSelectionService final
{
public:
    /*
     * Sao chép AP được chọn từ kết quả scanner.
     *
     * Dùng bản sao để AP Detail không phụ thuộc trực tiếp
     * vào index của danh sách scan.
     */
    static bool select(
        const WiFiScanEntry &network);

    static bool hasSelection();

    static const WiFiScanEntry *selected();

    static void clear();

    /*
     * Chuyển RSSI sang phần trăm chất lượng tín hiệu.
     *
     * -100 dBm hoặc yếu hơn = 0%
     * -50 dBm hoặc mạnh hơn = 100%
     */
    static std::uint8_t signalQuality();

    static std::uint16_t frequencyMHz();

private:
    static bool hasSelection_;

    static WiFiScanEntry selected_;
};
