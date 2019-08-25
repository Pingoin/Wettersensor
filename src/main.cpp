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
    client.disconnect(); //eventuell vorhandene Alte Verbindung löschen
    Serial.begin(115200);
    setup_wifi(); //Wlan Starten
    client.setServer(IPAddress(MQTT_BROKER), 1883);
    client.setCallback(callback);
    Wire.begin();
   if (!bme.begin(bme280I2C))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }
    if (client.connect("Wetterstation"))
    {
        Serial.println("MQTT Verbindung erfolgreich");
        
#ifndef sleepTime
        client.subscribe(listenTopic);
        client.publish(listenTopic, "Verbindung steht");
#endif
    }
    else
    {
        Serial.println("MQTT Verbindung nicht erfogreich");
    }
#ifdef sleepTime
    publishSensors();
    delay(100);
    ESP.deepSleep(sleepTime);
    delay(100);
#endif
}
/**
 * @brief Schleifenfunktion
 * 
 */
void loop()
{
#ifndef sleepTime
    if (!client.loop())
    {
        Serial.println("MQTT Abgebrochen");
        delay(5000);
        if (client.connect("arduinoClient"))
        {
            Serial.println("MQTT wiederhergestellt");
            client.subscribe(listenTopic);
            client.publish(publishTopic, "neue Verbindung");
        }
    }
#endif
}

void publishSensors()
{
    snprintf(msg, msgLength, "{\"Luftdruck\":%.2f,\"Temperatur\":%.2f,\"Luftfeuchte\":%.2f}", bme.readPressure(), bme.readTemperature(), bme.readHumidity());
    Serial.println(client.state());
    Serial.println(publishTopic);
    client.publish(publishTopic, msg);
    Serial.println("Sensordaten gesendet:");
    Serial.println(msg);
}

void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
#if defined(staticIP) && defined(staticGateway) && defined(staticSubnet) && defined(staticDNS)
    IPAddress ip(staticIP);
    IPAddress gateway(staticGateway);
    IPAddress subnet(staticSubnet);
    IPAddress dns(staticDNS);
    WiFi.config(ip, dns, gateway, subnet);
#endif
    WiFi.begin(SSID, PSK);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    // In order to republish this payload, a copy must be made
    // as the orignal payload buffer will be overwritten whilst
    // constructing the PUBLISH packet.

    // Allocate the correct amount of memory for the payload copy
    byte *p = (byte *)malloc(length);
    // Copy the payload to the new buffer
    memcpy(p, payload, length);
    if (strcasecmp(topic, listenTopic) == 0)
    { //anpassen
        publishSensors();
    }
    // Free the memory
    free(p);
}