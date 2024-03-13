#include "asyncMqtt.h"

#include <ArduinoLog.h>

#include "communications/osc.h"
#include "lights/Animation.h"
#include "lights/Pixels.h"
#include "Morphose.h"
#include "Logger.h"
#include "Network.h"

namespace mqtt {

#define WIFI_SSID "yourSSID"
#define WIFI_PASSWORD "yourpass"

#define MQTT_HOST IPAddress(192, 168, 0, 200)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


// RTLS IDs.
static const char *ROBOT_RTLS_IDS[N_ROBOTS] = { "1a1e", "0f32", "5b26" };
static char ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS][32];  // Internal use to keep full MQTT addresses.
static char ROBOT_CUSTOM_MQTT_ADDRESS[32];

#if (ROBOT_ID == 110)
const char* cid = "robot1";
#elif (ROBOT_ID == 120)
const char* cid = "robot2";
#elif (ROBOT_ID == 130)
const char* cid = "robot3";
#elif (ROBOT_ID == 140)
const char* cid = "robot4";
#endif


uint16_t mqttRobotLocations[N_ROBOTS];
uint16_t animId;

void connectToMqtt() {
  osc::debug("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    char buffer[64];
    sprintf(buffer,"[WiFi-event] event: %d\n", event);
    osc::debug(buffer);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        sprintf(buffer,"WiFi connected. IP address:%d.%d.%d.%d\n", WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
        osc::debug(buffer);

        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        osc::debug("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
  char buffer[64];
  sprintf(buffer, "Connected to MQTT.\n Session present: %d\n",sessionPresent);
  osc::debug(buffer);

  for (int i=0; i < N_ROBOTS; i++) {
    // Create subscription.
    sprintf(ROBOT_RTLS_MQTT_ADDRESS[i], "dwm/node/%s/uplink/location", ROBOT_RTLS_IDS[i]);
    mqttRobotLocations[i] = mqttClient.subscribe(ROBOT_RTLS_MQTT_ADDRESS[i], 2);

   osc::debug(ROBOT_RTLS_MQTT_ADDRESS[i]) ;
   sprintf(buffer,"\nSubscribing at QoS 2, packetId: %d\n",mqttRobotLocations[i]);
   osc::debug(buffer);
  }

  sprintf(ROBOT_CUSTOM_MQTT_ADDRESS, "morphoses/robot1/animation", morphose::name);
  animId = mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS, 2);
  osc::debug(ROBOT_CUSTOM_MQTT_ADDRESS) ;
  sprintf(buffer,"\nSubscribing at QoS 2, packetId: %d\n",animId);
   osc::debug(buffer);
  
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
 osc::debug("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  char buffer[64];
  sprintf(buffer, "Subscribe acknowledged.\n packetId: %d, qos: %d\n",packetId,qos);
  osc::debug(buffer);
}

void onMqttUnsubscribe(uint16_t packetId) {
  char buffer[64];
  sprintf(buffer, "Unsubscribe acknowledged.\n packetId: %d\n",packetId);
  osc::debug(buffer);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char buffer[1024];
  sprintf(buffer, "Message received.\n topic: %s\n payload: %s \n qos: %d\n dup: %d\n retain: %d\n len: %zu\n index: %zu\n total: %zu\n",topic,payload,properties.qos,properties.dup,properties.retain,len,index,total);
  osc::debug(buffer);
  //TODO(EtiENNE): Handle message
}

void onMqttPublish(uint16_t packetId) {
  char buffer[64];
  sprintf(buffer, "Publish acknowledged.\n packetId: %d\n",packetId);
  osc::debug(buffer);
  
}

void initialize() {
  Serial.println("-------------MQTT initialization-------------");
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(network::initialize));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  network::initialize();
  Serial.println("-------------MQTT initialization done-------------");
}

}   // namespace mqtt
