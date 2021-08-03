#ifndef __EXPRESS_SETTINGS_H
#define __EXPRESS_SETTINGS_H
//   #define CONFIGURATION_H_VERSION 010107
#define PROJECT_NAME "EXPRESS"
/**
 * Serial baudrate :[2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]
 */
#define BAUDRATE (int)115200

#define DEFAULT_VERBOSITY_LEVEL 3

#define cPLOT            false
#define DEFAULT_INTERVAL (uint16_t)30

/**
 *  Use non-volatile storage (NVS, flash) to save settings
 */
#define USE_NVS               true
#define USE_MENU              true
#define MENU_TABLE_WIDTH      (uint8_t)116
#define MENU_PADDING          (uint8_t)4
#define MENU_OFFSET           (uint8_t)30
#define DEFAULT_MENU_ECHO     true
#define DEFAULT_SUB_MENU_HELP (boolean) true

//* Status LED
#define USE_LED true
#define ESP32_TTGO_T8_V171

#if defined(ESP32_TTGO_T8_V171)
  #define LED_BUILTIN     21
  #define LED_IS_INVERTED (boolean) false
#elif defined(ESP32)
  #define LED_BUILTIN     2
  #define LED_IS_INVERTED (boolean) false
#else
  #define LED_IS_INVERTED (boolean) true
#endif

/**
 *
 */
#if defined(ESP8266) || defined(ESP8285) || defined(ESP32)

  #ifndef USE_WIFI
    #define USE_WIFI true
  #endif

  /**
   *  Use WiFi in station mode (connect to a network) [true]
   *  Or,
   *  use access point mode (create network) [false]
   */
  #define DEFAULT_WIFI_MODE_STATION (boolean) false

  // 19.5dBm [78]
  // 19dBm [76]
  // 18.5dBm [74]
  // 17dBm [68]
  // 15dBm [60]
  // 13dBm [52]
  // 11dBm [44]
  // 8.5dBm [34]
  // 7dBm [28]
  // 5dBm [20]
  // 2dBm [8]
  // -1dBm [-4]
  #define WIFI_MODEM_TX_POWER (int8_t)78
  #define WIFI_MODEM_SLEEP    (boolean) false
  #define WIFI_LONGRANG_MODE  (boolean) false

  #define WIFI_CONNECTION_RETRIES 8

  #define WIFI_SSID_MAX_LENGTH     (uint8_t)32
  #define WIFI_PASSWORD_MAX_LENGTH (uint8_t)64
  /**
   *  Configures static IP address
   *  Automatic [false]
   *  Static [true]
   */
  #define DEFAULT_WIFI_IS_STATIC_IP (boolean) false

  // clang-format off
  #define DEFAULT_WIFI_LOCAL_IP_ADDRESS (uint8_t[4]) { 10,0,0,150 }
  #define DEFAULT_WIFI_GATEWAY_IP_ADDRESS (uint8_t[4]) { 10,0,0,1 }
  #define DEFAULT_WIFI_SUBNET_MASK (uint8_t[4]) { 255,255,0,0 }

  /** optional */
  #define DEFAULT_WIFI_PRIMARY_DNS (uint8_t[4]) { 1,1,1,1 }
  #define DEFAULT_WIFI_SECONDARY_DNS  (uint8_t[4]) { 8,8,8,8 }

  #define DEFAULT_WIFI_SOFTAP_SSID  "EXPRESS"
  #define DEFAULT_WIFI_SOFTAP_PASSWORD  "12345678"
  #define DEFAULT_WIFI_SOFTAP_LOCAL_IP_ADDRESS (uint8_t[4]) { 10,0,0,1 }
  #define DEFAULT_WIFI_SOFTAP_GATEWAY_IP_ADDRESS (uint8_t[4]) { 10,0,0,1 }
  #define DEFAULT_WIFI_SOFTAP_SUBNET_MASK (char[4]) { 255,255,255,0 }

  #define DEFAULT_WIFI_HOST_NAME  "express"
  // clang-format on

  /**
   * UPDATE
   *
   */
  #define USE_LOCAL_UPDATE                 true
  #define LOCAL_UPDATE_PORT                3232
  #define DEFAULT_LOCAL_PASSWORD           "admin"
  #define LOCAL_UPDATE_PASSWORD_MAX_LENGTH (uint8_t)64
  #define LOCAL_UPDATE_USE_LED             true

  /**
   *  CULEX
   */
  #define USE_CULEX true
  // clang-format off
  #define DEFAULT_CULEX_SEVER_IP_ADDRESS  (IPAddress) { 10, 0, 0, 10 }
  // clang-format on
  #define DEFAULT_CULEX_SERVER_PORT (uint16_t)1883
  #define CULEX_PAYLOAD_SIZE        (uint32_t)1028
  #define CULEX_TOPIC_SIZE          (uint16_t)160

  /**
   *  Topic names
   *  [NAMESPACE]/[USER_TOKEN]/[DEVICE_ID]/#
   *  CULEXv1.0/123456789/12345678/#
   */
  #define DEFAULT_CULEX_NAMESPACE          "CULEXv1.0"
  #define DEFAULT_CULEX_USER_TOKEN         "USER_TOKEN"
  #define DEFAULT_CULEX_DEVICE_ID          "DEVICE_ID"
  #define DEFAULT_CULEX_USERNAME           "USERNAME"
  #define DEFAULT_CULEX_PASSWORD           "PASSWORD"
  #define DEFAULT_CULEX_KEEPALIVE          (uint16_t)5   // Seconds, not milliseconds!
  #define DEFAULT_CULEX_SET_SOCKET_TIMEOUT (uint16_t)10  // Seconds, not milliseconds!

  // WILL
  #define CULEX_LWT_QOS           1
  #define CULEX_LWT_RETAIN        true
  #define CULEX_LWT_CLEAN_SESSION true

  #define CULEX_CONNECTION_WAIT (uint16_t)250  // Millisecends

#endif
#endif
