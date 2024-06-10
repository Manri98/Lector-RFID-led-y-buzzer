#ifndef _MQTT_H
#define _MQTT_H
#include "base.h"


/**
 * @brief Configura e inicia la conexión al servidor MQTT.
 * 
 * @param uri El URL del servidor MQTT.
 * @param conn Función que se llama al conectarse al servidor.
 * @param disconn Función que se llama al desconectarse del servidor.
 * @param data_received Función que se llama al recibir datos desde un canal subscripto.
 * @return esp_err_t Retorna ESP_OK si todo salió bien.
 */
esp_err_t mqtt_init(const char* uri, esp_callback_t conn, esp_callback_t disconn,callback_data_t data_received);



/**
 * @brief 
 * 
 * @param msg 
 * @param topic 
 * @param qos 
 * @param retain 
 * @return esp_err_t 
 */
esp_err_t mqtt_publish(const char* msg, const char* topic,int qos, int retain);

esp_err_t mqtt_publish_on_queue(const char* msg, const char* topic,int qos, int retain);

esp_err_t mqtt_subcribe( char* topic, int qos);

#endif