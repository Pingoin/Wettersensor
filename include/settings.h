/**
 * @file settings.h
 * @author Pingoin (p.drente@gmx.de)
 * @brief Gesammelte Einstellungen
 * @version 0.1
 * @date 2019-08-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */

/**
 * @brief Netzwerk SSID
 */
#define SSID "Brokkoli_Extrem"

/**
 * @brief WPA-Passwort
 * 
 */
#define PSK "1123581321345589144233"

/**
 * @brief Adresse des MQTT-Brokers
 * 
 * Angabe in den Byte-Gruppen getrennt durch komma (z.B.: 192,168,178,111)
 */
#define MQTT_BROKER 192, 168, 178, 110

/**
 * @brief Statische IP des MCU
 * 
 * Wenn DHCP-Benutzt wird bitte auskommentieren
 * Angabe in den Byte-Gruppen getrennt durch komma (z.B.: 192,168,178,111)
 */
#define staticIP 192, 168, 178, 188

/**
 * @brief Gateway des MCU
 * 
 * Wenn DHCP-Benutzt wird bitte auskommentieren
 * Angabe in den Byte-Gruppen getrennt durch komma (z.B.: 192,168,178,111)
 */
#define staticGateway 192, 168, 178, 1

/**
 * @brief Subnetzsmaske des MCU
 * 
 * Wenn DHCP-Benutzt wird bitte auskommentieren
 * Angabe in den Byte-Gruppen getrennt durch komma (z.B.: 255,255,255,0)
 */
#define staticSubnet 255, 255, 255, 0

/**
 * @brief DNS-SeverIP des MCU
 * 
 * Wenn DHCP-Benutzt wird bitte auskommentieren
 * Angabe in den Byte-Gruppen getrennt durch komma (z.B.: 192,168,178,111)
 */
#define staticDNS 192, 178, 168, 1

/**
 * @brief das MQTT-Topic auf das gewartet wird
 * 
 */
#define listenTopic "sensors/BME280"

/**
 * @brief MQTT-Topic, in dem die Daten veröffentlicht werden
 * 
 */
#define publishTopic "wetter/daten"

/**
 * @brief i2CAdresse des BME280
 * 
 */
#define bme280I2C 0x77

/**
 * @brief Zeit zwischen den Sendungen in Millisekunden
 * 
 * angabe in minuten*60e6 möglich (z.B.: 5*60e6)
 */
#define sleepTime 5*60e3

/**
 * @brief Bezeihnung des Controllers
 * 
 * Wird benutzt als bezeichnung für MQTT und OTA
 */
#define hostName "Wetterstation"

/**
 * @brief 
 * 
 */
#define httpServerWiFi "http://192.168.178.110:1880/wettersensor"

/**
 * @brief Faktor für Spannugnsberechnung
 * 
 */
#define analogFactor 1 //0.00478125