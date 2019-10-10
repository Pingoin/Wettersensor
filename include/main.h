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
#pragma once
#ifndef MAIN_H
#define MAIN_H
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <string.h>
#include <ArduinoOTA.h>
#include <esp8266httpclient.h>
#include <user_interface.h>
#include <ArduinoJson.h>
#include <ads111x.h>
/**
 * @brief maximale Länge des Strings für den Nachrichtenaustausch
 * 
 */
#define msgLength 255
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
 * @brief Zeitpunkte der Letzten Ausführung Bestimmter Aktionen
 * 
 * für die Vermeidung von Delays
 * 0 -> Sensoren Ausgelesen und gesendet
 * 
 */
unsigned long previousMillis[1] = {0};

/**
 * @brief Liest den gewünschten Status d
 * 
 * @return byte 
 */
boolean checkOTA();
/**
 * @brief 
 * 
 */
boolean otaEnabled=false;
boolean sensorsConnected=false;

/**
 * @brief 
 * 
 */
void readSensors();
void sendData();

ADS1115 ads(0x48); 	

float voltageMesure();

#define vFactor0 0
#define vFactor1 4.9325e-3 
#define vFactor2 0
#define vFactor3 0
  StaticJsonDocument<256> incomeDoc;
  StaticJsonDocument<255> doc;
  JsonObject humidity;
  JsonObject pressure;
  JsonObject temperature;
  JsonObject battery;
  JsonObject powerSupply;
  JsonObject otaStatus;

#endif