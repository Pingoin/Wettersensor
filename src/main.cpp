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
    client.setServer(IPAddress(MQTT_BROKER), 1883); //MQTT-Server definieren
    client.setCallback(callback);                   //Funktion definieren, die aufgerufen wird, wenn eine abbonierte nachricht ankommt
    Wire.begin();
    if (!bme.begin(bme280I2C))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
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
    if ((currentMillis - previousMillis[0] >= sleepTime) || isFirstRun)
    {   
        if (isAsleep){
            wakeUp();
        }else{
        otaEnabled = checkOTA();
        }

        publishSensors();
        delay(100);
        isFirstRun=false;
        previousMillis[0] = currentMillis;
    }
    if (otaEnabled)
    {
        client.loop();
        ArduinoOTA.handle();
    }else{
        sleepIn();
        delay(sleepTime);
    }

}

void publishSensors()
{
    delay(200);
    float spannung = analogRead(A0) * 0.004883;
    publishValue((char *)"Luftdruck", bme.readPressure(), 100, 6, 4, (char *)"Pa");
    publishValue((char *)"Temperatur", bme.readTemperature(), 1, 6, 4, (char *)"°C");
    publishValue((char *)"Luftfeuchte", bme.readHumidity(), 1, 6, 4, (char *)"%");
    publishValue((char *)"Batterie", spannung, 1, 6, 4, (char *)"V");
    publishValue((char *)"OTAStatus", (float) otaEnabled, 1, 1, 0, (char *)"-");
}

void publishValue(char *description, float value, float prefix, int width, int prec, char *unit, char *topic)
{
    char tmpChar[10];
    dtostrf(value / prefix, width, prec, tmpChar);
    snprintf(msg, msgLength, "{\"%s\":{\"v\":%s,\"p\":%.3e,\"u\":\"%s\"}}", description, tmpChar, prefix, unit);
    client.publish(topic, msg);
    Serial.println(msg);
}

void publishValue(char *description, float value, float prefix, int width, int prec, char *unit)
{
    char topic[30];
    sniprintf(topic, 30, "%s/%s", publishTopic, description);
    publishValue(description, value, prefix, width, prec, unit, topic);
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
    if ((strcasecmp(topic, listenTopic) == 0) && memcmp(p, "send", length) == 0)
    { //anpassen
        publishSensors();
    }
    // Free the memory
    free(p);
}

byte checkOTA()
{
    HTTPClient http;
    if(http.begin(httpServerWiFi)){
    http.GET();
    String s = http.getString();
    char payload[s.length() + 1];
    strcpy(payload, s.c_str()); // or pass &s[0]
    byte tmp = strcasecmp((char *)payload, "true");
    return (tmp == 0);
    }else{
        return 1;
    }
}

void wakeUp(){
    wifi_fpm_do_wakeup();
    wifi_fpm_close();

    //Serial.println("Reconnecting");
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();

    Serial.println();
    Serial.print("Verbinde Zu ");
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


    otaEnabled = checkOTA();


    if (client.connect(hostName))
    {
        Serial.println("MQTT Verbindung erfolgreich");
        if (otaEnabled)
        {
            client.subscribe(listenTopic);
            publishValue((char *)"Chip-ID", (float)ESP.getChipId(), 1, 6, 0, (char *)"-", (char *)listenTopic);
        }
    }else{
        Serial.println("MQTT Verbindung nicht erfogreich");
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
    isAsleep=false;
}

void sleepIn(){

    isAsleep=true;
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);

}