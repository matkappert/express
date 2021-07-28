#include "express_update.hpp"

#include "Settings.h"

#if (USE_LOCAL_UPDATE == true)
express_local_update eLocalUpdate;  // global-scoped variable

void express_local_update::STATIC_updateOnStart() {
  #if (LOCAL_UPDATE_USE_LED == true)
  //   eLED.on();
  eLED.addTask(0, TASK::BLINK, 3);
  #endif
  eMenu.info("UPDATE").p("Start updating from ").pln(ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem"); // Either U_FLASH or U_SPIFFS
}
void express_local_update::STATIC_updateOnProgress(unsigned int progress, unsigned int total) {
  //   #if (LOCAL_UPDATE_USE_LED == true)
  //   eLED.addTask(0, TASK::BLINK, 5);
  //   #endif
  eMenu.info("UPDATE").p("Progress: ").p(progress / (total / 100)).pln("%");
}
void express_local_update::STATIC_updateOnEnd() {
  #if (LOCAL_UPDATE_USE_LED == true)
  eLED.removeTask(0);
  eLED.on();
  #endif
  eMenu.info("UPDATE").pln("DONE");
}
void express_local_update::STATIC_updateOnError(ota_error_t error) {
  #if (LOCAL_UPDATE_USE_LED == true)
  eLED.removeTask(0);
  eLED.error(8);
  #endif
  eMenu.error("UPDATE").pln(error);
  if (error == OTA_AUTH_ERROR)
    eMenu.error("UPDATE").pln("Auth Failed");
  else if (error == OTA_BEGIN_ERROR)
    eMenu.error("UPDATE").pln("Begin Failed");
  else if (error == OTA_CONNECT_ERROR)
    eMenu.error("UPDATE").pln("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR)
    eMenu.error("UPDATE").pln("Receive Failed");
  else if (error == OTA_END_ERROR)
    eMenu.error("UPDATE").pln("End Failed");
}

void express_local_update::init() {
  ArduinoOTA.setPort(LOCAL_UPDATE_PORT);
  ArduinoOTA.setHostname(eWifi.wifiData.host_name);
  if (updateData.passwordIsHashed) {
    ArduinoOTA.setPasswordHash(updateData.hassedPassword);
  } else {
    ArduinoOTA.setPassword(updateData.password);
  }

  ArduinoOTA.onStart(STATIC_updateOnStart).onEnd(STATIC_updateOnEnd).onProgress(STATIC_updateOnProgress).onError(STATIC_updateOnError);
  ArduinoOTA.begin();
}

void express_local_update::update() {
  ArduinoOTA.handle();
}

#endif