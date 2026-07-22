#include "WiFiManager.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>

namespace
{
    portMUX_TYPE callbackMux =
        portMUX_INITIALIZER_UNLOCKED;

    WiFiManager::PromiscuousCallback activeCallback =
        nullptr;

    void *activeCallbackContext =
        nullptr;

    bool callbackDispatchEnabled =
        false;

    std::uint32_t callbacksInFlight =
        0;

    constexpr std::uint32_t CallbackDrainTimeoutMs =
        25;

    void setCallbackDispatch(
        const bool enabled,
        const WiFiManager::PromiscuousCallback callback,
        void *context)
    {
        portENTER_CRITICAL(&callbackMux);
        callbackDispatchEnabled = enabled;
        activeCallback = callback;
        activeCallbackContext = context;
        portEXIT_CRITICAL(&callbackMux);
    }

    WiFiManager::FrameType mapFrameType(
        const wifi_promiscuous_pkt_type_t type)
    {
        switch (type)
        {
        case WIFI_PKT_MGMT:
            return WiFiManager::FrameType::Management;

        case WIFI_PKT_CTRL:
            return WiFiManager::FrameType::Control;

        case WIFI_PKT_DATA:
            return WiFiManager::FrameType::Data;

        case WIFI_PKT_MISC:
        default:
            return WiFiManager::FrameType::Misc;
        }
    }

    void promiscuousReceiveThunk(
        void *buffer,
        const wifi_promiscuous_pkt_type_t type)
    {
        WiFiManager::PromiscuousCallback callback =
            nullptr;

        void *context = nullptr;

        portENTER_CRITICAL(&callbackMux);

        if (callbackDispatchEnabled)
        {
            callback = activeCallback;
            context = activeCallbackContext;

            if (callback != nullptr)
            {
                ++callbacksInFlight;
            }
        }

        portEXIT_CRITICAL(&callbackMux);

        if (callback == nullptr)
        {
            return;
        }

        WiFiManager::FrameView frame;
        frame.type = mapFrameType(type);

        if (
            buffer != nullptr &&
            frame.type == WiFiManager::FrameType::Misc)
        {
            const wifi_pkt_rx_ctrl_t *metadata =
                static_cast<const wifi_pkt_rx_ctrl_t *>(
                    buffer);

            frame.rssi = static_cast<std::int8_t>(
                metadata->rssi);

            frame.channel = static_cast<std::uint8_t>(
                metadata->channel);

            frame.rxState = static_cast<std::uint8_t>(
                metadata->rx_state);
        }
        else if (buffer != nullptr)
        {
            const wifi_promiscuous_pkt_t *packet =
                static_cast<const wifi_promiscuous_pkt_t *>(
                    buffer);

            frame.rssi = static_cast<std::int8_t>(
                packet->rx_ctrl.rssi);

            frame.channel = static_cast<std::uint8_t>(
                packet->rx_ctrl.channel);

            frame.rxState = static_cast<std::uint8_t>(
                packet->rx_ctrl.rx_state);

            frame.payload = packet->payload;
            frame.length = static_cast<std::uint16_t>(
                packet->rx_ctrl.sig_len);
        }

        callback(context, frame);

        portENTER_CRITICAL(&callbackMux);
        --callbacksInFlight;
        portEXIT_CRITICAL(&callbackMux);
    }

    bool waitForCallbackQuiescence()
    {
        const std::uint32_t startedAt = millis();

        while (true)
        {
            portENTER_CRITICAL(&callbackMux);
            const bool quiescent =
                callbacksInFlight == 0;
            portEXIT_CRITICAL(&callbackMux);

            if (quiescent)
            {
                return true;
            }

            if (
                millis() - startedAt >=
                CallbackDrainTimeoutMs)
            {
                return false;
            }

            delay(1);
        }
    }

    bool isSafePromiscuousStopResult(
        const esp_err_t result)
    {
        return
            result == ESP_OK ||
            result == ESP_ERR_WIFI_NOT_INIT ||
            result == ESP_ERR_WIFI_NOT_STARTED;
    }
}

