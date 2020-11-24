#include <Arduino.h>
#include <EspWifiSetup.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "web/RootPage.h"
#include "web/SettingsPage.h"

const char PREFS_NAME[] = "settings";
const char ID_NAME[] = "name";
const char ID_ACTIVATE_REPORTING[] = "activateRep";
const char ID_EDIT_ADDRESS[] = "editAddress";
const char ID_INTERVAL_SECS[] = "intervalSecs";
const char ID_PASSIVE[] = "passive";
const char ID_REPORT_BATTERY[] = "activateRepBat";
const char ID_REPORT_BATTERY_ADDRESS[] = "editAddressBat";

const unsigned int TIMES_HALL_READ = 10;
const unsigned int DELAY_MS_HALL_READ = 100;
const unsigned int THRESHOLD_HALL = 30;
const int NUMBER_SAMPLES_BATTERY = 3;
const int DATA_PIN_THERMOMETER = 5;
const int PIN_BATTERY_MONITORING = 35;

AsyncWebServer *webServer = NULL;
OneWire oneWire(DATA_PIN_THERMOMETER);
DallasTemperature sensors(&oneWire);

float currentTemp = -127.0F;
float currentBatteryStatus = -1.0F;

String settingSensorName;
bool settingActivateReporting;
String settingBaseAddress;
unsigned int settingIntervalSecs;
bool settingPassive;
bool settingReportBattery;
String settingReportBatteryAddressSuffix;

unsigned long millisStart;

String getShortMac()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    String macStringLastPart = "";
    for (int i = 4; i < 6; i++)
    {
        macStringLastPart += String(mac[i], HEX);
    }

    return macStringLastPart;
}

void initiateDeepSleepForReporting()
{
    esp_sleep_enable_timer_wakeup(settingIntervalSecs * 1000 * 1000);
    esp_deep_sleep_start();
}

bool isTempValid(float temp)
{
    return (temp > -30.0F && temp < 60.0F);
}

void readSettings()
{
    Preferences prefs;
    prefs.begin(PREFS_NAME, true);

    settingSensorName = prefs.getString(ID_NAME, getShortMac());
    settingActivateReporting = prefs.getBool(ID_ACTIVATE_REPORTING, false);
    settingEditAddress = prefs.getString(ID_EDIT_ADDRESS, "");
    settingIntervalSecs = prefs.getUInt(ID_INTERVAL_SECS, 1800);
    settingPassive = prefs.getBool(ID_PASSIVE, false);
    settingReportBattery = prefs.getBool(ID_REPORT_BATTERY, false);
    settingReportBatteryAddress = prefs.getString(ID_REPORT_BATTERY_ADDRESS, "");

    prefs.end();
}

void saveSettings()
{
    Preferences prefs;
    prefs.begin(PREFS_NAME, false);

    prefs.putString(ID_NAME, settingSensorName);
    prefs.putBool(ID_ACTIVATE_REPORTING, settingActivateReporting);
    prefs.putString(ID_EDIT_ADDRESS, settingEditAddress);
    prefs.putUInt(ID_INTERVAL_SECS, settingIntervalSecs);
    prefs.putBool(ID_PASSIVE, settingPassive);
    prefs.putBool(ID_REPORT_BATTERY, settingReportBattery);
    prefs.putString(ID_REPORT_BATTERY_ADDRESS, settingReportBatteryAddress);

    prefs.end();
}

void resetSettings()
{
    Preferences prefs;
    prefs.begin(PREFS_NAME, false);
    prefs.clear();
    prefs.end();
}


void handleRootPage(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", rootPage);
}

void handleSettingsPage(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", settingsPage);
}

void handleGetTemperature(AsyncWebServerRequest *request)
{
    if (isTempValid(currentTemp))
        request->send(200, "text/plain", String(currentTemp));
    else
        request->send(500, "text/plain", "Temperature could not be determined!");
}

void handleGetSettings(AsyncWebServerRequest *request)
{
    DynamicJsonDocument doc(1024);

    doc[ID_NAME] = settingName;
    doc[ID_ACTIVATE_REPORTING] = settingActivateReporting;
    doc[ID_EDIT_ADDRESS] = settingEditAddress;
    doc[ID_INTERVAL_SECS] = settingIntervalSecs;
    doc[ID_PASSIVE] = settingPassive;
    doc[ID_REPORT_BATTERY] = settingReportBattery;
    doc[ID_REPORT_BATTERY_ADDRESS] = settingReportBatteryAddress;

    String json;
    serializeJson(doc, json);

    request->send(200, "application/json", json);
}

void handlePostSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    if (len != total)
    {
        request->send(400, "text/plain", "No settings found!");
    }
    else
    {
        DynamicJsonDocument doc(len);
        if (deserializeJson(doc, data) != DeserializationError::Ok)
        {
            request->send(400, "text/plain", "Could not parse JSON!");
        }
        else if (!doc.containsKey(ID_NAME))
            request->send(400, "text/plain", String(ID_NAME) + String(" missing!"));
        else if (!doc.containsKey(ID_ACTIVATE_REPORTING))
            request->send(400, "text/plain", String(ID_ACTIVATE_REPORTING) + String(" missing!"));
        else if (!doc.containsKey(ID_EDIT_ADDRESS))
            request->send(400, "text/plain", String(ID_EDIT_ADDRESS) + String(" missing!"));
        else if (!doc.containsKey(ID_INTERVAL_SECS))
            request->send(400, "text/plain", String(ID_INTERVAL_SECS) + String(" missing!"));
        else if (!doc.containsKey(ID_PASSIVE))
            request->send(400, "text/plain", String(ID_PASSIVE) + String(" missing!"));
        else if (!doc.containsKey(ID_REPORT_BATTERY))
            request->send(400, "text/plain", String(ID_REPORT_BATTERY) + String(" missing!"));
        else if (!doc.containsKey(ID_REPORT_BATTERY_ADDRESS))
            request->send(400, "text/plain", String(ID_REPORT_BATTERY_ADDRESS) + String(" missing!"));

        else
        {
            settingName = doc[ID_NAME].as<String>();
            settingActivateReporting = doc[ID_ACTIVATE_REPORTING];
            settingEditAddress = doc[ID_EDIT_ADDRESS].as<String>();
            settingIntervalSecs = doc[ID_INTERVAL_SECS];
            settingPassive = doc[ID_PASSIVE];
            settingReportBattery = doc[ID_REPORT_BATTERY];
            settingReportBatteryAddress = doc[ID_REPORT_BATTERY_ADDRESS].as<String>();

            saveSettings();
        }

        request->send(200, "text/plain", "OK!");

        delay(300);

        ESP.restart();
    }
}

void setupWebServer()
{
    webServer = new AsyncWebServer(80);

    webServer->onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found!"); });
    webServer->on("/", HTTP_GET, [&](AsyncWebServerRequest *request) { handleRootPage(request); });
    webServer->on("/settingsPage", HTTP_GET, [&](AsyncWebServerRequest *request) { handleSettingsPage(request); });
    webServer->on("/settings", HTTP_GET, [&](AsyncWebServerRequest *request) { handleGetSettings(request); });
    webServer->on("/settings", HTTP_POST,
                  [](AsyncWebServerRequest *request) -> void { request->send(400, "text/plain", "No data!"); },
                  [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) -> void { request->send(400, "text/plain", "Wrong data!"); },
                  [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) -> void { handlePostSettings(request, data, len, index, total); });
    webServer->on("/temperature", HTTP_GET, [&](AsyncWebServerRequest *request) { handleGetTemperature(request); });

    webServer->begin();
}

void updateTemperature()
{
    int tries = 0;
    bool successful = false;
    while (tries < 5 && !successful)
    {
        sensors.requestTemperatures();
        float temp = sensors.getTempCByIndex(0);

        if (!isTempValid(temp))
        {
            Serial.println(String("Could not read proper temperature on pin ") + String(DATA_PIN_THERMOMETER) + String("! Please check the wiring."));
            delay(100);
        }
        else
        {
            currentTemp = temp;
            Serial.println(String("Current temperature: ") + String(currentTemp) + String("°C"));
            successful = true;
        }
        tries++;
    }
}

void updateBatteryStatus()
{
    if (settingReportBattery)
    {
        int sumValues = 0;
        for (int i = 0; i < NUMBER_SAMPLES_BATTERY; i++)
        {
            sumValues += analogRead(PIN_BATTERY_MONITORING);
        }

        float value = (float)sumValues / (float)NUMBER_SAMPLES_BATTERY;
        float batteryVoltage = ((float)value / 2047.0F) * 4.2F;

        currentBatteryStatus = (batteryVoltage - (2.8F)) / (4.2F - 2.8F);

        Serial.println(String("Current battery level: ") + String(currentBatteryStatus * 100) + String("% (") + String(batteryVoltage) + String(" V) Raw: ") + String(value));
        Serial.println(value);
    }
}

