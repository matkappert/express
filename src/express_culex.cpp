/*
    @file       express_culex.cpp
    @author     matkappert
    @repo       github.com/matkappert/express
    @date       11/07/21
*/

#include "express_culex.h"

#include "Settings.h"

express_culex eCulex;  // global-scoped variable

// void express_culex::STATIC_DATA_EVENT(const char *topic, byte *payload, unsigned int length) {
//   eCulex.DATA_EVENT(topic, payload, length);
// }

void express_culex::DATA_EVENT(const char *topic, byte *payload, unsigned int length) {
  DeserializationError error = deserializeJson(doc, (const byte *)payload, length);
  if (error) {
    eMenu.error(__func__).p("deserializeJson").pln(error.f_str());
    return;
  }
  // uint64_t timestamp = doc["timestamp"].as<unsigned long long>(); // TODO update system time with NTP
  JsonArray arr = doc["metrics"].as<JsonArray>();
  for (JsonVariant metric : arr) {
    JsonObject metricObject = metric.as<JsonObject>();
    const char *name        = metricObject["name"].as<const char *>();
    const char *type        = metricObject["dataType"].as<const char *>();

    eMenu.debug(__func__).p("searching for transport:").p(name).p(" ... ");
    for (auto &transport : CULEX_TRANSPORT_VECTORS) {
      if (strcmp(name, transport->name) == 0) {
        eMenu.vvvv().pln("FOUND");
        transport->callback(metricObject, type);
        break;
      }
    }
  }
}

// struct trans_DATA_t : CULEX_TRANSPORT {
//   trans_DATA_t() : CULEX_TRANSPORT({(char *)"DATA", (EXPRESS_TYPE_ENUM)Boolean}) {}
// } trans_DATA;

struct trans_bdSeq_t : CULEX_TRANSPORT {
  trans_bdSeq_t() : CULEX_TRANSPORT({(char *)"bdSeq", (EXPRESS_TYPE_ENUM)UInt8}) {
    this->value_uint8 = &eCulex._message_bdSeq;
  }
  void callback(JsonObject objectValue, const char *type) override {
    eMenu.info(__func__).pln();
  }
} trans_bdSeq;

struct trans_system_coreVoltage_t : CULEX_TRANSPORT {
  float coreVoltage = 0.00;
  trans_system_coreVoltage_t() : CULEX_TRANSPORT({(char *)"system/coreVoltage", (EXPRESS_TYPE_ENUM)Float}) {
    this->server_permissions = READ_PERMISSION;
    this->user_permissions   = READ_PERMISSION;
    this->value_float        = &coreVoltage;
    this->timer              = 5000;
  }
  boolean update() {
    coreVoltage = eUtil.getVoltage();
    return true;
  }
} trans_system_coreVoltage;

void express_culex::init(WiFiClass *WiFi) {
  _WiFi = WiFi;
  generateTopics();
  //   postTopics();
}

void express_culex::generateTopic(char *ptr, const char *method) {
  //*  [NAMESPACE]/[USER_TOKEN]/[DEVICE_ID]/[METHOD]
  sprintf(ptr, "%s/%s/%s/%s", DEFAULT_CULEX_NAMESPACE, DEFAULT_CULEX_USER_TOKEN, DEFAULT_CULEX_DEVICE_ID, method);
  eMenu.debug(__func__).p("method: ").pln(method);
}

void express_culex::generateTopics() {
  //*  [NAMESPACE]/[USER_TOKEN]/[DEVICE_ID]/#
  for (auto &transport : CULEX_TRANSPORT_VECTORS) {
    generateTopic(transport->topic, transport->name);
    // sprintf(transport->topic, "%s/%s/%s/%s", DEFAULT_CULEX_NAMESPACE, DEFAULT_CULEX_USER_TOKEN, DEFAULT_CULEX_DEVICE_ID, transport->name);
    eMenu.debug(__func__).p("name: ").p(transport->name).p(", topic: ").pln(transport->topic);
  }
}

void express_culex::postTopics() {
  for (auto &transport : CULEX_TRANSPORT_VECTORS) {
    generatePayload(transport);
  }
}

void express_culex::birth() {
  char payload[CULEX_PAYLOAD_SIZE];
  StaticJsonDocument<CULEX_PAYLOAD_SIZE> doc;
  doc["timestamp"] = millis();
  // JsonArray metrics = doc.to<JsonArray>();
  JsonArray metrics = doc.createNestedArray("metrics");

  for (auto &transport : CULEX_TRANSPORT_VECTORS) {
    JsonObject metric     = metrics.createNestedObject();
    metric["name"]        = transport->name;
    metric["dataType"]    = EXPRESS_TYPE_CHAR[transport->type];
    metric["value"]       = valueToBuffer(transport);
    metric["permissions"] = ((uint16_t)(((uint8_t)transport->user_permissions << 8) | (uint8_t)transport->server_permissions));
  }
  
  doc["seq"] = message_seq();
  size_t n   = serializeJson(doc, payload);
  eMenu.debug(__func__).pln("\npayload:");
  serializeJsonPretty(doc, Serial);

  char topic[CULEX_TOPIC_SIZE];
  generateTopic(topic, "BIRTH");
  culexClient.publish(topic, payload, n);
}