bool WiFiManager::initialized_ = false;
WiFiManager::Owner WiFiManager::owner_ = WiFiManager::Owner::None;
WiFiManager::State WiFiManager::state_ = WiFiManager::State::Uninitialized;
std::uint8_t WiFiManager::requestedChannel_ = 0;
bool WiFiManager::showHidden_ = true;
std::uint16_t WiFiManager::passiveDwellMs_ = 300;
std::uint32_t WiFiManager::stateStartedAt_ = 0;
std::uint32_t WiFiManager::firstFailureAt_ = 0;
bool WiFiManager::recoveryAttempted_ = false;
bool WiFiManager::recoveryPending_ = false;
std::int16_t WiFiManager::lastScanCode_ = 0;
std::int16_t WiFiManager::lastRadioCode_ = 0;
std::int16_t WiFiManager::resultCount_ = 0;
bool WiFiManager::promiscuousActive_ = false;
bool WiFiManager::promiscuousConfigured_ = false;

bool WiFiManager::begin()
{
    if (initialized_)
    {
        return true;
    }

    Serial.println("[WiFiManager] Initializing radio");

    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    delay(100);

    if (!WiFi.mode(WIFI_STA))
    {
        state_ = State::Failed;
        lastScanCode_ = -100;
        return false;
    }

    WiFi.setSleep(false);
    WiFi.disconnect(false, false);
    delay(120);

    clearDriverResults();

    const esp_err_t storageResult =
        esp_wifi_set_storage(WIFI_STORAGE_RAM);

    if (
        storageResult != ESP_OK &&
        storageResult != ESP_ERR_WIFI_NOT_INIT)
    {
        Serial.printf(
            "[WiFiManager] Storage warning: %d\n",
            static_cast<int>(storageResult));
    }

    initialized_ = true;
    owner_ = Owner::None;
    state_ = State::Idle;
    stateStartedAt_ = millis();
    firstFailureAt_ = 0;
    recoveryAttempted_ = false;
    recoveryPending_ = false;
    lastScanCode_ = 0;
    lastRadioCode_ = 0;
    resultCount_ = 0;
    promiscuousActive_ = false;
    promiscuousConfigured_ = false;

    setCallbackDispatch(false, nullptr, nullptr);

    Serial.println("[WiFiManager] Ready");
    return true;
}

void WiFiManager::update()
{
    if (!initialized_)
    {
        return;
    }

    const std::uint32_t now = millis();

    if (state_ == State::Preparing)
    {
        if (recoveryPending_)
        {
            if (now - stateStartedAt_ < PrepareDelayMs)
            {
                return;
            }

            if (!WiFi.mode(WIFI_STA) || !prepareRadio())
            {
                recoveryPending_ = false;
                fail(-103);
                return;
            }

            recoveryPending_ = false;
            stateStartedAt_ = now;
            Serial.println("[WiFiManager] Radio recovery complete");
            return;
        }

        if (now - stateStartedAt_ < PrepareDelayMs)
        {
            return;
        }

        if (launchScan())
        {
            return;
        }

        if (!recoveryAttempted_)
        {
            recoveryAttempted_ = true;

            if (!startRadioRecovery())
            {
                fail(-103);
                return;
            }

            return;
        }

        fail(lastScanCode_);
        return;
    }

    if (state_ != State::Scanning)
    {
        return;
    }

    const std::int16_t result = WiFi.scanComplete();
    lastScanCode_ = result;

    if (result == WIFI_SCAN_RUNNING)
    {
        firstFailureAt_ = 0;

        if (now - stateStartedAt_ > ScanTimeoutMs)
        {
            stopDriverScan("timeout");
            clearDriverResults();
            fail(-102);
        }

        return;
    }

    if (result == WIFI_SCAN_FAILED)
    {
        if (firstFailureAt_ == 0)
        {
            firstFailureAt_ = now;
            return;
        }

        if (now - firstFailureAt_ < FailureGraceMs)
        {
            return;
        }

        const std::int16_t finalResult = WiFi.scanComplete();
        lastScanCode_ = finalResult;

        if (finalResult >= 0)
        {
            finish(finalResult);
        }
        else
        {
            clearDriverResults();
            fail(finalResult);
        }

        return;
    }

    if (result < 0)
    {
        if (now - stateStartedAt_ > ScanTimeoutMs)
        {
            stopDriverScan("unknown timeout");
            clearDriverResults();
            fail(result);
        }

        return;
    }

    finish(result);
}

