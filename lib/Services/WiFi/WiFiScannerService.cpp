#include "WiFiScannerService.h"

#include <WiFi.h>

#include <esp_wifi.h>

bool WiFiScannerService::initialized_ =
    false;

WiFiScannerService::State
    WiFiScannerService::state_ =
        WiFiScannerService::State::Idle;

WiFiScannerService::Network
    WiFiScannerService::networks_[
        WiFiScannerService::MaxNetworks];

std::uint8_t
    WiFiScannerService::networkCount_ =
        0;

std::uint32_t
    WiFiScannerService::stateStartedAt_ =
        0;

std::uint32_t
    WiFiScannerService::firstFailureAt_ =
        0;

std::int16_t
    WiFiScannerService::lastScanCode_ =
        0;

bool WiFiScannerService::begin()
{
    if (initialized_)
    {
        return true;
    }

    Serial.println(
        "[WiFiScanner] Initializing Wi-Fi radio");

    /*
     * Tắt persistence để scanner không ghi cấu hình
     * Wi-Fi linh tinh vào flash.
     */
    WiFi.persistent(false);

    /*
     * Reset radio sạch sẽ.
     */
    WiFi.mode(WIFI_OFF);

    delay(100);

    if (!WiFi.mode(WIFI_STA))
    {
        Serial.println(
            "[WiFiScanner] Cannot enter WIFI_STA");

        state_ =
            State::Failed;

        lastScanCode_ =
            -100;

        return false;
    }

    /*
     * Tắt power saving để scan ổn định hơn.
     *
     * Không bắt buộc nhưng phù hợp với thiết bị
     * scanner cầm tay.
     */
    WiFi.setSleep(false);

    /*
     * Không xóa credentials đã lưu trong flash.
     * Chỉ ngắt kết nối hiện tại.
     */
    WiFi.disconnect(
        false,
        false);

    delay(150);

    WiFi.scanDelete();

    /*
     * Đặt storage trong RAM.
     *
     * Không cần lưu cấu hình Wi-Fi của scanner.
     */
    const esp_err_t storageResult =
        esp_wifi_set_storage(
            WIFI_STORAGE_RAM);

    if (
        storageResult != ESP_OK &&
        storageResult != ESP_ERR_WIFI_NOT_INIT)
    {
        Serial.printf(
            "[WiFiScanner] Storage warning: %d\n",
            static_cast<int>(
                storageResult));
    }

    networkCount_ =
        0;

    state_ =
        State::Idle;

    stateStartedAt_ =
        millis();

    firstFailureAt_ =
        0;

    lastScanCode_ =
        0;

    initialized_ =
        true;

    Serial.print(
        "[WiFiScanner] Ready, mode=");

    Serial.println(
        static_cast<int>(
            WiFi.getMode()));

    return true;
}

bool WiFiScannerService::startScan()
{
    if (!initialized_)
    {
        if (!begin())
        {
            return false;
        }
    }

    if (
        state_ == State::Preparing ||
        state_ == State::Scanning)
    {
        return false;
    }

    Serial.println(
        "[WiFiScanner] Scan requested");

    clear();

    /*
     * Chuyển qua Preparing trước thay vì gọi scan
     * ngay trong input callback.
     *
     * Điều này cho event loop và Wi-Fi task thời gian
     * hoàn tất reset radio.
     */
    if (!prepareRadio())
    {
        failScan(
            -101);

        return false;
    }

    state_ =
        State::Preparing;

    stateStartedAt_ =
        millis();

    firstFailureAt_ =
        0;

    return true;
}

void WiFiScannerService::update()
{
    const std::uint32_t now =
        millis();

    if (state_ == State::Preparing)
    {
        if (
            now - stateStartedAt_ <
            PrepareDelayMs)
        {
            return;
        }

        if (!launchAsyncScan())
        {
            failScan(
                lastScanCode_);

            return;
        }

        return;
    }

    if (state_ != State::Scanning)
    {
        return;
    }

    const std::int16_t result =
        WiFi.scanComplete();

    lastScanCode_ =
        result;

    if (result == WIFI_SCAN_RUNNING)
    {
        firstFailureAt_ =
            0;

        if (
            now - stateStartedAt_ >
            ScanTimeoutMs)
        {
            Serial.println(
                "[WiFiScanner] Scan timeout");

            esp_wifi_scan_stop();
            WiFi.scanDelete();

            failScan(
                -102);
        }

        return;
    }

    /*
     * Trên một số Arduino-ESP32 core,
     * scanComplete() có thể trả WIFI_SCAN_FAILED
     * trong thời gian ngắn trước khi event SCAN_DONE
     * được xử lý.
     *
     * Không kết luận fail ngay.
     */
    if (result == WIFI_SCAN_FAILED)
    {
        if (firstFailureAt_ == 0)
        {
            firstFailureAt_ =
                now;

            Serial.println(
                "[WiFiScanner] Temporary scan failed code, waiting");

            return;
        }

        if (
            now - firstFailureAt_ <
            FailureGraceMs)
        {
            return;
        }

        /*
         * Thử đọc lại một lần cuối sau grace period.
         */
        const std::int16_t finalResult =
            WiFi.scanComplete();

        lastScanCode_ =
            finalResult;

        if (finalResult >= 0)
        {
            finishScan(
                finalResult);

            return;
        }

        Serial.printf(
            "[WiFiScanner] Scan failed permanently: %d\n",
            static_cast<int>(
                finalResult));

        WiFi.scanDelete();

        failScan(
            finalResult);

        return;
    }

    if (result < 0)
    {
        /*
         * Giá trị âm không biết trước.
         *
         * Đợi đến timeout thay vì fail ngay để tránh
         * lỗi race tương tự.
         */
        if (
            now - stateStartedAt_ >
            ScanTimeoutMs)
        {
            Serial.printf(
                "[WiFiScanner] Unknown scan error: %d\n",
                static_cast<int>(
                    result));

            WiFi.scanDelete();

            failScan(
                result);
        }

        return;
    }

    finishScan(
        result);
}