void express_culex::generatePayload(CULEX_TRANSPORT *transport) {
  char payload[256];
  StaticJsonDocument<256> doc;
  doc["name"] = transport->name;
  // doc["timestamp"] = millis();
  // doc["dataType"] = EXPRESS_TYPE_CHAR[transport->type];
  doc["value"] = valueToBuffer(transport);

  //* permissions = [USER, SERVER] 0x0F0F; uint16_t wd = ((uint16_t)d2 << 8) | d1
  // doc["permissions"] = ((uint16_t)(((uint8_t)transport->user_permissions << 8) | (uint8_t)transport->server_permissions));

  size_t n = serializeJson(doc, payload);
  eMenu.debug(__func__).p("name: ").p(transport->name).p("topic: \n").p(transport->topic).p("\npayload: \n").pln(payload);
  culexClient.publish(transport->topic, payload, n);
}

void express_culex::update() {
  // TODO if ( (wifiClient.connected()) && (WiFi.status() == WL_CONNECTED) )
  if (!culexClient.loop() && _WiFi->isConnected()) {  // if culex is NOT connected && wifi is connected
    if (millis() >= connection_timeout_last + connection_timeout_wait) {
      if (!connect()) {  // if FAILED to connect
        connection_timeout_last = millis();
        if (connection_timeout_wait <= 32000) {
          //  multiply connection timeout eg. 500ms, 1000ms, 2000ms ... 32000ms, 64000ms.
          connection_timeout_wait = connection_timeout_wait * 2;
        }
        eMenu.info(__func__).p("connection timeout: ").pln(connection_timeout_wait);
      }
    }
  } else if (this->isConnected && (culexClient.state() != 0 || !_WiFi->isConnected())) {  // if culex is NOT connected && wifi is NOT connected
    this->isConnected = false;
    eMenu.error(__func__).p("connection terminated!").pln();
  } 
   if(this->isConnected){
    for (auto &transport : CULEX_TRANSPORT_VECTORS) {
      if (transport->timer > 0) {
        if (millis() - transport->timer_last >= transport->timer) {
          transport->timer_last = millis();
          if (transport->update()) {
            //* publish
            eCulex.generatePayload(transport);
          }
        }
      }
    }
  }
}

void express_culex::disconnect() {
  eMenu.error(__func__).pln();
  culexClient.disconnect();
}

boolean express_culex::connect() {
  if (culexClient.state() != 0) {  // if NOT connected

    culexClient.disconnect();
    culexClient.setBufferSize((uint16_t)CULEX_PAYLOAD_SIZE);
    culexClient.setServer(DEFAULT_CULEX_SEVER_IP_ADDRESS, DEFAULT_CULEX_SERVER_PORT);
    culexClient.setKeepAlive(DEFAULT_CULEX_KEEPALIVE);
    culexClient.setSocketTimeout(DEFAULT_CULEX_SET_SOCKET_TIMEOUT);
    culexClient.setCallback(eCulex.STATIC_DATA_EVENT);

    if (culexClient.connect(PROJECT_NAME, DEFAULT_CULEX_USERNAME, DEFAULT_CULEX_PASSWORD, trans_bdSeq.topic, CULEX_LWT_QOS, CULEX_LWT_RETAIN, "{\"birth\":false}", CULEX_LWT_CLEAN_SESSION)) {
      this->isConnected = true;
      eMenu.info(__func__).p("connected").pln();
      connection_timeout_wait = CULEX_CONNECTION_WAIT;
      connection_timeout_last = 0;

      birth();

#if (USE_LED == true)
      hasErrorBecauseNoConnection = false;
      eLED.removeTask(2);
#endif
      return true;
    } else {
#if (USE_LED == true)
      if (!hasErrorBecauseNoConnection) {
        hasErrorBecauseNoConnection = true;
        eLED.addTask(2, TASK::ERROR, 2);
      }
#endif
      eMenu.error(__func__).p("connection failed! ").p(culexClient.state()).p(" ").pln(MQTT_CONNECTION_ERRORS_CHAR[culexClient.state() + 4]);
      return false;
    }
    return false;
  }
}

String express_culex::valueToBuffer(CULEX_TRANSPORT *transport) {
  char buffer[(20 * sizeof(char)) + 1];
  if (transport->type == Int8) {
    itoa(*transport->value_int8, buffer, 10);
  } else if (transport->type == Int16) {
    itoa(*transport->value_int16, buffer, 10);
  } else if (transport->type == Int32) {
    itoa(*transport->value_int32, buffer, 10);
  } else if (transport->type == UInt8) {
    utoa(*transport->value_uint8, buffer, 10);
  } else if (transport->type == UInt16) {
    utoa(*transport->value_uint16, buffer, 10);
  } else if (transport->type == UInt32) {
    utoa(*transport->value_uint32, buffer, 10);
  } else if (transport->type == Float) {
    dtostrf(*transport->value_float, 6, 6, buffer);
  } else if (transport->type == Double) {
    dtostrf(*transport->value_double, 6, 6, buffer);
  } else if (transport->type == Boolean) {
    sprintf(buffer, "%s", *transport->value_boolean ? "TRUE" : "FALSE");
  }
  return String(buffer);
}