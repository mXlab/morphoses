#include "asyncMqtt.h"



#include "lights/Animation.h"
#include "lights/Pixels.h"
#include "Morphose.h"
#include "hardware/Engine.h"
#include "hardware/IMU.h"

#include "Network.h"
#include <ArduinoJson.h>
#include <VectorXf.h>

namespace mqtt {

#define MQTT_HOST IPAddress(192, 168, 0, 200)
#define MQTT_PORT 1883

AsyncMqttClient client;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


// RTLS IDs.
static const char *ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS] = {"dwm/node/019f/uplink/location", //1a1e
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
    REBOOT,
    IDLE,
    PING
};


#if (ROBOT_ID == 1)
const  char* cid = "robot1";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS[11] ={"morphoses/robot1/steer",
                                              "morphoses/robot1/speed",
                                              "morphoses/robot1/power",
                                              "morphoses/robot1/animation",
                                              "morphoses/robot1/get-data",
                                              "morphoses/robot1/nav",
                                              "morphoses/robot1/calib",
                                              "morphoses/robot1/stream",
                                              "morphoses/robot1/reboot",
                                              "morphoses/robot1/idle",
                                              "morphoses/robot1/ping"};
                                            
const char* debugAddress  = "morphoses/robot1/debug";
const char* temperatureAddress  = "morphoses/robot1/temperature";
const char* batteryAddress  = "morphoses/robot1/battery";
const char* batteryCriticalAddress  = "morphoses/robot1/batteryCritical";
const char* ackAddress  = "morphoses/robot1/acknowledge";

#elif (ROBOT_ID == 2)
const char* cid = "robot2";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS[11] ={"morphoses/robot2/steer",
                                            "morphoses/robot2/speed",
                                            "morphoses/robot2/power",
                                            "morphoses/robot2/animation",
                                            "morphoses/robot2/get-data",
                                            "morphoses/robot2/nav",
                                            "morphoses/robot2/calib",
                                            "morphoses/robot2/stream",
                                            "morphoses/robot2/reboot",
                                            "morphoses/robot2/idle",
                                              "morphoses/robot2/ping"};
const char* debugAddress  = "morphoses/robot2/debug";
const char* temperatureAddress  = "morphoses/robot2/temperature";
const char* batteryAddress  = "morphoses/robot2/battery";
const char* batteryCriticalAddress  = "morphoses/robot2/batteryCritical";
const char* ackAddress  = "morphoses/robot2/acknowledge";


#elif (ROBOT_ID == 3)
const char* cid = "robot3";
static const char *ROBOT_CUSTOM_MQTT_ADDRESS[11] ={"morphoses/robot3/steer",
                                            "morphoses/robot3/speed",
                                            "morphoses/robot3/power",
                                            "morphoses/robot3/animation",
                                            "morphoses/robot3/get-data",
                                            "morphoses/robot3/nav",
                                            "morphoses/robot3/calib",
                                            "morphoses/robot3/stream",
                                            "morphoses/robot3/reboot",
                                            "morphoses/robot3/idle",
                                            "morphoses/robot3/ping"};

const char* debugAddress  = "morphoses/robot3/debug";
const char* temperatureAddress  = "morphoses/robot3/temperature";
const char* batteryAddress  = "morphoses/robot3/battery";
const char* batteryCriticalAddress  = "morphoses/robot3/batteryCritical";
const char* ackAddress  = "morphoses/robot3/acknowledge";

#endif


uint16_t mqttRobotLocations[N_ROBOTS];
uint16_t animId;
int qos = 1;

void sendAck(){
  const char* msg = "1";
  client.publish(ackAddress, qos, true, msg);
}

void sendBatteryCritical(){
  const char* msg = "1";
  client.publish(batteryCriticalAddress, qos, true, msg);
}

void sendTemperature(const char* msg){
  client.publish(temperatureAddress, qos, true, msg);
}

void connectToMqtt() {
  mqtt::debug("Connecting to MQTT...");
  client.connect();
}

void sendBatteryVoltage(float v){
  char buff[16];
            sprintf(buff, "%.2F", v);
            mqtt::client.publish(mqtt::batteryAddress,2,true,buff);
}

