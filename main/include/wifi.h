#ifndef _WIFI_H
#define _WIFI_H
#include  "base.h"








/**
 * @brief Inicio conexión Wifi en STATION MODE por default.
 * 
 * @param WIFI_ID Nombre de la red WiFi
 * @param PASS  Contraseña de la red
 * @param cb_conn Funcion callback se llama cuando el dispositivo se conecto a la red
 * @param cb_desconn  Funcion callback se llama cuando el dispositivo se desconecto de la red.
 * @return esp_err_t 
 */
 esp_err_t wifi_connect(const char* WIFI_ID, const char * PASS,
                        esp_callback_t cb_conn, esp_callback_t cb_desconn);

/**
 * @brief Libera los recursos asignados al WiFi.
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_desconnect(void);

#endif