bool WiFiManager::acquire(const Owner owner)
{
    if (owner == Owner::None)
    {
        Serial.println("[WiFiManager] Ownership rejected: invalid owner");
        return false;
    }

    if (!initialized_ && !begin())
    {
        Serial.printf(
            "[WiFiManager] Ownership rejected: initialization failed owner=%u\n",
            static_cast<unsigned int>(owner));
        return false;
    }

    if (owner_ == owner)
    {
        return true;
    }

    if (owner_ != Owner::None)
    {
        Serial.printf(
            "[WiFiManager] Ownership rejected: owner=%u busyBy=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    owner_ = owner;

    Serial.printf(
        "[WiFiManager] Ownership acquired: owner=%u\n",
        static_cast<unsigned int>(owner_));
    return true;
}

bool WiFiManager::requestScan(
    const Owner owner,
    const std::uint8_t channel,
    const bool showHidden,
    const std::uint16_t passiveDwellMs)
{
    if (owner == Owner::None)
    {
        return false;
    }

    if (!initialized_)
    {
        Serial.println("[WiFiManager] Scan rejected: manager not initialized");
        return false;
    }

    if (owner_ != owner)
    {
        Serial.printf(
            "[WiFiManager] Scan rejected: owner=%u currentOwner=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    if (state_ != State::Idle)
    {
        Serial.printf(
            "[WiFiManager] Scan rejected: owner=%u state=%s\n",
            static_cast<unsigned int>(owner),
            stateText());
        return false;
    }

    clearDriverResults();

    owner_ = owner;
    requestedChannel_ = channel <= 14 ? channel : 0;
    showHidden_ = showHidden;
    passiveDwellMs_ = passiveDwellMs;
    resultCount_ = 0;
    lastScanCode_ = 0;
    firstFailureAt_ = 0;
    recoveryAttempted_ = false;
    recoveryPending_ = false;

    if (!prepareRadio())
    {
        fail(-101);
        release(owner);
        return false;
    }

    state_ = State::Preparing;
    stateStartedAt_ = millis();

    Serial.printf(
        "[WiFiManager] Scan requested owner=%u channel=%u\n",
        static_cast<unsigned int>(owner_),
        static_cast<unsigned int>(requestedChannel_));

    return true;
}

bool WiFiManager::startPromiscuous(
    const Owner owner,
    const std::uint8_t channel,
    const PromiscuousCallback callback,
    void *context)
{
    if (
        owner == Owner::None ||
        channel < 1 ||
        channel > 13 ||
        callback == nullptr)
    {
        lastRadioCode_ = -201;
        Serial.println(
            "[WiFiManager] Promiscuous start rejected: invalid arguments");
        return false;
    }

    if (!initialized_)
    {
        lastRadioCode_ = -202;
        Serial.println(
            "[WiFiManager] Promiscuous start rejected: manager not initialized");
        return false;
    }

    if (owner_ != owner)
    {
        lastRadioCode_ = -203;
        Serial.printf(
            "[WiFiManager] Promiscuous start rejected: owner=%u currentOwner=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    if (state_ != State::Idle)
    {
        lastRadioCode_ = -204;
        Serial.printf(
            "[WiFiManager] Promiscuous start rejected: state=%s\n",
            stateText());
        return false;
    }

    lastRadioCode_ = 0;
    setCallbackDispatch(false, callback, context);

    if (!prepareRadio())
    {
        lastRadioCode_ = -205;
        setCallbackDispatch(false, nullptr, nullptr);
        return false;
    }

    const wifi_promiscuous_filter_t filter = {
        static_cast<std::uint32_t>(
            WIFI_PROMIS_FILTER_MASK_MGMT |
            WIFI_PROMIS_FILTER_MASK_CTRL |
            WIFI_PROMIS_FILTER_MASK_DATA)};

    esp_err_t result =
        esp_wifi_set_promiscuous_filter(&filter);

    if (result != ESP_OK)
    {
        lastRadioCode_ = static_cast<std::int16_t>(result);
        Serial.printf(
            "[WiFiManager] Promiscuous filter failed: code=%d\n",
            static_cast<int>(result));
        setCallbackDispatch(false, nullptr, nullptr);
        return false;
    }

    const wifi_promiscuous_filter_t controlFilter = {
        static_cast<std::uint32_t>(
            WIFI_PROMIS_CTRL_FILTER_MASK_ALL)};

    result = esp_wifi_set_promiscuous_ctrl_filter(
        &controlFilter);

    if (result != ESP_OK)
    {
        lastRadioCode_ = static_cast<std::int16_t>(result);
        Serial.printf(
            "[WiFiManager] Promiscuous control filter failed: code=%d\n",
            static_cast<int>(result));
        setCallbackDispatch(false, nullptr, nullptr);
        return false;
    }

    result = esp_wifi_set_channel(
        channel,
        WIFI_SECOND_CHAN_NONE);

    if (result != ESP_OK)
    {
        lastRadioCode_ = static_cast<std::int16_t>(result);
        Serial.printf(
            "[WiFiManager] Channel selection failed: channel=%u code=%d\n",
            static_cast<unsigned int>(channel),
            static_cast<int>(result));
        setCallbackDispatch(false, nullptr, nullptr);
        return false;
    }

    result = esp_wifi_set_promiscuous_rx_cb(
        &promiscuousReceiveThunk);

    if (result != ESP_OK)
    {
        lastRadioCode_ = static_cast<std::int16_t>(result);
        Serial.printf(
            "[WiFiManager] Promiscuous callback install failed: code=%d\n",
            static_cast<int>(result));
        setCallbackDispatch(false, nullptr, nullptr);
        return false;
    }

    promiscuousConfigured_ = true;

    result = esp_wifi_set_promiscuous(true);

    if (result != ESP_OK)
    {
        const std::int16_t enableError =
            static_cast<std::int16_t>(result);

        lastRadioCode_ = enableError;
        Serial.printf(
            "[WiFiManager] Promiscuous enable failed: code=%d\n",
            static_cast<int>(result));
        cleanupPromiscuous();
        lastRadioCode_ = enableError;
        return false;
    }

    promiscuousActive_ = true;
    state_ = State::Monitoring;
    stateStartedAt_ = millis();
    setCallbackDispatch(true, callback, context);

    Serial.printf(
        "[WiFiManager] Promiscuous monitoring started: owner=%u channel=%u\n",
        static_cast<unsigned int>(owner_),
        static_cast<unsigned int>(channel));

    return true;
}

bool WiFiManager::stopPromiscuous(
    const Owner owner)
{
    if (owner == Owner::None)
    {
        return false;
    }

    if (owner_ == Owner::None)
    {
        return true;
    }

    if (owner_ != owner)
    {
        lastRadioCode_ = -206;
        Serial.printf(
            "[WiFiManager] Promiscuous stop rejected: owner=%u currentOwner=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    if (
        !promiscuousConfigured_ &&
        !promiscuousActive_ &&
        state_ != State::Monitoring)
    {
        return true;
    }

    return cleanupPromiscuous();
}


bool WiFiManager::cancel(const Owner owner)
{
    if (owner == Owner::None)
    {
        return false;
    }

    if (owner_ == Owner::None)
    {
        return true;
    }

    if (owner_ != owner)
    {
        Serial.printf(
            "[WiFiManager] Cancel rejected: owner=%u currentOwner=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    if (
        state_ == State::Monitoring ||
        promiscuousConfigured_ ||
        promiscuousActive_)
    {
        if (!stopPromiscuous(owner))
        {
            return false;
        }
    }

    const bool operationActive =
        state_ == State::Preparing ||
        state_ == State::Scanning;

    if (operationActive)
    {
        stopDriverScan("cancel");
    }

    clearDriverResults();
    state_ = initialized_
        ? State::Idle
        : State::Uninitialized;
    resultCount_ = 0;
    firstFailureAt_ = 0;
    recoveryAttempted_ = false;
    recoveryPending_ = false;
    stateStartedAt_ = millis();

    if (operationActive)
    {
        Serial.printf(
            "[WiFiManager] Operation cancelled: owner=%u\n",
            static_cast<unsigned int>(owner_));
    }

    return true;
}

bool WiFiManager::release(const Owner owner)
{
    if (owner == Owner::None)
    {
        return false;
    }

    if (owner_ == Owner::None)
    {
        return true;
    }

    if (owner_ != owner)
    {
        Serial.printf(
            "[WiFiManager] Release rejected: owner=%u currentOwner=%u\n",
            static_cast<unsigned int>(owner),
            static_cast<unsigned int>(owner_));
        return false;
    }

    if (!cancel(owner))
    {
        return false;
    }

    owner_ = Owner::None;
    Serial.printf(
        "[WiFiManager] Ownership released: owner=%u\n",
        static_cast<unsigned int>(owner));
    return true;
}

void WiFiManager::reset()
{
    setCallbackDispatch(false, nullptr, nullptr);

    if (
        state_ == State::Monitoring ||
        promiscuousConfigured_ ||
        promiscuousActive_)
    {
        if (!cleanupPromiscuous())
        {
            const bool callbackQuiescent =
                waitForCallbackQuiescence();

            const bool poweredOff =
                callbackQuiescent &&
                WiFi.mode(WIFI_OFF);

            if (!poweredOff || !callbackQuiescent)
            {
                state_ = State::Failed;
                lastRadioCode_ = !callbackQuiescent
                    ? -208
                    : -209;
                Serial.printf(
                    "[WiFiManager] Radio reset failed: code=%d\n",
                    static_cast<int>(lastRadioCode_));
                return;
            }

            initialized_ = false;
            promiscuousActive_ = false;
            promiscuousConfigured_ = false;
        }
    }

    if (state_ == State::Scanning)
    {
        stopDriverScan("reset");
    }

    if (initialized_)
    {
        clearDriverResults();

        if (!WiFi.mode(WIFI_OFF))
        {
            state_ = State::Failed;
            lastRadioCode_ = -209;
            Serial.println(
                "[WiFiManager] Radio power-off reset failed");
            return;
        }
    }

    if (!waitForCallbackQuiescence())
    {
        state_ = State::Failed;
        lastRadioCode_ = -208;
        Serial.println(
            "[WiFiManager] Callback drain failed during reset");
        return;
    }

    delay(80);

    initialized_ = false;
    owner_ = Owner::None;
    state_ = State::Uninitialized;
    resultCount_ = 0;
    lastScanCode_ = 0;
    lastRadioCode_ = 0;
    recoveryAttempted_ = false;
    recoveryPending_ = false;
    promiscuousActive_ = false;
    promiscuousConfigured_ = false;
}

bool WiFiManager::isInitialized()
{
    return initialized_;
}

bool WiFiManager::isBusy()
{
    return owner_ != Owner::None;
}

bool WiFiManager::isOwnedBy(const Owner owner)
{
    return owner_ == owner;
}

WiFiManager::Owner WiFiManager::owner()
{
    return owner_;
}

WiFiManager::State WiFiManager::state()
{
    return state_;
}

const char *WiFiManager::stateText()
{
    switch (state_)
    {
    case State::Uninitialized:
        return "UNINITIALIZED";
    case State::Idle:
        return "IDLE";
    case State::Preparing:
        return "PREPARING";
    case State::Scanning:
        return "SCANNING";
    case State::Monitoring:
        return "MONITORING";
    case State::Ready:
        return "READY";
    case State::Failed:
        return "FAILED";
    default:
        return "UNKNOWN";
    }
}

std::int16_t WiFiManager::lastScanCode()
{
    return lastScanCode_;
}

std::int16_t WiFiManager::lastRadioCode()
{
    return lastRadioCode_;
}

std::int16_t WiFiManager::resultCount()
{
    return state_ == State::Ready ? resultCount_ : 0;
}

String WiFiManager::ssid(const std::int16_t index)
{
    return index >= 0 && index < resultCount_ ? WiFi.SSID(index) : String();
}

String WiFiManager::bssid(const std::int16_t index)
{
    return index >= 0 && index < resultCount_ ? WiFi.BSSIDstr(index) : String();
}

std::int32_t WiFiManager::rssi(const std::int16_t index)
{
    return index >= 0 && index < resultCount_ ? WiFi.RSSI(index) : -127;
}

std::int32_t WiFiManager::channel(const std::int16_t index)
{
    return index >= 0 && index < resultCount_ ? WiFi.channel(index) : 0;
}

std::uint8_t WiFiManager::encryptionType(const std::int16_t index)
{
    return index >= 0 && index < resultCount_
        ? static_cast<std::uint8_t>(WiFi.encryptionType(index))
        : 0;
}

const char *WiFiManager::securityText(const std::uint8_t encryptionType)
{
    switch (static_cast<wifi_auth_mode_t>(encryptionType))
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
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2/3";
    case WIFI_AUTH_WAPI_PSK:
        return "WAPI";
    case WIFI_AUTH_WPA3_ENT_192:
        return "WPA3-E";
    default:
        return "SECURED";
    }
}

bool WiFiManager::prepareRadio()
{
    if (WiFi.getMode() != WIFI_STA)
    {
        if (!WiFi.mode(WIFI_STA))
        {
            Serial.println("[WiFiManager] Failed to restore STA mode");
            return false;
        }
    }

    WiFi.setSleep(false);
    WiFi.disconnect(false, false);
    stopDriverScan("prepare");
    clearDriverResults();
    return true;
}

bool WiFiManager::cleanupPromiscuous()
{
    setCallbackDispatch(false, nullptr, nullptr);

    const esp_err_t disableResult =
        esp_wifi_set_promiscuous(false);

    bool radioSafe =
        isSafePromiscuousStopResult(disableResult);

    bool radioReady = false;

    if (!radioSafe)
    {
        lastRadioCode_ = static_cast<std::int16_t>(
            disableResult);

        Serial.printf(
            "[WiFiManager] Promiscuous disable failed, recovering radio: code=%d\n",
            static_cast<int>(disableResult));
    }

    const bool callbackQuiescent =
        waitForCallbackQuiescence();

    if (!callbackQuiescent)
    {
        state_ = State::Failed;
        lastRadioCode_ = -208;
        Serial.println(
            "[WiFiManager] Promiscuous callback drain timed out");
        return false;
    }

    if (radioSafe)
    {
        radioReady = prepareRadio();
    }

    if (!radioReady)
    {
        const bool poweredOff =
            WiFi.mode(WIFI_OFF);

        radioSafe = radioSafe || poweredOff;

        if (poweredOff && WiFi.mode(WIFI_STA))
        {
            radioReady = prepareRadio();
        }
    }

    if (!radioSafe)
    {
        state_ = State::Failed;

        if (lastRadioCode_ == 0)
        {
            lastRadioCode_ = -207;
        }

        Serial.printf(
            "[WiFiManager] Promiscuous cleanup failed: code=%d\n",
            static_cast<int>(lastRadioCode_));
        return false;
    }

    promiscuousActive_ = false;
    promiscuousConfigured_ = false;
    resultCount_ = 0;
    firstFailureAt_ = 0;
    recoveryAttempted_ = false;
    recoveryPending_ = false;
    stateStartedAt_ = millis();

    if (radioReady)
    {
        state_ = State::Idle;
    }
    else
    {
        initialized_ = false;
        state_ = State::Uninitialized;
    }

    lastRadioCode_ = 0;

    Serial.printf(
        "[WiFiManager] Promiscuous monitoring stopped: owner=%u\n",
        static_cast<unsigned int>(owner_));

    return true;
}


bool WiFiManager::launchScan()
{
    const std::int16_t result = WiFi.scanNetworks(
        true,
        showHidden_,
        false,
        passiveDwellMs_,
        requestedChannel_);

    lastScanCode_ = result;

    if (result == WIFI_SCAN_RUNNING)
    {
        state_ = State::Scanning;
        stateStartedAt_ = millis();
        firstFailureAt_ = 0;
        return true;
    }

    if (result >= 0)
    {
        finish(result);
        return true;
    }

    return false;
}

bool WiFiManager::startRadioRecovery()
{
    stopDriverScan("recovery");
    clearDriverResults();
    WiFi.disconnect(false, false);

    if (!WiFi.mode(WIFI_OFF))
    {
        Serial.println("[WiFiManager] Failed to power down radio for recovery");
        return false;
    }

    recoveryPending_ = true;
    stateStartedAt_ = millis();
    Serial.println("[WiFiManager] Radio recovery started");
    return true;
}

void WiFiManager::stopDriverScan(const char *context)
{
    const esp_err_t result = esp_wifi_scan_stop();

    if (
        result == ESP_OK ||
        result == ESP_ERR_WIFI_NOT_INIT ||
        result == ESP_ERR_WIFI_NOT_STARTED ||
        result == ESP_ERR_WIFI_STATE)
    {
        return;
    }

    Serial.printf(
        "[WiFiManager] Scan stop warning: context=%s code=%d\n",
        context,
        static_cast<int>(result));
}

void WiFiManager::finish(const std::int16_t resultCount)
{
    resultCount_ = resultCount;
    lastScanCode_ = resultCount;
    state_ = State::Ready;
    stateStartedAt_ = millis();
    firstFailureAt_ = 0;
    recoveryPending_ = false;

    Serial.printf(
        "[WiFiManager] Scan complete owner=%u results=%d\n",
        static_cast<unsigned int>(owner_),
        static_cast<int>(resultCount_));
}

void WiFiManager::fail(const std::int16_t errorCode)
{
    lastScanCode_ = errorCode;
    resultCount_ = 0;
    state_ = State::Failed;
    stateStartedAt_ = millis();
    firstFailureAt_ = 0;
    recoveryPending_ = false;

    Serial.printf(
        "[WiFiManager] Scan failed owner=%u code=%d\n",
        static_cast<unsigned int>(owner_),
        static_cast<int>(errorCode));
}

void WiFiManager::clearDriverResults()
{
    WiFi.scanDelete();
}
