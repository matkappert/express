/*
    @file       express_console_menu.cpp
    @author     matkappert
    @repo       github.com/matkappert/express
    @date       26/09/20
*/

#include "express_console_menu.h"

#include "Settings.h"

express_console_menu eMenu;  // global-scoped variable

struct menu_system_t : MENU_STRUCT {
  menu_system_t() : MENU_STRUCT({(char *)"system settings"}) {
    this->commands.push_back((char *)"sys");
    this->commands.push_back((char *)"system");
    eMenu.MENU_MAIN_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.enterSubMenu(eMenu.MENU_SYSTEM_VECTOR);
  }
} menu_system;

/*
  MENU: INFO
  */
struct express_console_menu::menu_information_t : MENU_STRUCT {
  menu_information_t() : MENU_STRUCT({(char *)"Displays firmware info."}) {
    this->commands.push_back((char *)"i");
    this->commands.push_back((char *)"info");
    eMenu.MENU_SYSTEM_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    /**********************************
     *  Firmware
     **********************************/
    eMenu.printTable(TABLE_HEADER_START);
    eMenu.printTable(TABLE_DATA, "Firmware", "(value)");
    eMenu.printTable(TABLE_HEADER_END);
    eMenu.printTable(TABLE_ROW);
    sprintf(eMenu.tableBuffer, "v%d.%d.%d", eMenu.version.major, eMenu.version.minor, eMenu.version.patch);
    eMenu.printTable(TABLE_DATA, "Firmware Version", eMenu.tableBuffer);
    eMenu.printTable(TABLE_DATA, "Build Date", __TIMESTAMP__);
    eMenu.printTable(TABLE_DATA, "GCC Version", __VERSION__);
    eMenu.printTable(TABLE_END);

    /**********************************
     *  Modules
     **********************************/
    eMenu.printTable(TABLE_HEADER_START);
    eMenu.printTable(TABLE_DATA, "Modules", "(version)");
    eMenu.printTable(TABLE_HEADER_END);
    eMenu.printTable(TABLE_ROW);
    sprintf(eMenu.tableBuffer, "v%s", EXPRESS_CONSOLE_MENU_VER);
    eMenu.printTable(TABLE_DATA, "Menu", eMenu.tableBuffer);
#if (USE_NVS == true)
    sprintf(eMenu.tableBuffer, "v%s", eNvs.version);
    eMenu.printTable(TABLE_DATA, "NVS", eMenu.tableBuffer);
#endif
#if (USE_WIFI == true)
    sprintf(eMenu.tableBuffer, "v%s", eWifi.version);
    eMenu.printTable(TABLE_DATA, "WiFi", eMenu.tableBuffer);
#endif
#if (USE_CULEX == true)
    sprintf(eMenu.tableBuffer, "v%s", eCulex.version);
    eMenu.printTable(TABLE_DATA, "Culex", eMenu.tableBuffer);
#endif
#if (USE_PLOT == true)
    sprintf(eMenu.tableBuffer, "v%s", ePlot.version);
    eMenu.printTable(TABLE_DATA, "Plotter", eMenu.tableBuffer);
#endif
#if (USE_LED == true)
    sprintf(eMenu.tableBuffer, "v%s", eLED.version);
    eMenu.printTable(TABLE_DATA, "LED", eMenu.tableBuffer);
#endif

    eMenu.printTable(TABLE_END);

    /**********************************
     *  System
     **********************************/
    eMenu.printTable(TABLE_HEADER_START);
    eMenu.printTable(TABLE_DATA, "System", "(value)");
    eMenu.printTable(TABLE_HEADER_END);
    eMenu.printTable(TABLE_ROW);
#if defined(ESP_PLATFORM)
    sprintf(eMenu.tableBuffer, "%.2f °C", eUtil.getTemperature());
    eMenu.printTable(TABLE_DATA, "Temperature", eMenu.tableBuffer);
    sprintf(eMenu.tableBuffer, "%.2f v", eUtil.getVoltage());
    eMenu.printTable(TABLE_DATA, "Core Voltage", eMenu.tableBuffer); // TODO string  length  error
#endif
#if defined(ESP_PLATFORM) && (USE_NVS == true)
    utoa(eMenu.reboot_counter, eMenu.tableBuffer, 10);
    eMenu.printTable(TABLE_DATA, "Total Reboots", eMenu.tableBuffer);
    utoa(eMenu.reboot_counter_resettable, eMenu.tableBuffer, 10);
    eMenu.printTable(TABLE_DATA, "Resettable Reboots", eMenu.tableBuffer);
#endif
    eMenu.printTable(TABLE_END);

    /**********************************
     *  ESP
     **********************************/
#if defined(ESP_PLATFORM)
    eMenu.printTable(TABLE_HEADER_START);
    eMenu.printTable(TABLE_DATA, "ESP Platform", "(value)");
    eMenu.printTable(TABLE_HEADER_END);
    eMenu.printTable(TABLE_ROW);
    uint64_t chipid = ESP.getEfuseMac();
    sprintf(eMenu.tableBuffer, "%3X%3X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    eMenu.printTable(TABLE_DATA, "Chip ID", eMenu.tableBuffer);
    eMenu.printTable(TABLE_DATA, "Chip Model", ESP.getChipModel());
    utoa(ESP.getChipRevision(), eMenu.tableBuffer, 10);
    eMenu.printTable(TABLE_DATA, "Chip Revision", eMenu.tableBuffer);
    utoa(ESP.getChipCores(), eMenu.tableBuffer, 10);
    eMenu.printTable(TABLE_DATA, "Chip Cores", eMenu.tableBuffer);
    eMenu.printTable(TABLE_ROW);
    sprintf(eMenu.tableBuffer, "%.2f %%", 100.00 - (((float)ESP.getFreeHeap() / (float)ESP.getHeapSize()) * 100.00));
    eMenu.printTable(TABLE_DATA, "Used Heap", eMenu.tableBuffer);
    sprintf(eMenu.tableBuffer, "%d KiB", (uint32_t)ESP.getHeapSize() / 1024);
    eMenu.printTable(TABLE_DATA, "Heap Size", eMenu.tableBuffer);
    sprintf(eMenu.tableBuffer, "%d KiB", (uint32_t)ESP.getFreeHeap() / 1024);
    eMenu.printTable(TABLE_DATA, "Free Size", eMenu.tableBuffer);
    eMenu.printTable(TABLE_ROW);

    eMenu.printTable(TABLE_DATA, "SDK Version", ESP.getSdkVersion());
    sprintf(eMenu.tableBuffer, "%d MHz", (uint32_t)ESP.getFlashChipSpeed() / 1000000);
    eMenu.printTable(TABLE_DATA, "Flash Chip Size", eMenu.tableBuffer);
    FlashMode_t ideMode = ESP.getFlashChipMode();
    eMenu.printTable(TABLE_DATA, "Flash Chip Mode", ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN");

    eMenu.printTable(TABLE_END);
#endif
  }
} menu_information;

/*
    MENU: VERBOSE
    */
struct express_console_menu::menu_verbose_t : MENU_STRUCT {
  menu_verbose_t() : MENU_STRUCT({(char *)"Sets the message verbosity level."}) {
    this->commands.push_back((char *)"v");
    this->commands.push_back((char *)"verbose");
    eMenu.MENU_GLOBAL_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    if (length) {
      uint8_t level = strtol(arg, nullptr, 10);
      if (level >= 0 && level < 5) {
        eMenu.verbosity_level = level;
        eMenu._filter_level   = (express_console::Level)eMenu.verbosity_level;
        eNvs.set("verbosity_level", &eMenu.verbosity_level);
        eMenu.info("verbose set: ").pln(eMenu.verbosity_level);
      } else {
        eMenu.error("verbosity").pln("unkown level, pick a rang from 1-4");
      }
    } else {
      eMenu.v().p("verbosity: ").pln(eMenu.verbosity_level);
    }
  }
} menu_verbose;

/*
MENU: REBOOT
*/
#if defined(ESP_PLATFORM)
struct express_console_menu::menu_reboot_t : MENU_STRUCT {
  menu_reboot_t() : MENU_STRUCT({(char *)"Reboot system."}) {
    this->commands.push_back((char *)"reboot");
    eMenu.MENU_GLOBAL_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.info("menu_reboot").pln("Software Rebooting...").pln();
    delay(500);
    ESP.restart();
  }
} menu_reboot;
#endif

/*
MENU: RESET
*/
struct express_console_menu::menu_reset_t : MENU_STRUCT {
  menu_reset_t() : MENU_STRUCT({(char *)"reset system settings."}) {
    this->commands.push_back((char *)"reset");
    eMenu.MENU_SYSTEM_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.info("menu_reset").pln("Reseting system settings...").pln();
    eMenu.default_verbosity_level();
    eMenu.default_reboot_counter();
  }
} menu_reset;

/*
MENU: FACTORY RESET
*/
struct express_console_menu::menu_factoryReset_t : MENU_STRUCT {
  menu_factoryReset_t() : MENU_STRUCT({(char *)""}) {
    this->commands.push_back((char *)"factory-reset");
    this->hidden = true;
    eMenu.MENU_SYSTEM_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.info("menu_factoryReset").pln("factory resetting the system...").pln();
#if (USE_NVS == true)
    eNvs.erase_all();
  #if defined(ESP8266) || defined(ESP8285) || defined(ESP32)
    delay(1000);
    ESP.restart();
  #endif
#endif
  }
} menu_factoryReset;

// /*
// MENU: HALT
// */
// struct express_console_menu::menu_halt_t : MENU_STRUCT {
//   menu_halt_t() : MENU_STRUCT({(char *)"Halt system for N ms"}) {
//     this->commands.push_back((char *)"delay");
//     this->commands.push_back((char *)"sleep");
//     this->commands.push_back((char *)"halt");
//     this->hidden = true;
//   }
//   void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
//     uint32_t value = strtol(arg, nullptr, 10);
//     eMenu.v().p("delay(").p(value).p(")...").pln();
//     delay(value);
//   }
// }menu_halt;

/*
MENU: HELP
*/
// namespace MENU {
struct express_console_menu::menu_help_t : MENU_STRUCT {
  express_console_menu *c;
  menu_help_t() : MENU_STRUCT({(char *)"Displays a list of the available commands."}) {
    this->commands.push_back((char *)"?");
    this->commands.push_back((char *)"help");
    // eMenu.MENU_GLOBAL_VECTOR.push_back(this);
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.printTable(TABLE_HEADER_START);
    eMenu.printTable(TABLE_DATA, "Command", "Description");
    eMenu.printTable(TABLE_HEADER_END);
    eMenu.printTable(TABLE_ROW);
    for (auto &item : eMenu.MENU_BUFFER_VECTOR) {
      char cmds[30] = "\0";
      if (item->hidden == false) {
        if (item->commands.size() >= 2) {
          sprintf(cmds, "%s - %s", item->commands[0], item->commands[1]);
        } else {
          sprintf(cmds, "    %s", item->commands[0]);
        }
        eMenu.printTable(TABLE_DATA, cmds, item->help);
      }
    }
    eMenu.printTable(TABLE_ROW);
    for (auto &item : eMenu.MENU_GLOBAL_VECTOR) {
      char cmds[30] = "\0";
      if (item->hidden == false) {
        if (item->commands.size() >= 2) {
          sprintf(cmds, "%s - %s", item->commands[0], item->commands[1]);
        } else {
          sprintf(cmds, "    %s", item->commands[0]);
        }
        eMenu.printTable(TABLE_DATA, cmds, item->help);
      }
    }
    eMenu.printTable(TABLE_END);
  }
} menu_help;
// };  // namespace MENU

/*
MENU: EXIT
*/
struct express_console_menu::menu_exitSubMenu_t : MENU_STRUCT {
  menu_exitSubMenu_t() : MENU_STRUCT({(char *)"return from sub menu"}) {
    this->commands.push_back((char *)"*");
    this->commands.push_back((char *)"exit");
  }
  void callback(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) override {
    eMenu.enterSubMenu(eMenu.MENU_MAIN_VECTOR, false);
  }
};  // menu_exitSubMenu;

void express_console_menu::enterSubMenu(vector<MENU_STRUCT *> vector_t, boolean insertExitCallback) {
  MENU_BUFFER_VECTOR = vector_t;
  // MENU_BUFFER_VECTOR.assign(vector_t.begin(), vector_t.end());

  menu_help_t *help = new menu_help_t();
  MENU_BUFFER_VECTOR.insert(MENU_BUFFER_VECTOR.begin(), help);
  if (insertExitCallback) {
    MENU_BUFFER_VECTOR.insert(MENU_BUFFER_VECTOR.begin(), new menu_exitSubMenu_t());
  }
  if (DEFAULT_SUB_MENU_HELP) {
    help->callback(nullptr, nullptr, 0);
  }
}

void express_console_menu::init(Print &printer) {
  _bufferLen = 0;
  _printer   = &printer;
  _stream    = (Stream *)&printer;

  for (uint8_t t = 2; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  delay(100);
  v().pln();
  v().pln();

  nvs_init();
}

void express_console_menu::nvs_init() {
#if (USE_NVS == true)
  debug(" ... ").pln();
  eNvs.init();
  esp_err_t err;

  //
  // verbosity_level
  err = eNvs.get("verbosity_level", &verbosity_level);
  if (err == ESP_OK) {
    eMenu._filter_level = (express_console::Level)verbosity_level;
  } else {
    error(__func__).pln("restoring defaults for verbosity_level");
    default_verbosity_level();
  }

  //
  // restart_conters
  eNvs.get("reboot_cnt", &reboot_counter);
  reboot_counter++;
  eNvs.set("reboot_cnt", &reboot_counter);
  eNvs.get("reboot_cnt_rst", &reboot_counter_resettable);
  reboot_counter_resettable++;
  eNvs.set("reboot_cnt_rst", &reboot_counter_resettable);

  debug("... done").pln();
#else
  verbosity_level = DEFAULT_VERBOSITY_LEVEL;
  setLevel(DEFAULT_VERBOSITY_LEVEL);
#endif
}

void express_console_menu::update() {
  if (!hasFinishedInit) {
    eMenu.enterSubMenu(eMenu.MENU_MAIN_VECTOR, false);
    hasFinishedInit = true;
  }
  while (_stream->available()) {
    char in = _stream->read();

    if (_bufferLen == 0 && in == ' ') {
      // trim
    } else if (in == '\r' || in == '\n') {
      if (in == '\r') {
        if (eMenu._filter_level > Level::p) {
          _printer->write('\n');
        }
      } else if (_bufferLen == 0) {
        // received "\n" alone: ignore it
        break;
      }

      // finish command
      _buffer[_bufferLen] = '\0';

// echo the command back
#if (DEFAULT_MENU_ECHO == true)
      // @TODO: save echo in nvs
      if (eMenu._filter_level > Level::p) {
        v().pln().p("-> ").pln(_buffer);
      }
#endif

      // process it
      if (_bufferLen > 0) {
        processCommand(_buffer);
      }

      // start again
      _bufferLen = 0;

      break;
    } else if ((in == 0x7F) || (in == '\b')) {
      // backspace
      if (_bufferLen > 0) {
        if (eMenu._filter_level > Level::p) {
          --_bufferLen;
          _printer->write("\b \b", 3);
        }
      }
    } else if (_bufferLen < CONSOLE_BUFFER_SIZE - 1) {
      _buffer[_bufferLen++] = in;
    }
  }
}

void express_console_menu::processCommand(char *_ptr) {
  char *ptr = _ptr;
  char *arg = nullptr;
  while (*ptr != '\0') {
    if (*ptr == ' ' || *ptr == '=') {
      *ptr++ = '\0';
      break;
    }
    ++ptr;
  }
  // trim whitespace after command
  while (*ptr == ' ' || *ptr == '=') {
    ++ptr;
  }

  arg = ptr;
  if (arg[0] == '"') {
    String str         = arg;
    uint16_t start_pos = str.indexOf('"') + 1;
    uint16_t end_pos   = str.indexOf('"', start_pos);
    String value       = str.substring(start_pos, end_pos);
    String then        = str.substring(end_pos + 1, -1);
    then.trim();
    processFunction(_ptr, value.c_str(), strlen(value.c_str()));
    if (strlen(then.c_str()) > 0) {
      processCommand((char *)then.c_str());
    } else {
      return;
    }
  } else {
    processFunction(_ptr, arg, strlen(arg));
  }
}

void express_console_menu::processFunction(const char *cmd = nullptr, const char *arg = nullptr, const uint8_t length = 0) {
  debug(__func__).p("cmd: ").p(cmd).p(", length: ").p(length).p(", arg: ").pln(arg);

  boolean matched = false;
  for (auto &item : MENU_BUFFER_VECTOR) {
    if (matched == true) {
      break;
    }
    for (auto &command : item->commands) {
      if (strcmp(command, cmd) == 0) {
        matched = true;
        item->callback(cmd, arg, length);
        break;
      }
    }
  }
  for (auto &item : MENU_GLOBAL_VECTOR) {
    if (matched == true) {
      break;
    }
    for (auto &command : item->commands) {
      if (strcmp(command, cmd) == 0) {
        matched = true;
        item->callback(cmd, arg, length);
        break;
      }
    }
  }
  if (matched == false) {
    error(__func__).p("invalid option - ").pln(cmd);
  }
}
