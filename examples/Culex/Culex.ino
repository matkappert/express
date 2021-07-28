#include <Arduino.h>
#define USE_MENU  true
#define USE_NVS   true
#define USE_WIFI  true
#define USE_CULEX true
#define USE_LED   true

#include "Settings.h"
#if (USE_MENU == true)
  #include "express_console_menu.h"
#else
  #include "express_console.h"
#endif
#if (USE_WIFI == true)
  #include "express_wifi.h"
#endif
#if (USE_CULEX == true)
  #include "express_culex.h"
#endif
#if (USE_PLOT == true)
  #include "express_plot.h"
#endif
#if (USE_LED == true)
  #include "express_status_led.h"
#endif

uint8_t dot = 0;

struct trans_dot_t : CULEX_TRANSPORT {
  trans_dot_t() : CULEX_TRANSPORT({(char *)"dot", (EXPRESS_TYPE_ENUM)UInt8}) {
    this->server_permissions = READ_PERMISSION;
    this->user_permissions   = READ_PERMISSION;
    this->value_uint8        = &dot;
    this->timer              = 1000;
  }
  boolean update() {
    dot++;
    return false;
  }
} trans_dot;

struct menu_connect_t : MENU_STRUCT {
  menu_connect_t() : MENU_STRUCT({(char *)"Connect or Disconnect to Culex server"}) {
    this->commands.push_back((char *)"c");
    this->commands.push_back((char *)"connect");
    eMenu.MENU_MAIN_VECTOR.push_back(this);  // add to main menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    isTrue_t value = eMenu.argIsTrue(arg);
    if (value == isError) {
      eMenu.v().pln("ERROR: unkown input! [true|false]");
      return;
    } else if (value == isTrue) {
      eCulex.connect();
      // trans_bdSeq.value.UInt8++;
    } else {
      eCulex.disconnect();
    }
  }
} menu_connect;

// struct menu_speedTest_t;

// replace speedTest
#define SPEED_TEST_COUNT (uint8_t)10
uint32_t speed_start;
uint32_t speed_end;
uint8_t speed_lap[SPEED_TEST_COUNT];  // in seconds
uint8_t speed_counter;
uint32_t speed_value;

struct trans_speedTest_t : CULEX_TRANSPORT {
  trans_speedTest_t() : CULEX_TRANSPORT({(char *)"speedTest", (EXPRESS_TYPE_ENUM)UInt32}) {
    this->server_permissions = READ_AND_WRITE_PERMISSION;
    this->value_uint32       = &speed_value;
  }
  void callback(JsonObject object, const char *type) override {
    if (eCulex.typeToValue(&this->value_uint32, type, object)) {
      eMenu.error("speedTest").pln();
      return;
    }
    if (speed_value == (speed_counter * 1024) / 2) {
      speed_end                = millis();
      speed_lap[speed_counter] = (speed_end - speed_start);
      eMenu.v().p(speed_counter).p("/").p(SPEED_TEST_COUNT - 1).p(": ").p(speed_lap[speed_counter]).pln("ms");
      if (speed_counter + 1 < SPEED_TEST_COUNT) {
        delay(500);
        speed_counter++;
        speed_start = millis();
        speed_end   = 0;
        speed_value = speed_counter * 1024;
        // *this->value_uint32 = speed_value;
        this->publish();
      } else {
        speed_counter       = 0;
        float average_speed = 0;
        for (uint8_t lap = 0; lap < SPEED_TEST_COUNT; lap++) {
          average_speed = average_speed + speed_lap[lap];
        }
        eMenu.v().p("average speed: ").p(average_speed / SPEED_TEST_COUNT).pln("ms");
      }
    } else {
      eMenu.error(__func__).p("unkown data");
    }
  }
} trans_speedTest;

struct menu_speedTest_t : MENU_STRUCT {
  menu_speedTest_t() : MENU_STRUCT({(char *)"run a Culex speed test"}) {
    this->commands.push_back((char *)"speed");
    this->commands.push_back((char *)"speedTest");
    eMenu.MENU_MAIN_VECTOR.push_back(this);  // add to main menu vector
  }
  void callback(const char *cmd, const char *arg, const uint8_t length) override {
    speed_counter = 0;
    speed_start   = millis();
    speed_end     = 0;
    speed_value   = 0;
    // trans_speedTest.value.UInt32 = 0;
    trans_speedTest.publish();
  }
} menu_speedTest;

void setup() {
  eLED.init();
  eLED.on();
  Serial.begin(BAUDRATE);
  eMenu.init(Serial);
  eMenu.version = {1, 2, 3};

  eWifi.init(&WiFi);
  eCulex.init(&WiFi);
}

void loop() {

  eCulex.update();
  eMenu.update();
}