void WiFiScannerService::clear()
{
    /*
     * Chỉ stop khi thực sự đang scan.
     */
    if (
        state_ == State::Preparing ||
        state_ == State::Scanning)
    {
        esp_wifi_scan_stop();
    }

    WiFi.scanDelete();

    for (std::uint8_t index = 0;
         index < MaxNetworks;
         ++index)
    {
        networks_[index].ssid =
            String();

        networks_[index].bssid =
            String();

        networks_[index].rssi =
            0;

        networks_[index].channel =
            0;

        networks_[index].encryptionType =
            0;

        networks_[index].hidden =
            false;
    }

    networkCount_ =
        0;

    state_ =
        State::Idle;

    stateStartedAt_ =
        millis();

    firstFailureAt_ =
        0;

    lastScanCode_ =
        0;
}

WiFiScannerService::State
WiFiScannerService::state()
{
    return state_;
}

bool WiFiScannerService::isScanning()
{
    return
        state_ == State::Preparing ||
        state_ == State::Scanning;
}

std::uint8_t
WiFiScannerService::networkCount()
{
    return networkCount_;
}

const WiFiScannerService::Network *
WiFiScannerService::network(
    const std::uint8_t index)
{
    if (index >= networkCount_)
    {
        return nullptr;
    }

    return &networks_[index];
}

const char *
WiFiScannerService::stateText()
{
    switch (state_)
    {
    case State::Idle:
        return "IDLE";

    case State::Preparing:
        return "PREPARING";

    case State::Scanning:
        return "SCANNING";

    case State::Complete:
        return "COMPLETE";

    case State::Failed:
        return "FAILED";

    default:
        return "UNKNOWN";
    }
}

const char *
WiFiScannerService::securityText(
    const std::uint8_t encryptionType)
{
    switch (encryptionType)
    {
    case WIFI_AUTH_OPEN:
        return "OPEN";

    case WIFI_AUTH_WEP:
        return "WEP";

    case WIFI_AUTH_WPA_PSK:
        return "WPA";

    case WIFI_AUTH_WPA2_PSK:
        return "WPA2";

    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA/WPA2";

    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2-E";

#if defined(WIFI_AUTH_WPA3_PSK)
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3";
#endif

#if defined(WIFI_AUTH_WPA2_WPA3_PSK)
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2/3";
#endif

#if defined(WIFI_AUTH_WAPI_PSK)
    case WIFI_AUTH_WAPI_PSK:
        return "WAPI";
#endif

    default:
        return "SECURED";
    }
}

std::int16_t
WiFiScannerService::lastScanCode()
{
    return lastScanCode_;
}

bool WiFiScannerService::prepareRadio()
{
    Serial.println(
        "[WiFiScanner] Preparing radio");

    /*
     * Nếu mode bị module khác thay đổi thì ép lại STA.
     */
    if (WiFi.getMode() != WIFI_STA)
    {
        WiFi.mode(WIFI_OFF);

        delay(80);

        if (!WiFi.mode(WIFI_STA))
        {
            Serial.println(
                "[WiFiScanner] Failed to restore STA mode");

            return false;
        }
    }

    WiFi.setSleep(false);

    /*
     * Ngắt association nhưng giữ radio bật.
     */
    WiFi.disconnect(
        false,
        false);

    /*
     * Hủy scan cũ nếu driver còn giữ trạng thái.
     */
    esp_wifi_scan_stop();

    WiFi.scanDelete();

    return true;
}

