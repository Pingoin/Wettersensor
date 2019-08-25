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
    client.disconnect();
    Serial.begin(115200);
    setup_wifi();
    client.setServer(IPAddress(MQTT_BROKER), 1883);
    client.setCallback(callback);
    if (!bmp.begin())
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }
    if (client.connect("arduinoClient"))
    {
        Serial.println("MQTT Verbindung erfolgreich");
        client.subscribe(listenTopic);
        client.publish(listenTopic, "Verbindung steht");
    }
    else
    {
        Serial.println("MQTT Verbindung nicht erfogreich");
    }
}
/**
 * @brief Schleifenfunktion
 * 
 */
void loop()
{
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
}

void pupblishSensors()
{
    snprintf(msg, msgLength, "{\"Luftdruck\":%.2f,\"Temperatur\":%.2f,\"Luftfeuchte\":%.2f}", bmp.readPressure(), bmp.readTemperature(), bmp.readHumidity());
    Serial.println(msg);
    client.publish(publishTopic, msg);
    Serial.println("Sensordaten gesendet");
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

// Callback function
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
        pupblishSensors();
    }
    // Free the memory
    free(p);
}