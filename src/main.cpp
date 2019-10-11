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
    WiFi.begin(SSID, PSK); //Wifi Starten
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
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

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
    battery["v"] = ads.readVoltage(A0GND,181,333);
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