bool WiFiScannerService::launchAsyncScan()
{
    Serial.println(
        "[WiFiScanner] Starting async scan");

    firstFailureAt_ =
        0;

    /*
     * Dùng tham số mặc định của driver:
     *
     * async       = true
     * show_hidden = true
     *
     * Không ép max_ms_per_chan=300 nữa.
     * Default active scan của ESP-IDF được quản lý
     * bởi driver và thường ổn định hơn.
     */
    const std::int16_t result =
        WiFi.scanNetworks(
            true,
            true);

    lastScanCode_ =
        result;

    Serial.printf(
        "[WiFiScanner] scanNetworks returned %d\n",
        static_cast<int>(
            result));

    if (result == WIFI_SCAN_RUNNING)
    {
        state_ =
            State::Scanning;

        stateStartedAt_ =
            millis();

        return true;
    }

    /*
     * Một số core có thể trả kết quả ngay.
     */
    if (result >= 0)
    {
        finishScan(
            result);

        return true;
    }

    /*
     * Recovery giống hướng xử lý được đề xuất cho
     * WIFI_SCAN_FAILED: disconnect rồi scan lại.
     */
    if (result == WIFI_SCAN_FAILED)
    {
        Serial.println(
            "[WiFiScanner] First launch failed, resetting STA");

        WiFi.disconnect(
            false,
            false);

        delay(120);

        WiFi.mode(WIFI_OFF);

        delay(120);

        if (!WiFi.mode(WIFI_STA))
        {
            lastScanCode_ =
                -103;

            return false;
        }

        WiFi.setSleep(false);

        delay(200);

        const std::int16_t retryResult =
            WiFi.scanNetworks(
                true,
                true);

        lastScanCode_ =
            retryResult;

        Serial.printf(
            "[WiFiScanner] Retry returned %d\n",
            static_cast<int>(
                retryResult));

        if (retryResult == WIFI_SCAN_RUNNING)
        {
            state_ =
                State::Scanning;

            stateStartedAt_ =
                millis();

            firstFailureAt_ =
                0;

            return true;
        }

        if (retryResult >= 0)
        {
            finishScan(
                retryResult);

            return true;
        }
    }

    return false;
}

void WiFiScannerService::finishScan(
    const std::int16_t resultCount)
{
    Serial.printf(
        "[WiFiScanner] Scan complete: %d APs\n",
        static_cast<int>(
            resultCount));

    collectResults(
        resultCount);

    /*
     * Sau khi đã copy SSID/BSSID/RSSI/channel,
     * giải phóng danh sách nội bộ của WiFi library.
     */
    WiFi.scanDelete();

    state_ =
        State::Complete;

    stateStartedAt_ =
        millis();

    firstFailureAt_ =
        0;

    lastScanCode_ =
        resultCount;
}

void WiFiScannerService::failScan(
    const std::int16_t errorCode)
{
    networkCount_ =
        0;

    state_ =
        State::Failed;

    stateStartedAt_ =
        millis();

    firstFailureAt_ =
        0;

    lastScanCode_ =
        errorCode;

    Serial.printf(
        "[WiFiScanner] FAILED, code=%d mode=%d status=%d heap=%u\n",
        static_cast<int>(
            errorCode),
        static_cast<int>(
            WiFi.getMode()),
        static_cast<int>(
            WiFi.status()),
        static_cast<unsigned int>(
            ESP.getFreeHeap()));
}

void WiFiScannerService::collectResults(
    const std::int16_t resultCount)
{
    networkCount_ =
        0;

    if (resultCount <= 0)
    {
        return;
    }

    const std::int16_t copyCount =
        resultCount >
                static_cast<std::int16_t>(
                    MaxNetworks)
            ? static_cast<std::int16_t>(
                  MaxNetworks)
            : resultCount;

    for (std::int16_t index = 0;
         index < copyCount;
         ++index)
    {
        Network &network =
            networks_[networkCount_];

        network.ssid =
            WiFi.SSID(index);

        network.hidden =
            network.ssid.length() == 0;

        if (network.hidden)
        {
            network.ssid =
                "<hidden>";
        }

        network.bssid =
            WiFi.BSSIDstr(index);

        network.rssi =
            WiFi.RSSI(index);

        network.channel =
            WiFi.channel(index);

        network.encryptionType =
            static_cast<std::uint8_t>(
                WiFi.encryptionType(index));

        ++networkCount_;
    }

    sortBySignalStrength();
}

void WiFiScannerService::sortBySignalStrength()
{
    if (networkCount_ < 2)
    {
        return;
    }

    for (std::uint8_t index = 1;
         index < networkCount_;
         ++index)
    {
        Network current =
            networks_[index];

        std::int16_t position =
            static_cast<std::int16_t>(
                index) - 1;

        while (
            position >= 0 &&
            networks_[position].rssi <
                current.rssi)
        {
            networks_[position + 1] =
                networks_[position];

            --position;
        }

        networks_[position + 1] =
            current;
    }
}