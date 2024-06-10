## Proyecto lector RFID con conexión mediante protocolo MQTT.

En este proyecto tenemos un lector de tarjetas RFID que publica las tarjetas leídas en un cierto tópico. Un servicio Backend recibe esa publicación, verifica que el código RFID pertenezca a un usuario autorizado y publica una respuesta.

## Estructura del proyecto
```
ESP-RC522/
│
├── Documentacion/
│
├── imgs/
│   ├── circuito.jpg
│   ├── diagrama.jpg
│   └── esp32_pinout.jpg
│
├── main/
│   ├── include/
│   │    ├── base.h/
│   │    ├── guards.h/
│   │    ├── mqtt.h/
│   │    ├── rc522_registers.h/
│   │    ├── rc522.h/
│   │    ├── wifi_credentials.h/
│   │    └── wifi.h/
│   │ 
│   ├── main.c
│   ├── mqtt.c
│   ├── rc522.c
│   └── wifi.c/
│ 
├── .gitignore
├── CMakeLists.txt
└── README.md
```
### Observaciones
- **Librerías en `include/`:** Aquí se encuentran los archivos de encabezado que contienen las definiciones de las funciones y estructuras utilizadas en el proyecto.
## Conexión del modulo RFID    

En la configuración por defecto usamos el puerto SPI3 con los siguientes pines:
```
rc522_config_t config = {
    .spi.host = VSPI_HOST,
    .spi.miso_gpio = 25,
    .spi.mosi_gpio = 23, 
    .spi.sck_gpio = 19,
    .spi.sda_gpio = 22,
    .spi.device_flags= SPI_DEVICE_HALFDUPLEX
};
```

La placa utilizada para realizar las pruebas tiene la siguiente distribución de pines.
![pinout_esp32](./imgs/esp32_pinout.jpg)


Revisar que su placa de desarrollo coincida con el siguiente pinout.



### Imagen del circuito montado

![circuito](./imgs/circuito.jpg)


### Funcionamiento.
![funcionamiento](./imgs/funcionamiento.gif)


### Diagrama de funcionamiento del dispositivo.

![diagrama](./imgs/diagrama.png)







## ESP32: lector RFID




# Lector de Tarjetas RFID con MQTT y Wi-Fi

Este proyecto implementa un lector de tarjetas RFID utilizando el módulo RC522 con un ESP32. Los datos de las tarjetas detectadas se envían a un broker MQTT, y un LED indica si una tarjeta específica es reconocida.


### Configuración
1. **Credenciales Wi-Fi:** Almacena tu SSID y contraseña de Wi-Fi en `wifi_credentials.h` dentro de include.
```c
#ifndef _WIFICREDENTIALS_H
#define _WIFI_CREDENTIALS_H

#define WIFI_CREDENTIALS_ID    ("name")
#define WIFI_CREDENTIALS_PASS  ("password")

#endif
```
3. **URL del MQTT:** Define la URL de tu broker MQTT (por defecto: `mqtt://broker.hivemq.com`).


### Manejo de Eventos
- **Tarjeta RFID Escaneada:** Cuando se escanea una tarjeta, su número de serie se registra y se publica en el tema `request` del broker MQTT.
- **Datos Recibidos por MQTT:** Al recibir datos, se extrae el número de serie y se compara con un número de tarjeta predefinido. El LED se enciende si los números coinciden, y se apaga si no coinciden.

### Bucle Principal
El bucle principal asegura que el LED se apague si no se detecta ninguna tarjeta.

### Descripción del Código

#### Función Principal: `get_data`

Esta función se encarga de procesar los datos recibidos a través de MQTT, extraer el número de serie de la tarjeta RFID y comparar este número con el número de tarjeta válido. Si coinciden, enciende el LED; de lo contrario, lo apaga.

```c
static void get_data(char* data, char* topic) {
    printf("\n[%s] %s\n", topic, data);
    // Extraer el número de serie de la cadena recibida
    char* start = strstr(data, "[sn: ");
    if (start != NULL) {
        start += 5; // Mover el puntero al inicio del número de serie
        char* end = strchr(start, ']'); // Buscar el final del número de serie
        if (end != NULL) {
            *end = '\0'; // Terminar la cadena en el punto final del número de serie
            // Compara el número de serie extraído con el número de tarjeta deseado
            if (strcmp(start, tarjeta_valida) == 0) {
                gpio_set_level(LED_GPIO, 1); // Enciende el LED si los números son iguales
            } else {
                gpio_set_level(LED_GPIO, 0); // Apaga el LED si los números son diferentes
            }
        }
    }
}