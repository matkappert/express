/**
 * @file express_wifi.h
 * @author Mathew Kappert
 * @brief
 * @version 0.1
 * @date 14-11-2020
 *
 * @copyright Copyright (c) 2020
 *
 * @paragraph https://techtutorialsx.com/2021/01/04/esp32-soft-ap-and-station-modes/
 * @paragraph https://randomnerdtutorials.com/solved-reconnect-esp32-to-wifi/
 * @paragraph https://randomnerdtutorials.com/esp32-set-custom-hostname-arduino/
 * @paragraph https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#11
 */
#pragma once
#define EXPRESS_CONSOLE_WIFI_VER "0.1"

#include <Arduino.h>

#include "Settings.h"

/*
 * Forward-Declarations
 */
struct express_wifi;
extern express_wifi eWifi;

#ifndef USE_WIFI
  #define USE_WIFI true
#endif
#if (USE_WIFI == true)
  #if (USE_MENU == true)
    #include "express_console_menu.h"
  #endif

  #include <vector>
using std::vector;

  #include <WiFi.h>
  #if (USE_LED == true)
    #include "express_status_led.h"
  #endif

  #if defined(ESP8266) || defined(ESP8285)
    #include <ESP8266WiFi.h>
  #elif defined(ESP32)
    #include <WiFi.h>
  #endif

struct express_wifi {
 private:
  typedef uint32_t nvs_handle_t;
  boolean hasErrorBecauseNoConnection = false;

 public:
  struct wifiData_t {
    char ssid[WIFI_SSID_MAX_LENGTH + 1] = "\0";
    char pwd[WIFI_PASSWORD_MAX_LENGTH + 1]   = "\0";
    // Static STA
    IPAddress local_IP      = {DEFAULT_WIFI_LOCAL_IP_ADDRESS[0], DEFAULT_WIFI_LOCAL_IP_ADDRESS[1], DEFAULT_WIFI_LOCAL_IP_ADDRESS[2], DEFAULT_WIFI_LOCAL_IP_ADDRESS[3]};
    IPAddress gateway_IP    = {DEFAULT_WIFI_GATEWAY_IP_ADDRESS[0], DEFAULT_WIFI_GATEWAY_IP_ADDRESS[1], DEFAULT_WIFI_GATEWAY_IP_ADDRESS[2], DEFAULT_WIFI_GATEWAY_IP_ADDRESS[3]};
    IPAddress subnet_mask   = {DEFAULT_WIFI_SUBNET_MASK[0], DEFAULT_WIFI_SUBNET_MASK[1], DEFAULT_WIFI_SUBNET_MASK[2], DEFAULT_WIFI_SUBNET_MASK[3]};
    IPAddress primary_DNS   = {DEFAULT_WIFI_PRIMARY_DNS[0], DEFAULT_WIFI_PRIMARY_DNS[1], DEFAULT_WIFI_PRIMARY_DNS[2], DEFAULT_WIFI_PRIMARY_DNS[3]};
    IPAddress secondary_DNS = {DEFAULT_WIFI_SECONDARY_DNS[0], DEFAULT_WIFI_SECONDARY_DNS[1], DEFAULT_WIFI_SECONDARY_DNS[2], DEFAULT_WIFI_SECONDARY_DNS[3]};
    boolean is_static_IP    = DEFAULT_WIFI_IS_STATIC_IP;
    boolean is_station_mode = DEFAULT_WIFI_MODE_STATION;
    // Soft AP
    char softap_ssid[WIFI_SSID_MAX_LENGTH + 1] = DEFAULT_WIFI_SOFTAP_SSID;
    char softap_pwd[WIFI_PASSWORD_MAX_LENGTH + 1]   = DEFAULT_WIFI_SOFTAP_PASSWORD;
    IPAddress softap_local_IP      = {DEFAULT_WIFI_SOFTAP_LOCAL_IP_ADDRESS[0], DEFAULT_WIFI_SOFTAP_LOCAL_IP_ADDRESS[1], DEFAULT_WIFI_SOFTAP_LOCAL_IP_ADDRESS[2], DEFAULT_WIFI_SOFTAP_LOCAL_IP_ADDRESS[3]};
    IPAddress softap_gateway_IP    = {DEFAULT_WIFI_SOFTAP_GATEWAY_IP_ADDRESS[0], DEFAULT_WIFI_SOFTAP_GATEWAY_IP_ADDRESS[1], DEFAULT_WIFI_SOFTAP_GATEWAY_IP_ADDRESS[2], DEFAULT_WIFI_SOFTAP_GATEWAY_IP_ADDRESS[3]};
    IPAddress softap_subnet_mask   = {DEFAULT_WIFI_SOFTAP_SUBNET_MASK[0], DEFAULT_WIFI_SOFTAP_SUBNET_MASK[1], DEFAULT_WIFI_SOFTAP_SUBNET_MASK[2], DEFAULT_WIFI_SOFTAP_SUBNET_MASK[3]};
    char host_name[WIFI_SSID_MAX_LENGTH + 1]   = DEFAULT_WIFI_HOST_NAME; // TODO this wont work without the null end "\0";
  };
  wifiData_t wifiData;

