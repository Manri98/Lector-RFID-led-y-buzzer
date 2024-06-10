#include <esp_log.h>
#include <inttypes.h>
#include "rc522.h"
#include "mqtt.h"
#include "wifi.h"
#include "wifi_credentials.h"
#include <driver/gpio.h>
#include <string.h>

#define LED_GPIO_1    2
#define LED_GPIO_2    4
#define BUZZER_GPIO   5
#define MQTT_URL      "mqtt://broker.hivemq.com"

static const char* TAG = "Lector de tarjetas";
static bool tarjeta_detectada = false;
static rc522_handle_t scanner;

rc522_config_t config = {
    .spi.host = VSPI_HOST,
    .spi.miso_gpio = 25,
    .spi.mosi_gpio = 23, 
    .spi.sck_gpio = 19,
    .spi.sda_gpio = 22,
    .spi.device_flags = SPI_DEVICE_HALFDUPLEX
};

#define LEN_BUFFER      100
char buffer[LEN_BUFFER]= {0};
const char* tarjeta_valida_1 = "215648837556"; // Número de serie de la primera tarjeta
const char* tarjeta_valida_2 = "1027575116129"; // Número de serie de la segunda tarjeta

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data) {
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;
    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                sprintf(buffer,"Verificar tarjeta [sn: %" PRIu64 "]", tag->serial_number);
                ESP_LOGI(TAG,"%s\n",buffer );
                mqtt_publish(buffer,"request",2,0);
            }
            break;
    }
}

static void RFID_reader_init() {
    rc522_create(&config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);
}

static void buzzer_beep(int duration_ms, int times) {
    for (int i = 0; i < times; i++) {
        gpio_set_level(BUZZER_GPIO, 1);
        vTaskDelay(duration_ms / portTICK_PERIOD_MS);
        gpio_set_level(BUZZER_GPIO, 0);
        vTaskDelay(duration_ms / portTICK_PERIOD_MS);
    }
}

static void get_data(char* data, char* topic) {
    printf("\n[%s] %s\n", topic, data);
    char* start = strstr(data, "[sn: ");
    if (start != NULL) {
        start += 5;
        char* end = strchr(start, ']');
        if (end != NULL) {
            *end = '\0';
            if (strcmp(start, tarjeta_valida_1) == 0) {
                gpio_set_level(LED_GPIO_1, 1);
                gpio_set_level(LED_GPIO_2, 0); // Asegúrate de que el segundo LED esté apagado si se lee la primera tarjeta
                buzzer_beep(250, 2); // Suena dos veces por 250 ms cada vez
            } else if (strcmp(start, tarjeta_valida_2) == 0) {
                gpio_set_level(LED_GPIO_1, 0); // Asegúrate de que el primer LED esté apagado si se lee la segunda tarjeta
                gpio_set_level(LED_GPIO_2, 1);
                buzzer_beep(1000, 1); // Suena una vez por 5000 ms
            } else {
                gpio_set_level(LED_GPIO_1, 0);
                gpio_set_level(LED_GPIO_2, 0);
            }
        }
    }
}

static void mqtt_connected() {
    printf("init mqtt\n");
    mqtt_subcribe("request", 2);
}

static void callback_wifi_connected() {
    printf("WIFI Ok\n");
    mqtt_init(MQTT_URL, mqtt_connected, NULL, get_data);
}

static void led_config() {
    gpio_set_direction(LED_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_1, 0);
    gpio_set_level(LED_GPIO_2, 0);
    gpio_set_level(BUZZER_GPIO, 0);
}

int app_main() {
    wifi_connect(WIFI_CREDENTIALS_ID, WIFI_CREDENTIALS_PASS, callback_wifi_connected, NULL);
    led_config();
    RFID_reader_init();

    while(1) {
        if (!tarjeta_detectada) {
            gpio_set_level(LED_GPIO_1, 0);
            gpio_set_level(LED_GPIO_2, 0);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    return 0;
}