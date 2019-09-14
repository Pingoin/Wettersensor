/**
 * @file main.h
 * @author Pingoin (p.drente@gmx.de)
 * @brief 
 * @version 0.1
 * @date 25.08.2019
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef MAIN_H
#define MAIN_H
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <string.h>
#include <ArduinoOTA.h>
#include <esp8266httpclient.h>
/**
 * @brief maximale Länge des Strings für den Nachrichtenaustausch
 * 
 */
#define msgLength 80
/**
 * @brief 
 * 
 */
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF
/**
 * @brief Sensorklasse des BME280
 * 
 */
Adafruit_BME280 bme;
/**
 * @brief Netzwerkklasse
 * 
 */
WiFiClient ethClient;
/**
 * @brief MQTT-Clientklasse
 * 
 */
PubSubClient client(ethClient);
/**
 * @brief String für den Datenaustausch
 * 
 */
char msg[msgLength];
/**
 * @brief MQTT-Callback-Funktion
 * 
 * Wird ausgeführt, wenn auf dem MQTT-Broker in einem Topic, dem gefolgt wird eine Nachricht eingeht
 * 
 * @param topic Name und Pfad des topics
 * @param payload Inhalt der Nachricht
 * @param length Länge der Nachricht
 */
void callback(char* topic, byte* payload, unsigned int length);
/**
 * @brief Die Sensordaten werden ausgelesen und ausgegeben
 * 
 */
void publishSensors();

/**
 * @brief Sendet einen Wert mit einheit und prefix ins MQTT
 * 
 * @param description Bezeichnung der Größe, die gesendet werden soll
 * @param value der Wert (gekürzt um den prefix (z.B.: 1013 bei 1013 hPa))
 * @param prefix der Prefix als float (100 bei hPa)
 * @param unit die einheit ohne prefix (Pa)
 */
void publishValue(char* description,float value,float prefix,int width,int prec,char* unit);

/**
 * @brief Sendet einen Wert mit einheit und prefix ins MQTT
 * 
 * @param description Bezeichnung der Größe, die gesendet werden soll
 * @param value der Wert (gekürzt um den prefix (z.B.: 1013 bei 1013 hPa))
 * @param prefix der Prefix als float (100 bei hPa)
 * @param unit die einheit ohne prefix (Pa)
 */
void publishValue(char* description,float value,float prefix,int width,int prec,char* unit,char* topic);

/**
 * @brief Erzwingt auslesen der Sensoren
 * 
 */
void bmeForceRead();

/**
 * @brief Zeitpunkte der Letzten Ausführung Bestimmter Aktionen
 * 
 * für die Vermeidung von Delays
 * 0 -> Sensoren Ausgelesen und gesendet
 * 
 */
unsigned long previousMillis[1] = {0};

/**
 * @brief 
 * 
 * @return byte 
 */
byte checkOTA();
/**
 * @brief 
 * 
 */
byte otaEnabled;
/**
 * @brief 
 * 
 */
void wakeUp();
/**
 * @brief 
 * 
 */
void sleepIn();

/**
 * @brief 
 * 
 */
byte isAsleep=true;
/**
 * @brief 
 * 
 */
byte isFirstRun=true;


#endif