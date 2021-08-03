#ifndef __EXPRESS_CULEX_H
  #define __EXPRESS_CULEX_H
  /*
      @file       express_culex.h
      @author     matkappert
      @repo       github.com/matkappert/express
      @date       11/07/21
  */
  #define EXPRESS_CULEX_VER "1.0.0"
  /**
   * @ref https://pubsubclient.knolleary.net/api
   * @ref https://arduinojson.org/v6/doc/
   */
  #include <Arduino.h>

  #include "express_utilities.h"
  #include "settings.h"
  #define ARDUINOJSON_USE_LONG_LONG 1
  #include <ArduinoJson.h>

/*
 * Forward-Declarations
 */
struct express_culex;
extern express_culex eCulex;

  #if (USE_MENU == true)
    #include "express_console_menu.h"
  #endif

  // https://www.eclipse.org/tahu/spec/Sparkplug%20Topic%20Namespace%20and%20State%20ManagementV2.2-with%20appendix%20B%20format%20-%20Eclipse.pdf

  // https://www.dfrobot.com/blog-1161.html
  // https://www.dfrobot.com/blog-1177.html
  #include <PubSubClient.h>
  #include <WiFi.h>

enum CULEX_TOPICS_ENUM {
  NBIRTH = 0,
  NDEATH,
  DBIRTH,
  DDEATH,
  NDATA,
  DDATA,
  NCMD,
  DCMD,
  STATE,
  DATA,
};
static const char *CULEX_TOPICS_CHAR[] = {
    "NBIRTH",  // Birth certificate for MQTT EoN nodes.
    "NDEATH",  // Death certificate for MQTT EoN nodes.
    "DBIRTH",  // Birth certificate for Devices.
    "DDEATH",  // Death certificate for Devices.
    "NDATA",   // Node data message.
    "DDATA",   // Device data message.
    "NCMD",    // Node command message.
    "DCMD",    // Device command message.
    "STATE",   // Critical application state message.
    "DATA",    //  data message.
};
static const char *MQTT_CONNECTION_ERRORS_CHAR[] = {
    "MQTT_CONNECTION_TIMEOUT",       // [-4] the server didn't respond within the keepalive time
    "MQTT_CONNECTION_LOST",          // [-3] the network connection was broken
    "MQTT_CONNECT_FAILED",           // [-2] the network connection failed
    "MQTT_DISCONNECTED",             // [-1] the client is disconnected cleanly
    "MQTT_CONNECTED",                // [0] the client is connected
    "MQTT_CONNECT_BAD_PROTOCOL",     // [1] the server doesn't support the requested version of MQTT
    "MQTT_CONNECT_BAD_CLIENT_ID",    // [2] the server rejected the client identifier
    "MQTT_CONNECT_UNAVAILABLE",      // [3] the server was unable to accept the connection
    "MQTT_CONNECT_BAD_CREDENTIALS",  // [4] the username/password were rejected
    "MQTT_CONNECT_UNAUTHORIZED",     // [5] the client was not authorized to connect
};

enum CULEX_PERMISSIONS_e {
  NO_PERMISSION             = 0,  // No access permission
  WRITE_PERMISSION          = 2,  // Write permission
  READ_PERMISSION           = 4,  // Read permission
  READ_AND_WRITE_PERMISSION = 6,  // Read and write permission: 4 (read) + 2 (write) = 6
};

struct CULEX_TRANSPORT;

struct express_culex {
 public:
  const String version = EXPRESS_CULEX_VER;

 private:
  WiFiClass *_WiFi;
  WiFiClient _clientTcpCulex;
  PubSubClient culexClient = PubSubClient(_clientTcpCulex);
  boolean hasErrorBecauseNoConnection = false;
  // char payload_buf[CULEX_PAYLOAD_SIZE];

  // connection
  boolean isConnected              = false;
  uint32_t connection_timeout_last = 0;
  uint16_t connection_timeout_wait = CULEX_CONNECTION_WAIT;

 public:
  vector<CULEX_TRANSPORT *> CULEX_TRANSPORT_VECTORS;

  

 public:
  void init(WiFiClass *WiFi);
  void update();
  boolean connect();
  void disconnect();
  //   void topic(char *buf, CULEX_TOPICS_ENUM topic);
  // void payload(char *buf);
  // void metric(char *buf, CULEX_TRANSPORT *transport);
  void birth();

uint8_t _message_bdSeq = 0;
  uint32_t timestamp = 0;
  uint32_t _message_seq = 0;
  uint32_t message_seq(){
    _message_seq++;
    return _message_seq;
  }

  
 public:
  StaticJsonDocument<CULEX_PAYLOAD_SIZE> doc;

