/*
    @file       express_wifi.cpp
    @author     matkappert
    @repo       github.com/matkappert/express_wifi
    @date       14/11/20
*/

#include "express_wifi.h"
// #include <WiFi.h>

#if (USE_WIFI == true)
  #if (USE_MENU == true)
    #include <esp_wifi.h>

    #include "express_console_menu.h"

express_wifi eWifi;  // global-scoped variable

// WIFI_REASON_UNSPECIFIED              = 1,
// WIFI_REASON_AUTH_EXPIRE              = 2,
// WIFI_REASON_AUTH_LEAVE               = 3,
// WIFI_REASON_ASSOC_EXPIRE             = 4,
// WIFI_REASON_ASSOC_TOOMANY            = 5,
// WIFI_REASON_NOT_AUTHED               = 6,
// WIFI_REASON_NOT_ASSOCED              = 7,
// WIFI_REASON_ASSOC_LEAVE              = 8,
// WIFI_REASON_ASSOC_NOT_AUTHED         = 9,
// WIFI_REASON_DISASSOC_PWRCAP_BAD      = 10,
// WIFI_REASON_DISASSOC_SUPCHAN_BAD     = 11,
// WIFI_REASON_IE_INVALID               = 13,
// WIFI_REASON_MIC_FAILURE              = 14,
// WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT   = 15,
// WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
// WIFI_REASON_IE_IN_4WAY_DIFFERS       = 17,
// WIFI_REASON_GROUP_CIPHER_INVALID     = 18,
// WIFI_REASON_PAIRWISE_CIPHER_INVALID  = 19,
// WIFI_REASON_AKMP_INVALID             = 20,
// WIFI_REASON_UNSUPP_RSN_IE_VERSION    = 21,
// WIFI_REASON_INVALID_RSN_IE_CAP       = 22,
// WIFI_REASON_802_1X_AUTH_FAILED       = 23,
// WIFI_REASON_CIPHER_SUITE_REJECTED    = 24,

// WIFI_REASON_BEACON_TIMEOUT           = 200,
// WIFI_REASON_NO_AP_FOUND              = 201,
// WIFI_REASON_AUTH_FAIL                = 202,
// WIFI_REASON_ASSOC_FAIL               = 203,
// WIFI_REASON_HANDSHAKE_TIMEOUT        = 204,
// WIFI_REASON_CONNECTION_FAIL          = 205,

void express_wifi::WiFiEvent_static(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == SYSTEM_EVENT_STA_DISCONNECTED || event == SYSTEM_EVENT_AP_STADISCONNECTED) {
    eMenu.error(__func__).p("WiFi disconnected:").pln(info.disconnected.reason);
    delay(100);
    eWifi.WiFiHasDisconnected();
    if (info.disconnected.reason <= 24) {
      eWifi.connection_retries = 0;
      eWifi.establishWiFiConnection();
    }
  } else if (event == SYSTEM_EVENT_STA_GOT_IP || event == SYSTEM_EVENT_AP_STA_GOT_IP6 || event == SYSTEM_EVENT_AP_STAIPASSIGNED) {
    eWifi.connection_retries = 0;
    eMenu.info(__func__).pln("WiFi connected!");
    #if (USE_LED == true)
    eLED.removeTask(0);
    eWifi.hasErrorBecauseNoConnection = false;
    eLED.removeTask(1);
    #endif
  } else {
    eMenu.debug(__func__).pln(eWifi.system_event_id_cstr[(int)event]);
  }
}

