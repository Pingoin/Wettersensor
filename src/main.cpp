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
    float spannung=analogRead(A0)*0.004883;


    publishValue((char *) "Luftdruck",bme.readPressure()/100,100,(char *) "Pa");
    publishValue((char *) "Temperatur",bme.readTemperature(),1,(char *) "°C");// \370 entspricht dem Gradzeichen
    publishValue((char *) "Luftfeuchte",bme.readHumidity(),1,(char *) "%");
    publishValue((char *) "Batterie",spannung,1,(char *) "V");
}


void publishValue(char* description,float value,float prefix,char* unit){
    snprintf(msg, msgLength, "{\"%s\":{\"v\":%04.4f,\"p\":%e,\"u\":\"%s\"}}",description,value,prefix,unit); 
    char topic[30];
    sniprintf(topic,30,"%s/%s",publishTopic,description);
    client.publish(topic, msg);
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