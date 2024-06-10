#include "mqtt.h"
#include "mqtt_client.h"



static esp_mqtt_client_handle_t _client= NULL;
static esp_callback_t _on_conn = NULL;
static esp_callback_t _on_disconn = NULL;
static callback_data_t _on_received_data =NULL;


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        printf( "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    printf( "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;


    switch(event_id) {
        
    case MQTT_EVENT_CONNECTED:
        CHECK_RUN_F(_on_conn);
        break;
    case MQTT_EVENT_DISCONNECTED:
        CHECK_RUN_F(_on_disconn);

        break;

    case MQTT_EVENT_SUBSCRIBED:
        printf("\nSubscripcion correcta\n");
        
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        printf("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        int len_d = event->data_len;
        int len_t = event->topic_len;
        static char b_data[100]={0};
        static char b_topic[100]={0};
        
        strncpy(b_data,event->data,len_d); 
        strncpy(b_topic,event->topic,len_d);

        b_data[len_d]= 0;
        b_topic[len_t]= 0;
        if(_on_received_data !=NULL)_on_received_data(b_data,b_topic);
     
    
        break;

    case MQTT_EVENT_ERROR:
        printf("Error, liberamos recursos del esp_mqtt\n");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            printf("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        printf("Evento no soportado:%d", event->event_id);
        break;
    }
}

esp_err_t mqtt_init(const char* uri, esp_callback_t conn, esp_callback_t disconn,callback_data_t data_received){
    _on_conn = conn;
    _on_disconn = disconn;
    _on_received_data = data_received;
    esp_err_t err= ESP_OK;

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri,
    };
    _client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(_client);
    return err;
}


esp_err_t mqtt_publish(const char* msg, const char* topic,int qos, int retain){
    if(_client == NULL) return 0;
    esp_mqtt_client_publish(_client, topic, msg, 0, qos, retain);
    return ESP_OK;
}

esp_err_t mqtt_publish_on_queue(const char* msg, const char* topic,int qos, int retain){
    if(_client == NULL) return 0;
  
    esp_mqtt_client_enqueue(_client, topic, msg, 0, qos, retain,1);
    return ESP_OK;
}

esp_err_t mqtt_subcribe( char* topic, int qos){
    if(_client ==NULL) return 0;
    esp_mqtt_client_subscribe(_client, topic, qos);
    return ESP_OK;
}