struct express_wifi::menu_wifi_information_t : MENU_STRUCT {
  menu_wifi_information_t() : MENU_STRUCT({(char *)"WiFi Info."}) {
    this->commands.push_back((char *)"i");
    this->commands.push_back((char *)"info");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    eMenu.printTable(express_console_menu::TABLE_HEADER_START);
    eMenu.printTable(express_console_menu::TABLE_DATA, "Status", "(value)");
    eMenu.printTable(express_console_menu::TABLE_HEADER_END);
    eMenu.printTable(express_console_menu::TABLE_ROW);

    if (WiFi.getMode() == WIFI_MODE_NULL) {
      eMenu.printTable(express_console_menu::TABLE_DATA, "Status", "DISABLED");
    } else {
      eMenu.printTable(express_console_menu::TABLE_DATA, "Status", eWifi.wl_status_cstr[(uint8_t)WiFi.status() == 255 ? 7 : (uint8_t)WiFi.status()]);
    }

    if (WiFi.getMode() == WIFI_MODE_STA) {
      eMenu.printTable(express_console_menu::TABLE_DATA, "Mode", "STA");
      eMenu.printTable(express_console_menu::TABLE_DATA, "SSID", WiFi.SSID().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Password", eMenu.mask(WiFi.psk().c_str(), 2).c_str());

      itoa(WiFi.RSSI(), eMenu.tableBuffer, 10);
      eMenu.printTable(express_console_menu::TABLE_DATA, "RSSI", eMenu.tableBuffer);
      // TODO add ip add auto or man
      eMenu.printTable(express_console_menu::TABLE_DATA, "IPv4 Address", WiFi.localIP().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "IPv6 Address", WiFi.localIPv6().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Subnet Mask", WiFi.subnetMask().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Router", WiFi.gatewayIP().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_ROW);

      eMenu.printTable(express_console_menu::TABLE_DATA, "MAC Address", WiFi.macAddress().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Hostname", WiFi.getHostname());

    } else if (WiFi.getMode() == WIFI_MODE_AP) {
      eMenu.printTable(express_console_menu::TABLE_DATA, "Mode", "AP");
      eMenu.printTable(express_console_menu::TABLE_DATA, "SSID", WiFi.softAPSSID().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Password", eMenu.mask(eWifi.wifiData.softap_pwd, 2).c_str());

      eMenu.printTable(express_console_menu::TABLE_DATA, "IPv4 Address", WiFi.softAPIP().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "IPv6 Address", WiFi.softAPIPv6().toString().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "MAC Address", WiFi.softAPmacAddress().c_str());
      eMenu.printTable(express_console_menu::TABLE_DATA, "Hostname", WiFi.softAPgetHostname());
    } else {
      eMenu.printTable(express_console_menu::TABLE_DATA, "Mode", "UNKNOWN");
    }

    eMenu.printTable(express_console_menu::TABLE_ROW);
    eMenu.printTable(express_console_menu::TABLE_DATA, "Auto Connect", WiFi.getAutoConnect() ? "TRUE" : "FALSE");
    eMenu.printTable(express_console_menu::TABLE_DATA, "Auto Reconnect", WiFi.getAutoReconnect() ? "TRUE" : "FALSE");
    eMenu.printTable(express_console_menu::TABLE_DATA, "Sleep", WiFi.getSleep() ? "TRUE" : "FALSE");
    itoa(WiFi.getTxPower(), eMenu.tableBuffer, 10);
    eMenu.printTable(express_console_menu::TABLE_DATA, "TX Power", eMenu.tableBuffer);

    eMenu.printTable(express_console_menu::TABLE_END);
  }
} menu_wifi_information;

struct express_wifi::menu_wifi_exportSettings_t : MENU_STRUCT {
  menu_wifi_exportSettings_t() : MENU_STRUCT({(char *)""}) {
    this->commands.push_back((char *)"export");
    this->hidden = true;
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    eMenu.printTable(express_console_menu::TABLE_HEADER_START);
    eMenu.printTable(express_console_menu::TABLE_DATA, "Settings", "(value)");
    eMenu.printTable(express_console_menu::TABLE_HEADER_END);
    eMenu.printTable(express_console_menu::TABLE_ROW);

    eMenu.printTable(express_console_menu::TABLE_DATA, "ssid", eWifi.wifiData.ssid);
    eMenu.printTable(express_console_menu::TABLE_DATA, "pwd", eMenu.mask(eWifi.wifiData.pwd, 2).c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "host_name", eWifi.wifiData.host_name);

    eMenu.printTable(express_console_menu::TABLE_ROW);
    eMenu.printTable(express_console_menu::TABLE_DATA, "local_IP", eWifi.wifiData.local_IP.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "gateway_IP", eWifi.wifiData.gateway_IP.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "subnet_mask", eWifi.wifiData.subnet_mask.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "primary_DNS", eWifi.wifiData.primary_DNS.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "secondary_DNS", eWifi.wifiData.secondary_DNS.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "is_static_IP", eWifi.wifiData.is_static_IP ? "TRUE" : "FALSE");
    eMenu.printTable(express_console_menu::TABLE_DATA, "is_station_mode", eWifi.wifiData.is_station_mode ? "TRUE" : "FALSE");

    eMenu.printTable(express_console_menu::TABLE_ROW);
    eMenu.printTable(express_console_menu::TABLE_DATA, "softap_ssid", eWifi.wifiData.softap_ssid);
    eMenu.printTable(express_console_menu::TABLE_DATA, "softap_pwd", eMenu.mask(eWifi.wifiData.softap_pwd, 2).c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "softap_local_IP", eWifi.wifiData.softap_local_IP.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "softap_gateway_IP", eWifi.wifiData.softap_gateway_IP.toString().c_str());
    eMenu.printTable(express_console_menu::TABLE_DATA, "softap_subnet_mask", eWifi.wifiData.softap_subnet_mask.toString().c_str());

    eMenu.printTable(express_console_menu::TABLE_END);
  }
} menu_wifi_exportSettings;

struct express_wifi::menu_wifi_reconnect_t : MENU_STRUCT {
  menu_wifi_reconnect_t() : MENU_STRUCT({(char *)"Connect|Disconnect|Reconnect to WiFi network"}) {
    this->commands.push_back((char *)"c");
    this->commands.push_back((char *)"connect");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    if (length == 0) {
      eMenu.info("Reapply WiFi settings!").pln();
      eWifi.establishWiFiConnection();
    } else {
      isTrue_t value = eMenu.argIsTrue(arg);
      if (value == isError) {
        eMenu.error("unkown input! [true|false]").pln();
        return;
      }
      if (value == isTrue) {
        eWifi.establishWiFiConnection();
      } else if (value == isFalse) {
        if (WiFi.getMode() == WIFI_MODE_AP) {
          WiFi.softAPdisconnect(true);
        } else if (WiFi.getMode() == WIFI_MODE_STA) {
          WiFi.disconnect(true);
        }
      }
    }
  }
} menu_wifi_reconnect;

struct express_wifi::menu_wifi_ssid_t : MENU_STRUCT {
  menu_wifi_ssid_t() : MENU_STRUCT({(char *)"Set WiFi SSID."}) {
    this->commands.push_back((char *)"s");
    this->commands.push_back((char *)"ssid");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    switch (length) {
      case 0:
        scanForNetworks();
        break;
      case 1:
        selectNetwork(atoi(arg));  //? atoi might not work
        break;
      default:
        manualNetwork(arg, length);
        break;
    }
  }
  void scanForNetworks() {
    eMenu.warn("Scanning for Networks ...").pln();
    eWifi.scanForWiFiNetworks();
    eMenu.printTable(express_console_menu::TABLE_HEADER_START);
    eMenu.printTable(express_console_menu::TABLE_DATA, "Index", "Name (SSID)");
    eMenu.printTable(express_console_menu::TABLE_HEADER_END);
    eMenu.printTable(express_console_menu::TABLE_ROW);
    for (uint8_t i = 0; i < eWifi.numSSID; i++) {
      utoa(i + 1, eMenu.tableBuffer, 10);
      eMenu.printTable(express_console_menu::TABLE_DATA, eMenu.tableBuffer, eWifi.ssidList[i]);
    }
    eMenu.printTable(express_console_menu::TABLE_END);
  }
  void selectNetwork(uint8_t ssidListIndex) {
    eMenu.debug("selected:").pln(ssidListIndex);
    if (ssidListIndex > 0 && ssidListIndex <= eWifi.numSSID) {
      eWifi.wifiData.ssid[0] = '\0';
      strcpy(eWifi.wifiData.ssid, eWifi.ssidList[ssidListIndex - 1]);
      eMenu.info("New SSID:").pln(eWifi.wifiData.ssid);
      eWifi.saveWiFiSettings();
    } else {
      if (eWifi.numSSID == 0) {
        eMenu.error(__func__).pln("Scan for WiFi SSID before picking one.");
      } else {
        eMenu.error(__func__).p("Unkown option, pick a rang from 1-").pln(eWifi.numSSID > 9 ? 9 : eWifi.numSSID);
      }
    }
  }
  void manualNetwork(const char *ssid, const uint8_t length) {
    if (length <= WIFI_SSID_MAX_LENGTH) {
      eWifi.wifiData.ssid[0] = '\0';
      strcpy(eWifi.wifiData.ssid, ssid);
      eWifi.saveWiFiSettings();
      eMenu.info("New SSID:").pln(eWifi.wifiData.ssid);
    } else {
      eMenu.error(__func__).p("to long! max: ").pln(WIFI_SSID_MAX_LENGTH);
    }
  }
} menu_wifi_ssid;

struct express_wifi::menu_wifi_password_t : MENU_STRUCT {
  menu_wifi_password_t() : MENU_STRUCT({(char *)"Set WiFi password."}) {
    this->commands.push_back((char *)"p");
    this->commands.push_back((char *)"pass");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    switch (length) {
      case 0:
        eMenu.info("Password: ").pln(eMenu.mask(eWifi.wifiData.pwd, 2));
        break;
      default:
        setPassword(arg, length);
        break;
    }
  }
  void setPassword(const char *password, const uint8_t length) {
    if (length <= WIFI_PASSWORD_MAX_LENGTH) {
      eWifi.wifiData.pwd[0] = '\0';
      strcpy(eWifi.wifiData.pwd, password);
      eWifi.saveWiFiSettings();
      eMenu.info("New Password:").pln(eMenu.mask(eWifi.wifiData.pwd, 2));
    } else {
      eMenu.error("Password to long! max:").pln(WIFI_PASSWORD_MAX_LENGTH);
    }
  }
} menu_wifi_password;

struct express_wifi::menu_wifi_sta_t : MENU_STRUCT {
  menu_wifi_sta_t() : MENU_STRUCT({(char *)"Station or Access point"}) {
    this->commands.push_back((char *)"sta");
    this->commands.push_back((char *)"station");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    if (length > 0) {
      isTrue_t value = eMenu.argIsTrue(arg);
      if (value == isError) {
        eMenu.error("unkown input! [true|false]").pln();
        return;
      } else {
        eWifi.wifiData.is_station_mode = value;
        eWifi.saveWiFiSettings();
      }
    }
    eMenu.info("STA Mode:").pln(eWifi.wifiData.is_station_mode ? "TRUE" : "FALSE");
  }
} menu_wifi_sta;

struct express_wifi::menu_wifi_static_t : MENU_STRUCT {
  menu_wifi_static_t() : MENU_STRUCT({(char *)"Static IP Address"}) {
    this->commands.push_back((char *)"ip");
    this->commands.push_back((char *)"static");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    if (length > 0) {
      isTrue_t value = eMenu.argIsTrue(arg);
      if (value == isError) {
        eMenu.error("unkown input! [true|false]").pln();
        return;
      } else {
        eWifi.wifiData.is_static_IP = value;
        eWifi.saveWiFiSettings();
      }
    }
    eMenu.info("Static IP Address:").pln(eWifi.wifiData.is_static_IP ? "Static" : "Auto");
  }
} menu_wifi_static;

struct express_wifi::menu_wifi_defaults_t : MENU_STRUCT {
  menu_wifi_defaults_t() : MENU_STRUCT({(char *)"Restore WiFi settings to defaults"}) {
    this->commands.push_back((char *)"reset");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    eWifi.restoreWiFiSettings();
  }
} menu_wifi_defaults;

  /**
   * @brief Macro fuction to import IP address from the menu
   *
   * @param __VALUE eWifi.wifiData param
   * @param __PRETTY humman readable value
   */
    #define IP_ADDRESS_IMPORT(__VALUE, __PRETTY)                  \
      if (length > 0) {                                           \
        if (length >= 7 || length <= 15) {                        \
          IPAddress ipv4;                                         \
          if (!ipv4.fromString(arg)) {                            \
            eMenu.error("IP address importing failed!").pln(arg); \
            return;                                               \
          }                                                       \
          eWifi.wifiData.__VALUE = ipv4;                          \
          eWifi.saveWiFiSettings();                               \
        } else {                                                  \
          eMenu.error("invalid input!").pln();                    \
          return;                                                 \
        }                                                         \
      }                                                           \
      eMenu.info(__PRETTY).pln(eWifi.wifiData.local_IP);

struct express_wifi::menu_wifi_localIP_t : MENU_STRUCT {
  menu_wifi_localIP_t() : MENU_STRUCT({(char *)"Set local IP address."}) {
    this->commands.push_back((char *)"l");
    this->commands.push_back((char *)"local");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(local_IP, "Local IP:")
  }
} menu_wifi_localIP;

struct express_wifi::menu_wifi_gatewayIP_t : MENU_STRUCT {
  menu_wifi_gatewayIP_t() : MENU_STRUCT({(char *)"Set gateway IP address."}) {
    this->commands.push_back((char *)"g");
    this->commands.push_back((char *)"gateway");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(gateway_IP, "Gateway IP:")
  }
} menu_wifi_gatwayIP;

struct express_wifi::menu_wifi_subnetMask_t : MENU_STRUCT {
  menu_wifi_subnetMask_t() : MENU_STRUCT({(char *)"Set subnet mask."}) {
    this->commands.push_back((char *)"subnet");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(subnet_mask, "Subnet mask:")
  }
} menu_wifi_subnetMask;

struct express_wifi::menu_wifi_primaryDNS_t : MENU_STRUCT {
  menu_wifi_primaryDNS_t() : MENU_STRUCT({(char *)"Set primary DNS address."}) {
    this->commands.push_back((char *)"dns1");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(primary_DNS, "Primary DNS:")
  }
} menu_wifi_primaryDNS;

struct express_wifi::menu_wifi_secondaryDNS_t : MENU_STRUCT {
  menu_wifi_secondaryDNS_t() : MENU_STRUCT({(char *)"Set secondary DNS address."}) {
    this->commands.push_back((char *)"dns2");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(secondary_DNS, "Secondary DNS:")
  }
} menu_wifi_secondaryDNS;

struct express_wifi::menu_wifi_softAP_localIP_t : MENU_STRUCT {
  menu_wifi_softAP_localIP_t() : MENU_STRUCT({(char *)"Set SoftAP local IP address."}) {
    this->commands.push_back((char *)"sl");
    this->commands.push_back((char *)"slocal");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(softap_local_IP, "SoftAP local IP:")
  }
} menu_wifi_softAP_localIP;

struct express_wifi::menu_wifi_softAP_gatwayIP_t : MENU_STRUCT {
  menu_wifi_softAP_gatwayIP_t() : MENU_STRUCT({(char *)"Set SoftAP gateway IP address."}) {
    this->commands.push_back((char *)"sg");
    this->commands.push_back((char *)"sgateway");
    eWifi.MENU_WIFI_VECTOR.push_back(this);  // add to wifi sub menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    IP_ADDRESS_IMPORT(softap_gateway_IP, "SoftAP gateway IP:")
  }
} menu_wifi_softAP_gatwayIP;

struct express_wifi::menu_wifi_enterSubMenu_t : MENU_STRUCT {
  menu_wifi_enterSubMenu_t() : MENU_STRUCT({(char *)"WiFi settings."}) {
    this->commands.push_back((char *)"wifi");
    eMenu.MENU_MAIN_VECTOR.push_back(this);  // add to main menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    eMenu.enterSubMenu(eWifi.MENU_WIFI_VECTOR);  // enter into wifi sub menu
  }
} menu_wifi_enterSubMenu;

  #endif

/**
 * @brief express_wifi initialisation
 *
 * @param WiFi wifi class pointer
 */
void express_wifi::init(WiFiClass *WiFi) {
  _WiFi = WiFi;

  #if (USE_NVS == true)
  //* import wifi settings from nvs
  loadWiFiSettings();
  #endif

  eMenu.debug(__func__).pln("Adding WiFi sub menu ... ");
  new menu_wifi_enterSubMenu_t();

  eMenu.debug(__func__).pln("Setting up WiFi event callbacks ... ");
  _WiFi->onEvent(eWifi.WiFiEvent_static);

  establishWiFiConnection();
}

void express_wifi::establishWiFiConnection() {

  #if (USE_LED == true)
  eLED.addTask(1, TASK::BLINK, 10);
  #endif
  WiFi.disconnect(true);
  delay(100);

  // eMenu.vvv().pln().p("Applying Hostname: ").p(wifiData.host_name).p(" ... ").pln(WiFi.setHostname(wifiData.host_name) ? "DONE" : "ERROR!");

  if (wifiData.is_station_mode) {
    /**
     * Station
     */
    WiFi.mode(WIFI_STA);
    if (wifiData.is_static_IP) {
      eMenu.info(__func__).p("Applying STA settings: ").p(wifiData.local_IP).p(" ... ");
      if (WiFi.config(wifiData.local_IP, wifiData.gateway_IP, wifiData.subnet_mask, wifiData.primary_DNS, wifiData.secondary_DNS)) {
        eMenu.v().pln("DONE");
      } else {
        eMenu.error(__func__).pln("STA Failed!");
      }
    }
    eMenu.debug(__func__).p("Applying radio settings ... ");
    WiFi.setSleep(WIFI_MODEM_SLEEP);
    WiFi.setTxPower((wifi_power_t)WIFI_MODEM_TX_POWER);
    WiFi.enableLongRange(WIFI_LONGRANG_MODE);
    eMenu.v().pln("DONE");
    delay(100);
    WiFi.begin(wifiData.ssid, wifiData.pwd);
    eMenu.info(__func__).pln("DONE");

  } else {

    /**
     * SoftAP
     */
    WiFi.mode(WIFI_AP);
    if (wifiData.is_static_IP) {
      eMenu.info(__func__).p("Applying SoftAP settings: ").p(wifiData.local_IP).p(" ... ");
      if (WiFi.softAPConfig(wifiData.softap_local_IP, wifiData.softap_gateway_IP, wifiData.softap_subnet_mask)) {
        eMenu.v().pln("DONE");
      } else {
        eMenu.error(__func__).pln("STA Failed!");
      }
    }
    eMenu.debug(__func__).p("Applying radio settings ... ");
    WiFi.setTxPower((wifi_power_t)WIFI_MODEM_TX_POWER);
    WiFi.enableLongRange(WIFI_LONGRANG_MODE);
    eMenu.v().pln("DONE");
    delay(100);
    WiFi.softAP(wifiData.softap_ssid, wifiData.softap_pwd);
    eMenu.info(__func__).pln("DONE");
  }
}

void express_wifi::WiFiHasDisconnected() {
  if (connection_retries >= WIFI_CONNECTION_RETRIES) {
    connection_retries = 0;
    eWifi.establishWiFiConnection();
  } else {
    connection_retries++;
    eMenu.debug(__func__).p("Connections retrying in: ").pln((WIFI_CONNECTION_RETRIES - connection_retries) + 1);  // todo use that same time \out used in culex
  #if (USE_LED == true)
    if (!hasErrorBecauseNoConnection) {
      hasErrorBecauseNoConnection = true;
      eLED.addTask(0, TASK::ERROR, 1);
    }

  #endif
  }
}

  #if (USE_NVS == true)

/**
 * @brief Load WiFi settings from NVS
 */
void express_wifi::loadWiFiSettings() {
  esp_err_t err;
  wifiData_t import_data;
  unsigned int sizeOfwifiData = sizeof(import_data);

  err = eNvs.get("wifi_data", &import_data, &sizeOfwifiData);
  if (err == ESP_OK) {
    wifiData = import_data;
  } else {
    eMenu.error(__func__).pln("Failed to load WiFi seetings");
    restoreWiFiSettings();
  }
}

/**
 * @brief Save WiFi settings to NVS
 */
void express_wifi::saveWiFiSettings() {
  esp_err_t err;
  unsigned int length = sizeof(eWifi.wifiData);

  err = eNvs.set("wifi_data", &eWifi.wifiData, length);
  if (err == ESP_OK) {
    eMenu.debug(__func__).pln("DONE");
  } else {
    eMenu.error(__func__).pln("Failed to save WiFi seetings");
  }
}
  #endif  // USE_NVS

void express_wifi::restoreWiFiSettings() {
  eMenu.warn(__func__).pln("Restoring WiFi settings to defaults ...");
  wifiData_t new_settings;

  // Host name
  uint8_t macAddress[6];
  WiFi.macAddress(macAddress);
  sprintf(new_settings.host_name, "%s-%x%x", DEFAULT_WIFI_HOST_NAME, macAddress[4], macAddress[5]);
  sprintf(new_settings.softap_ssid, "%s-%X%X", DEFAULT_WIFI_SOFTAP_SSID, macAddress[4], macAddress[5]);

  wifiData            = new_settings;
  unsigned int length = sizeof(new_settings);
  eNvs.set("wifi_data", &new_settings, length);
  eMenu.info(__func__).pln("DONE");
}

void express_wifi::scanForWiFiNetworks() {
  //? removed WiFi.disconnect because it stopped working, perhaps due to an update
  // WiFi.disconnect();
  // delay(100);
  int n = WiFi.scanNetworks();

  free(ssidList);
  ssidList = (char **)calloc(n, sizeof(char *));
  numSSID  = 0;

  for (int i = 0; i < n; i++) {
    boolean found = false;
    for (int j = 0; j < numSSID; j++) {
      if (!strcmp(WiFi.SSID(i).c_str(), ssidList[j]))
        found = true;
    }
    if (!found) {
      ssidList[numSSID] = (char *)calloc(WiFi.SSID(i).length() + 1, sizeof(char));
      sprintf(ssidList[numSSID], "%s", WiFi.SSID(i).c_str());
      numSSID++;
    }
  }
}

#endif
