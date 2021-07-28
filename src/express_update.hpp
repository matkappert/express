/**
 * @file express_update.hpp
 * @author Mathew Kappert
 * @brief
 * @version 0.1
 * @date 28-07-2021
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __EXPRESS_UPDATE_H
#define __EXPRESS_UPDATE_H
#define EXPRESS_LOCAL_UPDATE_VER "1.0.0"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "express_utilities.h"
#include "settings.h"

#if (USE_LOCAL_UPDATE == true)
struct express_local_update;
extern express_local_update eLocalUpdate;

  #if (USE_MENU == true)
    #include "express_console_menu.h"
  #endif

  #include "express_wifi.h"

struct express_local_update {
 public:
  const String version = EXPRESS_LOCAL_UPDATE_VER;

  struct updateData_t {
    char password[LOCAL_UPDATE_PASSWORD_MAX_LENGTH + 1] = DEFAULT_LOCAL_PASSWORD;
    char hassedPassword[33]                             = "\0";
    boolean passwordIsHashed                            = false;
  } updateData;

 public:
  void init();
  void update();

  static void STATIC_updateOnStart();
  static void STATIC_updateOnProgress(unsigned int progress, unsigned int total);
  static void STATIC_updateOnEnd();
  static void STATIC_updateOnError(ota_error_t error);
};

#endif
#endif