 private:
  uint8_t connection_retries = 0;

 public:
  WiFiClass *_WiFi;
  const String version = EXPRESS_CONSOLE_WIFI_VER;

  vector<MENU_STRUCT *> MENU_WIFI_VECTOR;

  char **ssidList = NULL;
  int numSSID;

  void init(WiFiClass *WiFi);

  #if (USE_NVS == true)
  void loadWiFiSettings();
  void saveWiFiSettings();
  #endif
  void restoreWiFiSettings();

  void establishWiFiConnection();
  void scanForWiFiNetworks();
  static void WiFiEvent_static(WiFiEvent_t event, WiFiEventInfo_t info);
  void WiFiHasDisconnected();

  struct menu_wifi_ssid_t;
  struct menu_wifi_password_t;
  struct menu_wifi_information_t;
  struct menu_wifi_exportSettings_t;
  struct menu_wifi_reconnect_t;
  struct menu_wifi_defaults_t;
  struct menu_wifi_sta_t;
  struct menu_wifi_static_t;
  struct menu_wifi_localIP_t;
  struct menu_wifi_gatewayIP_t;
  struct menu_wifi_subnetMask_t;
  struct menu_wifi_primaryDNS_t;
  struct menu_wifi_secondaryDNS_t;
  struct menu_wifi_softAP_localIP_t;
  struct menu_wifi_softAP_gatwayIP_t;
  struct menu_wifi_enterSubMenu_t;

  const char *system_event_id_cstr[21] = {
      "WIFI_READY",             /**< ESP32 WiFi ready */
      "SCAN_DONE",              /**< ESP32 finish scanning AP */
      "STA_START",              /**< ESP32 station start */
      "STA_STOP",               /**< ESP32 station stop */
      "STA_CONNECTED",          /**< ESP32 station connected to AP */
      "STA_DISCONNECTED",       /**< ESP32 station disconnected from AP */
      "STA_AUTHMODE_CHANGE",    /**< the auth mode of AP connected by ESP32 station changed */
      "STA_GOT_IP",             /**< ESP32 station got IP from connected AP */
      "STA_LOST_IP",            /**< ESP32 station lost IP and the IP is reset to 0 */
      "STA_WPS_ER_SUCCESS",     /**< ESP32 station wps succeeds in enrollee mode */
      "STA_WPS_ER_FAILED",      /**< ESP32 station wps fails in enrollee mode */
      "STA_WPS_ER_TIMEOUT",     /**< ESP32 station wps timeout in enrollee mode */
      "STA_WPS_ER_PIN",         /**< ESP32 station wps pin code in enrollee mode */
      "STA_WPS_ER_PBC_OVERLAP", /*!< ESP32 station wps overlap in enrollee mode */
      "AP_START",               /**< ESP32 soft-AP start */
      "AP_STOP",                /**< ESP32 soft-AP stop */
      "AP_STACONNECTED",        /**< a station connected to ESP32 soft-AP */
      "AP_STADISCONNECTED",     /**< a station disconnected from ESP32 soft-AP */
      "AP_STAIPASSIGNED",       /**< ESP32 soft-AP assign an IP to a connected station */
      "AP_PROBEREQRECVED",      /**< Receive probe request packet in soft-AP interface */
      "GOT_IP6",                /**< ESP32 station or ap or ethernet interface v6IP addr is preferred */
  };

  const char *wl_status_cstr[8] = {
      "WL_IDLE_STATUS",  // = 0
      "WL_NO_SSID_AVAI",
      "WL_SCAN_COMPLETED",
      "WL_CONNECTED",
      "WL_CONNECT_FAILED",
      "WL_CONNECTION_LOST",
      "WL_DISCONNECTED",
      "WL_NO_SHIELD",  // = 255
  };
};
#endif