 public:
  static void STATIC_DATA_EVENT(const char *topic, byte *payload, unsigned int length) {
    eCulex.DATA_EVENT(topic, payload, length);
  };
  void DATA_EVENT(const char *topic, byte *payload, unsigned int length);

  void generateTopic(char *ptr, const char *method);
  void generateTopics();
  void generatePayload(CULEX_TRANSPORT *transport);

  void postTopics();

  #define TYPE_TO_VALUE(TYPE, ENUM)                                                                          \
    boolean typeToValue(TYPE **value, const char *type, JsonObject objectValue, const char *key = "value") { \
      if (strcmp(type, EXPRESS_TYPE_CHAR[ENUM]) == 0) {                                                      \
        **value = objectValue[key].as<TYPE>();                                                               \
        return false;                                                                                        \
      } else {                                                                                               \
        return true;                                                                                         \
      }                                                                                                      \
    }
  TYPE_TO_VALUE(int8_t, Int8);
  TYPE_TO_VALUE(int16_t, Int16);
  TYPE_TO_VALUE(int32_t, Int32);
  TYPE_TO_VALUE(uint8_t, UInt8);
  TYPE_TO_VALUE(uint16_t, UInt16);
  TYPE_TO_VALUE(uint32_t, UInt32);
  TYPE_TO_VALUE(float, Float);
  TYPE_TO_VALUE(double, Double);
  TYPE_TO_VALUE(boolean, Boolean);

  char valueToBuffer_buffer[(20 * sizeof(char)) + 1];
  String valueToBuffer(CULEX_TRANSPORT *transport);
};  //! culex

/**
 * Culex transport templet.
 *
 * @callback sub //! update
 * @param name the topic name.
 * @param type the topic name.
 */

struct CULEX_TRANSPORT {
  const char *name;
  char topic[CULEX_TOPIC_SIZE];
  EXPRESS_TYPE_ENUM type;
  // EXPRESS_TYPE_UNION value;
  CULEX_PERMISSIONS_e server_permissions = READ_PERMISSION;
  CULEX_PERMISSIONS_e user_permissions   = NO_PERMISSION;
  int8_t *value_int8;
  int16_t *value_int16;
  int32_t *value_int32;
  uint8_t *value_uint8;
  uint16_t *value_uint16;
  uint32_t *value_uint32;
  float *value_float;
  double *value_double;
  boolean *value_boolean;
  uint16_t timer;
  uint32_t timer_last;

  CULEX_TRANSPORT(char *name, EXPRESS_TYPE_ENUM type) {
    this->name = name;
    this->type = type;
    eCulex.CULEX_TRANSPORT_VECTORS.push_back(this);
  }
  virtual void callback(JsonObject objectValue, const char *type) {}
  virtual boolean update() {}
  void publish() {
    eCulex.generatePayload(this);
  }
};

#endif

// /*
//   Important to not set vTaskDelay to less then 10. Errors begin to develop with the MQTT and network connection.
//   makes the initial wifi/mqtt connection and works to keeps those connections open.
// */
// void MQTTkeepalive( void *pvParameters )
// {
//   sema_MQTT_KeepAlive   = xSemaphoreCreateBinary();
//   xSemaphoreGive( sema_MQTT_KeepAlive ); // found keep alive can mess with a publish, stop keep alive during publish
//   MQTTclient.setKeepAlive( 90 ); // setting keep alive to 90 seconds makes for a very reliable connection, must be set before the 1st connection is made.
//   TickType_t xLastWakeTime = xTaskGetTickCount();
//   const TickType_t xFrequency = 250; //delay for ms
//   for (;;)
//   {
//     //check for a is-connected and if the WiFi 'thinks' its connected, found checking on both is more realible than just a single check
//     if ( (wifiClient.connected()) && (WiFi.status() == WL_CONNECTED) )
//     {
//       xSemaphoreTake( sema_MQTT_KeepAlive, portMAX_DELAY ); // whiles MQTTlient.loop() is running no other mqtt operations should be in process
//       MQTTclient.loop();
//       xSemaphoreGive( sema_MQTT_KeepAlive );
//     }
//     else {
//       log_i( "MQTT keep alive found MQTT status % s WiFi status % s", String(wifiClient.connected()), String(WiFi.status()) );
//       if ( !(wifiClient.connected()) || !(WiFi.status() == WL_CONNECTED) )
//       {
//         connectToWiFi();
//       }
//       connectToMQTT();
//     }
//     xLastWakeTime = xTaskGetTickCount();
//     vTaskDelayUntil( &xLastWakeTime, xFrequency );
//   }
//   vTaskDelete ( NULL );
// }