void WiFiEvent(WiFiEvent_t event) {
    char buffer[64];
    sprintf(buffer,"[WiFi-event] event: %d\n", event);
    //mqtt::debug(buffer);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        sprintf(buffer,"WiFi connected. IP address:%d.%d.%d.%d\n", WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
        mqtt::debug(buffer);

        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        mqtt::debug("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
  char buffer[64];
  sprintf(buffer, "Connected to MQTT.\n Session present: %d\n",sessionPresent);
  mqtt::debug(buffer);

  for (int i=0; i < N_ROBOTS; i++) {
    // Create subscription.

    mqttRobotLocations[i] = client.subscribe(ROBOT_RTLS_MQTT_ADDRESS[i], qos);


   mqtt::debug(ROBOT_RTLS_MQTT_ADDRESS[i]) ;
   sprintf(buffer,"\nSubscribing at QoS 2, packetId: %d\n",mqttRobotLocations[i]);
   mqtt::debug(buffer);
  }



  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[ANIMATION], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[STEER], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[SPEED], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[POWER], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[GET_DATA], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[NAV], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[CALIB], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[STREAM], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[REBOOT], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[IDLE], qos);
  client.subscribe(ROBOT_CUSTOM_MQTT_ADDRESS[PING], qos);


  
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
 mqtt::debug("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t _qos) {
  char buffer[64];
  sprintf(buffer, "Subscribe acknowledged.\n packetId: %d, qos: %d\n",packetId,_qos);
  mqtt::debug(buffer);
}

void onMqttUnsubscribe(uint16_t packetId) {
  char buffer[64];
  sprintf(buffer, "Unsubscribe acknowledged.\n packetId: %d\n",packetId);
  mqtt::debug(buffer);
}

#define MQTT_MESSAGE_MAX_SIZE 512
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  static char realPayload[MQTT_MESSAGE_MAX_SIZE+1];

  // Manage potential erroneous messages.
  if (total == 0 || total > MQTT_MESSAGE_MAX_SIZE)
    return;

  // Copy the current chunk into the full payload
  memcpy(realPayload + index, payload, len);

  // Check if message is complete.
  if (index + len == total) {
    
    // Convert to string.
    realPayload[total] = '\0';

    // char buffer[256];
    // sprintf(buffer, "Message received.\n topic: %s\n payload: %s \n qos: %d\n dup: %d\n retain: %d\n len: %zu\n index: %zu\n total: %zu\n",topic,realPayload,properties.qos,properties.dup,properties.retain,len,index,total);
    // mqtt::debug(buffer);
    
    // Call appropriate callback.

    if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[0]) == 0){
      //Serial.println("Position robot1");
      callbacks::handlePosition(1, realPayload);
    } else if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[1]) == 0){
      //Serial.println("Position robot2");
      callbacks::handlePosition(2, realPayload);
    } else if(strcmp(topic, ROBOT_RTLS_MQTT_ADDRESS[2]) == 0){
      //Serial.println("Position robot3");
      callbacks::handlePosition(3, realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[ANIMATION]) == 0){
      //Serial.println("animation");
      callbacks::handleAnimation(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[STEER]) == 0){
      //Serial.println("steer");
      callbacks::handleSteer(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[SPEED]) == 0){
      //Serial.println("speed");
      callbacks::handleSpeed(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[POWER]) == 0){
      //Serial.println("power");
      callbacks::handlePower(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[GET_DATA]) == 0){
      //Serial.println("get data");
      callbacks::handleGetData(realPayload);    
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[NAV]) == 0){
      //Serial.println("nav");
      callbacks::handleNav(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[CALIB]) == 0){
      //Serial.println("calib");
      callbacks::handleCalib(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[STREAM]) == 0){
      //Serial.println("stream");
      callbacks::handleStream(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[REBOOT]) == 0){
      //Serial.println("reboot");
      callbacks::handleReboot(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[IDLE]) == 0){
      //Serial.println("idle");
      callbacks::handleIdle(realPayload);
    } else if(strcmp(topic, ROBOT_CUSTOM_MQTT_ADDRESS[PING]) == 0){
      //Serial.println("idle");
      callbacks::handlePing(realPayload);
    }

  }
}



void initialize() {

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(network::initialize));

  WiFi.onEvent(WiFiEvent);

  client.onConnect(onMqttConnect);
  client.onDisconnect(onMqttDisconnect);
  client.onSubscribe(onMqttSubscribe);
  client.onUnsubscribe(onMqttUnsubscribe);
  client.onMessage(onMqttMessage);  
  client.setServer(MQTT_HOST, MQTT_PORT);

  network::initialize();
  
}

