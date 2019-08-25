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
/**
 * @brief maximale Länge des Strings für den Nachrichtenaustausch
 * 
 */
#define msgLength 70
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
void callback(char *topic, byte *payload, unsigned int length);
/**
 * @brief Die Sensordaten werden ausgelesen und ausgegeben
 * 
 */
void pupblishSensors();
/**
 * @brief Start und einstellung der WLan-Verbindung
 * 
 */
void setup_wifi();

#endif