/**
 * @file main.cpp
 * @author Pingoin (p.drente@gmx.de)
 * @brief 
 * @version 0.1
 * @date 25.08.2019
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "main.h"
#include "settings.h"

/**
 * @brief Startfunktion
 * 
 */
void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
    // Try to read WiFi settings from RTC memory
    bool rtcValid = false;
    if (ESP.rtcUserMemoryRead(0, (uint32_t *)&rtcData, sizeof(rtcData)))
    {
        // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
        uint32_t crc = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
        if (crc == rtcData.crc32)
        {
            rtcValid = true;
        }
    }

    client.disconnect(); //eventuell vorhandene Alte Verbindung löschen
    humidity = doc.createNestedObject("Luftfeuchte");
    pressure = doc.createNestedObject("Luftdruck");
    temperature = doc.createNestedObject("Temperatur");
    battery = doc.createNestedObject("Batterie");
    powerSupply = doc.createNestedObject("Power");
    otaStatus = doc.createNestedObject("OTA-Status");
    humidity["u"] = "%";
    pressure["u"] = "hPa";
    temperature["u"] = "°C";
    battery["u"] = "V";
    powerSupply["u"] = "V";
    otaStatus["u"] = "-";

    //Wifi definieren
    WiFi.forceSleepWake();
    delay(100);

    // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
#if defined(staticIP) && defined(staticGateway) && defined(staticSubnet) && defined(staticDNS)
    IPAddress ip(staticIP);
    IPAddress gateway(staticGateway);
    IPAddress subnet(staticSubnet);
    IPAddress dns(staticDNS);
    WiFi.config(ip, dns, gateway, subnet);
#endif
    if (rtcValid)
    {
        // The RTC data was good, make a quick connection
        WiFi.begin(SSID, PSK, rtcData.channel, rtcData.ap_mac, true);
    }
    else
    {
        // The RTC data was not valid, so make a regular connection
        WiFi.begin(SSID, PSK);
    }

    //Sensoren lesen
    Wire.begin();
    ads.setGain(FSR4096);

    if (bme.begin(bme280I2C))
    {
        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1, // temperature
                        Adafruit_BME280::SAMPLING_X1, // pressure
                        Adafruit_BME280::SAMPLING_X1, // humidity
                        Adafruit_BME280::FILTER_OFF);
        readSensors();
        sensorsConnected = true;
    }
    //Auf Wifi Warten
    int retries = 0;
    int wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED)
    {
        retries++;
        if (retries == 100)
        {
            // Quick connect is not working, reset WiFi and try regular connection
            WiFi.disconnect();
            delay(10);
            WiFi.forceSleepBegin();
            delay(10);
            WiFi.forceSleepWake();
            delay(10);
            WiFi.begin(SSID, PSK);
        }
        if (retries == 600)
        {
            // Giving up after 30 seconds and going back to sleep
            WiFi.disconnect(true);
            delay(1);
            WiFi.mode(WIFI_OFF);
            ESP.deepSleep(sleepTime * 1000, WAKE_RF_DISABLED);
            return; // Not expecting this to be called, the previous call will never return.
        }
        delay(50);
        wifiStatus = WiFi.status();
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    rtcData.channel = WiFi.channel();
    memcpy(rtcData.ap_mac, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
    rtcData.crc32 = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
    ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcData, sizeof(rtcData));
    otaEnabled = checkOTA(); //Überprüfung, ob wifi an bleiben soll

    Serial.println(msg);
    client.setServer(IPAddress(MQTT_BROKER), 1883); //MQTT-Server definieren
    client.setCallback(callback);                   //Funktion definieren, die aufgerufen wird, wenn eine abbonierte nachricht ankommt

    if (client.connect(hostName))
    {
        Serial.println("MQTT verbunden");
        //client.publish(publishTopic, msg);
        sendData();
        Serial.printf("%s gesendet in Topic: %s \n", msg, publishTopic);
        Serial.println("MQTT gesendet");
    }

    if (otaEnabled)
    {
        // Port defaults to 8266
        // ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        ArduinoOTA.setHostname(hostName);

        // No authentication by default
        ArduinoOTA.setPassword((const char *)hostName);

        ArduinoOTA.begin();
    }
    else
    {

        WiFi.disconnect(true);
        delay(1);
        ESP.deepSleep(sleepTime * 1000, WAKE_RF_DISABLED);
        delay(100);
    }
}
/**
 * @brief Schleifenfunktion
 * 
 */
void loop()
{
    unsigned long currentMillis = millis();
    // if enough millis have elapsed
    if ((currentMillis - previousMillis[0] >= sleepTime))
    {
        otaEnabled = checkOTA();
        if (sensorsConnected)
        {
            readSensors();
        }
        sendData();
        previousMillis[0] = currentMillis;
    }
    if (otaEnabled)
    {
        client.loop();
        ArduinoOTA.handle();
    }
    else
    {
        WiFi.disconnect(true);
        delay(1);
        ESP.deepSleep(sleepTime * 1000, WAKE_RF_DISABLED);
        delay(100);
    }
    delay(500);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    deserializeJson(incomeDoc, payload, length);
}

boolean checkOTA()
{
    HTTPClient http;
    if (http.begin(ethClient, httpServerWiFi))
    {
        http.GET();
        String s = http.getString();
        char payload[s.length() + 1];
        strcpy(payload, s.c_str()); // or pass &s[0]
        byte tmp = strcasecmp((char *)payload, "true");
        return (tmp == 0);
    }
    else
    {
        return true;
    }
}

void readSensors()
{
    battery["v"] = ads.readVoltage(A0GND, 181, 333);
    powerSupply["v"] = ads.readVoltage(A1GND);
    bme.takeForcedMeasurement();
    delay(15);
    temperature["v"] = bme.readTemperature();
    pressure["v"] = bme.readPressure() / 100;
    humidity["v"] = bme.readHumidity();
}

void sendData()
{
    otaStatus["v"] = otaEnabled;
    serializeJson(doc, msg);
    Serial.printf("Stringlänge: %d;\n zeichen nach länge:%d", strlen(msg), msg[strlen(msg) - 1]);
    client.beginPublish(publishTopic, strlen(msg), true);
    for (byte i = 0; i < strlen(msg); i++)
    {
        client.write(msg[i]);
        yield();
    }
    Serial.println("");
    client.endPublish();
    delay(100);
}

float voltageMesure()
{
    float adc = (float)analogRead(A0);
    return vFactor3 * pow(adc, 3) + vFactor2 * pow(adc, 3) + vFactor1 * adc + vFactor0;
}

uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffff;
    while (length--)
    {
        uint8_t c = *data++;
        for (uint32_t i = 0x80; i > 0; i >>= 1)
        {
            bool bit = crc & 0x80000000;
            if (c & i)
            {
                bit = !bit;
            }

            crc <<= 1;
            if (bit)
            {
                crc ^= 0x04c11db7;
            }
        }
    }

    return crc;
}