void debug(const char *_msg) {
  #if defined(MORPHOSE_DEBUG)
  Serial.println(_msg);
  #endif
  client.publish(debugAddress, 1, true, _msg);
}
namespace callbacks {
  Vec2f robotPositions[N_ROBOTS];
void handlePosition(int robot, char* data) {

  Vec2f newPosition;
  
  JsonDocument doc;
  
  DeserializationError error = deserializeJson(doc, data);

  // Test if parsing succeeds.
  if (error) {
    mqtt::debug(data);
    char buffer[256];
    sprintf(buffer,"handlePosition() deserializeJson() failed: %s\n", error.c_str());
    mqtt::debug(buffer);
    return;
  }


  JsonObject position = doc["location"];
  
  if ((int)position["quality"] > 50) {
    // Set current position.

    //handling case where we receive "NaN" values
    auto positionXStr = position["x"].as<const char*>();
    auto positionYStr = position["y"].as<const char*>();
    Serial.println(positionXStr);
    Serial.println(positionYStr);
    if(strcmp(positionXStr, "NaN") == 0 || strcmp(positionYStr, "NaN") == 0) {
      mqtt::debug("received NaN position from decawave anchors");
      return;
    }

    newPosition = Vec2f((float)double(position["x"]), (float)double(position["y"]));
    
    // buffer all robot positions
    robotPositions[robot-1].set(newPosition);

     if (robot == morphose::id) {
        morphose::setCurrentPosition(newPosition);
     }
  }

}

void handleAnimation(char* data){
    // Parse location.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    // Test if parsing succeeds.
    if (error) {
      char buffer[256];
      sprintf(buffer,"handleAnimation() deserializeJson() failed: %s\n", error.c_str());
      mqtt::debug(buffer);
      return;
    }
    
    // JSONVar animationData = JSON.parse(data);

    JsonArray _baseColor = doc["base"];
    JsonArray _altColor  = doc["alt"];


  if (animations::lockMutex()) {
    animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
    
    animations::currentAnimation().setBaseColor(_baseColor[0].as<int>(), _baseColor[1].as<int>(), _baseColor[2].as<int>());
    animations::currentAnimation().setAltColor(_altColor[0].as<int>(),  _altColor[1].as<int>(),  _altColor[2].as<int>());

    animations::currentAnimation().setNoise(doc["noise"].as<float>());
    animations::currentAnimation().setPeriod(doc["period"].as<float>());
    animations::currentAnimation().setType((animations::AnimationType)doc["type"].as<int>());
    animations::currentAnimation().setRegion((pixels::Region)doc["region"].as<int>() );

    animations::beginTransition();  // start transition
    animations::unlockMutex();
  }
}



void handleSteer(char* payload){
    const float val = atof(payload);

    char buffer[32];
    sprintf(buffer,"Set steer to %.2F\n", val);
    //mqtt::debug(buffer);

    motors::setEngineSteer(val);
  }

  
void handleSpeed(char* payload){
    const float val = atof(payload);

    char buffer[32];
    sprintf(buffer,"Set speed to %.2F\n", val);
    //mqtt::debug(buffer);

    motors::setEngineSpeed(val);
}

void handleIdle(char* payload){
    const float val = atof(payload);

    char buffer[32];
    sprintf(buffer,"Set idle to %.2F\n", val);
    mqtt::debug(buffer);

    morphose::setIdle(val);
}

void handlePower(char* payload){
    const int power = atoi(payload);

    char buffer[32];
    sprintf(buffer,"Set power to %d\n", power);
    //mqtt::debug(buffer);

    motors::setEnginePower(power);
  }
 
void handleGetData(char* payload){
  morphose::sendData();
}  
  
void handleNav(char* payload){

  char *result = strstr(payload, "stop");
  if(result != NULL){
    mqtt::debug("Stop navigation");
    morphose::navigation::stopHeading();
    return;
  }else{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error) {
      char buffer[256];
      sprintf(buffer,"handleNav() deserializeJson() failed: %s\n", error.c_str());
      mqtt::debug(buffer);
      return;
    }

    const char* action = doc["action"].as<const char*>();

    if (strcmp(action, "stop") == 0) {
      morphose::navigation::stopHeading();      
    }

    else if (strcmp(action, "start") == 0) {
      float speed = doc["speed"].as<float>();
      if (doc.containsKey("heading"))
        morphose::navigation::startHeading(speed, doc["heading"].as<float>());
      else
        morphose::navigation::startHeading(speed);
    }
    //todo(add fail safe here)
  }
}
 
void handleCalib(char* payload){
  char *result = strstr(payload,"begin");
  if(result != NULL){
    mqtt::debug("Start IMUS calibration");
    imus::beginCalibration();
    return;
  }

  result = strstr(payload,"end");
  if(result != NULL){
    mqtt::debug("Stop IMUS calibration");
    imus::endCalibration();
    return;
  }

  result = strstr(payload,"save");
  if(result != NULL){
    mqtt::debug("Saving imus calibration");
    imus::saveCalibration();
    return;
  }

}
  
void handleStream(char* payload){
    const int stream = atoi(payload);
    
    if (stream) {
      mqtt::debug("Starting stream");
      morphose::stream = 1;
    }else{
      mqtt::debug("Stopping stream");
      morphose::stream = 0;
    } 
  }


void handleReboot(char* payload){
  ESP.restart();
}

void handlePing(char* payload){
  mqtt::sendAck();
}

}

}   // namespace mqtt