const unsigned int UPDATE_INVERVAL_MS = 5000;
unsigned long lastUpdated = 0;
void updateValues()
{
    unsigned long now = millis();
    if (lastUpdated == 0 || lastUpdated + UPDATE_INVERVAL_MS < now || lastUpdated > now)
    {
        lastUpdated = millis();

        updateTemperature();
        updateBatteryStatus();
    }
}

void sendPutRequest(String address, String value)
{
    HTTPClient client;
    client.begin(address);
    client.addHeader("Content-Type", "text/plain");

    int resCode = client.PUT(value);

    if (resCode <= 0)
    {
        Serial.println(
            String("Could send PUT to address '") + address + String("'. The following error occured: '") +
            String(resCode) +
            String("' Please refer to the following address to get further information: ") +
            String("https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h"));
    }
    else if (resCode < 200 || resCode >= 300)
    {
        Serial.println(String("Server returned an error status code sending to '") + address + String("': '") + String(resCode) + String("'!"));
    }
}

void runThermometerReporting()
{
    if (settingEditAddress.length() == 0)
    {
        Serial.println("Invalid setting for address thermometer found!");
    }
    else if (!isTempValid(currentTemp))
    {
        Serial.println("Invalid temperature found! Cannot finish reporting successfully!");
    }
    else
    {
        sendPutRequest(settingEditAddress, String(currentTemp));
        Serial.println(String("Reporting temperature finished!"));
    }
}

void runBatteryReporting()
{
    if (settingReportBattery)
    {
        if (settingReportBatteryAddress.length() == 0)
        {
            Serial.println("Invalid setting for address battery found!");
        }
        else if (currentBatteryStatus < 0.0F)
        {
            Serial.println(String("Invalid battery status found! Cannot finish reporting successfully!"));
        }
        else
        {
            sendPutRequest(settingReportBatteryAddress, String(currentBatteryStatus));
            Serial.println(String("Reporting battery finished!"));
        }
    }
}

void runReporting()
{
    runThermometerReporting();
    runBatteryReporting();

    if (settingPassive)
    {
        long timeAwake = millis() - millisStart;
        Serial.println(String("Passive mode active and thus going to deep sleep. Time awake: ") + String(timeAwake) + String(" milliseconds"));
        initiateDeepSleepForReporting();
    }

    Serial.println(String("Reporting finished! Next update in '") + String(settingIntervalSecs) + String("' seconds!"));
}

unsigned long lastReportingChecked = 0;
void handleReporting()
{
    if (settingActivateReporting)
    {
        unsigned long now = millis();
        if (lastReportingChecked == 0 || lastReportingChecked + settingIntervalSecs * 1000 < now || lastReportingChecked > now)
        {
            Serial.println("Reporting executing...");
            lastReportingChecked = millis();
            runReporting();
        }
    }
}

void setup()
{
    Serial.begin(9600);
    analogSetAttenuation(ADC_11db);
    analogReadResolution(11);

    delay(10);
    Serial.println("RAW:");
    Serial.println(String(analogRead(PIN_BATTERY_MONITORING)));

    millisStart = millis();

    if (checkHallForReset())
    {
        Serial.println("Hall sensor threshold exceeded! Resetting settings...");
        EspWifiSetup::resetSettings();
        resetSettings();
    }

    Serial.println("Reading settings...");
    readSettings();
    Serial.println("Settings read!");

    Serial.println("Setting up wifi...");
    if (!EspWifiSetup::setup(String("Thermometer-") + settingSensorName, false, settingPassive) && settingPassive)
    {
        initiateDeepSleepForReporting();
    }
    Serial.println("WiFi successfully set up!");

    Serial.println("Setting up sensors");
    sensors.begin();
    Serial.println("Sensors set up!");

    if (!settingPassive)
    {
        Serial.println("Setting up web server...");
        setupWebServer();
        Serial.println("Webserver set up!");
    }
    else
    {
        Serial.println("Passive mode active");
    }
}

void loop()
{
    updateValues();
    handleReporting();
    delay(10);
}
