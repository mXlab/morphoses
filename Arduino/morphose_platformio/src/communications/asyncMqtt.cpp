#include "asyncMqtt.h"

#include <ArduinoLog.h>

#include "communications/osc.h"
#include "lights/Animation.h"
#include "lights/Pixels.h"
#include "Morphose.h"
#include "hardware/Engine.h"
#include "hardware/IMU.h"
#include "Logger.h"
#include "Network.h"
#include <Arduino_JSON.h>
#include <VectorXf.h>
namespace mqtt {

#define MQTT_HOST IPAddress(192, 168, 0, 200)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


// RTLS IDs.
static const char *ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS] = {"dwm/node/1a1e/uplink/location",
                                                  "dwm/node/0f32/uplink/location",
                                                  "dwm/node/5b26/uplink/location"};


enum {
    STEER,
    SPEED,
    POWER,
    ANIMATION,
    GET_DATA,
    NAV,
    CALIB,
    STREAM,
    REBOOT
};


#if (ROBOT_ID == 1)
const  char* cid = "robot1";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS ="morphoses/robot1";
#elif (ROBOT_ID == 2)
const char* cid = "robot2";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS ="morphoses/robot2";
#elif (ROBOT_ID == 3)
const char* cid = "robot3";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS ="morphoses/robot3";
#elif (ROBOT_ID == 4)
const char* cid = "robot4";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS[9] ={"morphoses/robot1/steer",
                                            "morphoses/robot1/speed",
                                            "morphoses/robot1/power",
                                            "morphoses/robot1/animation",
                                            "morphoses/robot1/get/data",
                                            "morphoses/robot1/nav",
                                            "morphoses/robot1/calib",
                                            "morphoses/robot1/stream",
                                            "morphoses/robot1/reboot",

                                            };
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
    mqttRobotLocations[i] = mqttClient.subscribe(ROBOT_RTLS_MQTT_ADDRESS[i], 2);

   osc::debug(ROBOT_RTLS_MQTT_ADDRESS[i]) ;
   sprintf(buffer,"\nSubscribing at QoS 2, packetId: %d\n",mqttRobotLocations[i]);
   osc::debug(buffer);
  }

  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[ANIMATION], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[STEER], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[SPEED], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[POWER], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[GET_DATA], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[NAV], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[CALIB], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[STREAM], 2);
  mqttClient.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[REBOOT], 2);

  
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
  // char buffer[1024];
  // sprintf(buffer, "Message received.\n topic: %s\n payload: %s \n qos: %d\n dup: %d\n retain: %d\n len: %zu\n index: %zu\n total: %zu\n",topic,payload,properties.qos,properties.dup,properties.retain,len,index,total);
  // osc::debug(buffer);
  
  
  if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[0]) == 0){
    //Serial.println("Position robot1");
    callbacks::handlePosition(1, payload);
  }else if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[1]) == 0){
    //Serial.println("Position robot2");
    callbacks::handlePosition(2, payload);
  }else if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[2]) == 0){
    //Serial.println("Position robot3");
    callbacks::handlePosition(3, payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[ANIMATION]) == 0){
    //Serial.println("animation");
    callbacks::handleAnimation(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[STEER]) == 0){
    Serial.println("steer");
    callbacks::handleSteer(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[SPEED]) == 0){
    Serial.println("speed");
    callbacks::handleSpeed(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[POWER]) == 0){
    Serial.println("power");
    callbacks::handlePower(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[GET_DATA]) == 0){
    Serial.println("get data");
    callbacks::handleGetData(payload);    
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[NAV]) == 0){
    Serial.println("nav");
    callbacks::handleNav(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[CALIB]) == 0){
    Serial.println("calib");
    callbacks::handleCalib(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[STREAM]) == 0){
    Serial.println("stream");
    callbacks::handleStream(payload);
  }else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[REBOOT]) == 0){
    Serial.println("reboot");
    callbacks::handleReboot(payload);
  }
  

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

namespace callbacks {
  Vec2f robotPositions[N_ROBOTS];
void handlePosition(int robot, char* data) {
    // Parse location.
  //Serial.printf("Handle position %d\n", robot); 
  Vec2f newPosition;
  JSONVar location = JSON.parse(data);
  JSONVar position = location["position"];
  if ((int)position["quality"] > 50) {
    // Set current position.
    //Serial.println("Quality > 50");
    newPosition = Vec2f((float)double(position["x"]), (float)double(position["y"]));
    
    // buffer all robot positions
    robotPositions[robot-1].set(newPosition);

     if (robot == morphose::id) {
        morphose::setCurrentPosition(newPosition);
     }
    

    // TODO(Etienne): Verify with Sofian why whe send a bundle here. There was some commented code in old version here
    //osc::sendBundle();
  }

}

void handleAnimation(char* data){
    // Parse location.
    JSONVar animationData = JSON.parse(data);
    JSONVar _baseColor = animationData["base"];
    JSONVar _altColor  = animationData["alt"];


  if (animations::lockMutex()) {
    animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
    
    animations::currentAnimation().setBaseColor(int(_baseColor[0]), int(_baseColor[1]), int(_baseColor[2]));

    animations::currentAnimation().setAltColor(int(_altColor[0]),  int(_altColor[1]),  int(_altColor[2]));

    animations::currentAnimation().setNoise((float)  double(animationData["noise"]));
    animations::currentAnimation().setPeriod((float) double(animationData["period"]));
    animations::currentAnimation().setType((animations::AnimationType)int(animationData["type"]));
    animations::currentAnimation().setRegion((pixels::Region)int(animationData["region"]) );
    animations::beginTransition();  // start transition
    animations::unlockMutex();
  }
}



void handleSteer(char* payload){
  Serial.println(payload);
    const float val = atof(payload);
    Serial.printf("Set steer to %.2F\n", val);
    motors::setEngineSteer(val);
  }

  
void handleSpeed(char* payload){
    Serial.println(payload);
    const float val = atof(payload);
    Serial.printf("Set speed to %.2F\n", val);
    motors::setEngineSpeed(val);
}

void handlePower(char* payload){
    const int power = atoi(payload);
    Serial.printf("Set power to %d\n", power);
    motors::setEnginePower(power);
  }
 
void handleGetData(char* payload){
  Serial.println("Get data");
  morphose::sendData();
}  
  
void handleNav(char* payload){

  char *result = strstr(payload,"stop");
  if(result != NULL){
    Serial.println("Stop navigation");
    morphose::navigation::stopHeading;
    return;
  }else{
    Serial.println("Start navigation");
    float speed, heading;
    sscanf(payload, "%F %F", &speed, &heading);
    //todo(add fail safe here)
    morphose::navigation::startHeading(speed, heading);

  }
}
 
void handleCalib(char* payload){
  char *result = strstr(payload,"begin");
  if(result != NULL){
    Serial.println("Start IMUS calibration");
    imus::beginCalibration();
    return;
  }

  result = strstr(payload,"end");
  if(result != NULL){
    Serial.println("Stop IMUS calibration");
    imus::endCalibration();
    return;
  }

  result = strstr(payload,"save");
  if(result != NULL){
    Serial.println("Saving imus calibration");
    imus::saveCalibration();
    return;
  }

}
  
void handleStream(char* payload){
    const int stream = atoi(payload);
    
    if (stream) {
      Serial.println("Starting stream");
      morphose::stream = 1;
    }else{
      Serial.println("Stopping stream");
      morphose::stream = 0;
    } 
  }


void handleReboot(char* payload){
  ESP.restart();
}

}

}   // namespace mqtt
