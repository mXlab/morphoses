#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSCCALLBACKS_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSCCALLBACKS_H_


// #include "Comm.h"

#include "hardware/Engine.h"
#include "hardware/IMU.h"
// #include "Navigation.h"
#include "Network.h"
#include "Osc.h"
#include "lights/Pixels.h"
#include "lights/Animation.h"
#include "Logger.h"


namespace oscCallback {

void broadcast(OSCMessage& msg){
  bool state = osc::getArgAsBool(msg,0);

  osc::setBroadcast(state);
  char buff[32];
  sprintf(buff, "set broadcasting to %d", state);
  osc::debug(buff);
}

void bonjour(OSCMessage& msg) {
  // Collect last part of IP address and add it to the list of destinations.
  // byte destIP3 = osc::argIsNumber(msg, 0) ? osc::getArgAsInt(msg, 0) : udp.remoteIP()[3];
  byte destIP3 = network::udp.remoteIP()[3];

  char buff[32];
  sprintf(buff, "ip3: %d", destIP3);
  osc::debug(buff);
  // TODO(Etienne) : Verify if still needed
  network::addDestinationIPAddress(destIP3);
  osc::bundle.add("/bonjour").add(morphose::name).add(destIP3);
  osc::sendBundle();
}

void reboot(OSCMessage& msg) {
  osc::debug("Rebooting esp.");
  network::removeWifiEvents();
  ESP.restart();
}

void speed(OSCMessage& msg) {
  if (osc::argIsNumber(msg, 0)) {
    float val = osc::getArgAsFloat(msg, 0);
    char buff[64];
    sprintf(buff, "Set speed to %.2F", val);
    osc::debug(buff);
    motors::setEngineSpeed(val);
  }
}

void steer(OSCMessage& msg) {
  if (osc::argIsNumber(msg, 0)) {
    float val = osc::getArgAsFloat(msg, 0);
    char buff[64];
    sprintf(buff, "Set steer to %.2F", val);
    osc::debug(buff);
    motors::setEngineSteer(val);
  }
}

void getData(OSCMessage& msg) {
  osc::debug("get data");

  morphose::sendData();
  // TODO(Etienne): Verify if this work or if get data should just broadcast instead
  
  osc::sendBundle(network::udp.remoteIP(), network::outgoingPort);
  
}

void stream(OSCMessage& msg) {
  Serial.println("Set stream");
  if (osc::argIsNumber(msg, 0)) {
    bool stream = osc::getArgAsBool(msg, 0);
    
    // If new value, enable/disable OSC & wake/sleep IMU.
    
    if (stream) {
      osc::debug("Starting stream");
      morphose::stream = 1;
      imus::wake();
    }else{
      osc::debug("Stopping stream");
      morphose::stream = 0;
      imus::sleep();
    } 
  }
}

void power(OSCMessage& msg) {
  bool power = (osc::argIsNumber(msg, 0) ? osc::getArgAsBool(msg, 0) : false);
  char buff[64];
  sprintf(buff, "Set power to %d", power);
  osc::debug(buff);
  motors::setEnginePower(power);
}

void startNavigation(OSCMessage& msg) {
    if (osc::argIsNumber(msg, 0)) {
      float speed = osc::getArgAsFloat(msg, 0);
      char buff[64];
      sprintf(buff, "Start navigation heading with speed %.2F", speed);
      osc::debug(buff);
      morphose::navigation::startHeading(speed, osc::argIsNumber(msg, 1) ? osc::getArgAsFloat(msg, 1) : 0);
    }
}

void stopNavigation(OSCMessage& msg) {
    osc::debug("Stop navigation heading");
    morphose::navigation::stopHeading();
}

void calibrationBegin(OSCMessage& msg) {
    osc::debug("Start IMUS calibration");
    imus::beginCalibration();
  }

void calibrationEnd(OSCMessage& msg) {
    osc::debug("Stop IMUS calibration");
    imus::endCalibration();
  }

void saveCalibration(OSCMessage& msg) {
    osc::debug("Saving imus calibration");
    imus::saveCalibration();
}

void rgbAll(OSCMessage& msg) {
    if (osc::argIsNumber(msg, 0) && osc::argIsNumber(msg, 1) && osc::argIsNumber(msg, 2)) {
      int r = osc::getArgAsInt(msg, 0);
      int g = osc::getArgAsInt(msg, 1);
      int b = osc::getArgAsInt(msg, 2);
      int w = osc::argIsNumber(msg, 3) ? osc::getArgAsInt(msg, 3) : 0;
      char buff[64];
      sprintf(buff, "Set all pixels to %d, %d, %d, %d", r, g, b, w);
      osc::debug(buff);
      pixels::setAll(r, g, b, w);
    }
}

void rgbOne(OSCMessage& msg) {
    if (osc::argIsNumber(msg, 0) && osc::argIsNumber(msg, 1) && osc::argIsNumber(msg, 2) && osc::argIsNumber(msg, 3)) {
      int i = osc::getArgAsInt(msg, 0);
      int r = osc::getArgAsInt(msg, 1);
      int g = osc::getArgAsInt(msg, 2);
      int b = osc::getArgAsInt(msg, 3);
      int w = osc::argIsNumber(msg, 4) ? osc::getArgAsInt(msg, 4) : 0;
      char buff[64];
      sprintf(buff, "Set pixel %d to %d, %d, %d, %d",i , r, g, b, w);
      osc::debug(buff);
      pixels::set(i, r, g, b, w);
    }
}

void rgbRegion(OSCMessage& msg) {
    if (osc::argIsNumber(msg, 0) && osc::argIsNumber(msg, 1) && osc::argIsNumber(msg, 2) && osc::argIsNumber(msg, 3)) {
      pixels::Region region = (pixels::Region) osc::getArgAsInt(msg, 0);
      int r = osc::getArgAsInt(msg, 1);
      int g = osc::getArgAsInt(msg, 2);
      int b = osc::getArgAsInt(msg, 3);
      int w = osc::argIsNumber(msg, 4) ? osc::getArgAsInt(msg, 4) : 0;
      char buff[64];
      sprintf(buff, "Set pixel region %d to %d, %d, %d, %d",osc::getArgAsInt(msg, 0), r, g, b, w);
      osc::debug(buff);
      pixels::setRegion(region, r, g, b, w);
    }
}


void baseColor(OSCMessage& msg) {
  if (osc::argIsNumber(msg, 0) && osc::argIsNumber(msg, 1) && osc::argIsNumber(msg, 2)) {
    int r = osc::getArgAsInt(msg, 0);
    int g = osc::getArgAsInt(msg, 1);
    int b = osc::getArgAsInt(msg, 2);
//        int w = osc::argIsNumber(msg, 3) ? osc::getArgAsInt(msg, 3) : 0;
    char buff[64];
    sprintf(buff, "base color to %d, %d, %d", r, g, b);
    osc::debug(buff);
    animations::currentAnimation().baseColor.setRgb(r, g, b);
  }
                 }

void altColor(OSCMessage& msg) {
                   if (osc::argIsNumber(msg, 0) && osc::argIsNumber(msg, 1) && osc::argIsNumber(msg, 2)) {
                     int r = osc::getArgAsInt(msg, 0);
                     int g = osc::getArgAsInt(msg, 1);
                     int b = osc::getArgAsInt(msg, 2);
                //        int w = osc::argIsNumber(msg, 3) ? osc::getArgAsInt(msg, 3) : 0;
                char buff[64];
    sprintf(buff, "alt color to %d, %d, %d", r, g, b);
    osc::debug(buff);
                     animations::currentAnimation().altColor.setRgb(r, g, b);
                   }
                 }

void animationPeriod(OSCMessage& msg) {
  {
                   if (osc::argIsNumber(msg, 0)) {
                     float p = osc::getArgAsFloat(msg, 0);

                     char buff[64];
    sprintf(buff, "period to %d ", p);
    osc::debug(buff);
                     animations::currentAnimation().setPeriod(p);
                   }
                 }
}

void noise(OSCMessage& msg) {
                   if (osc::argIsNumber(msg, 0)) {
                     float noise = osc::getArgAsFloat(msg, 0);
                     int global = osc::argIsNumber(msg, 1) ? osc::getArgAsBool(msg, 1) : true;

                     char buff[64];
    sprintf(buff, "noise to %.2F", noise);
    osc::debug(buff);
                     animations::currentAnimation().setNoise(noise);
                   }
                 }

void animationType(OSCMessage& msg) {
                   if (osc::argIsNumber(msg, 0)) {
                     animations::AnimationType type = (animations::AnimationType) osc::getArgAsInt(msg, 0);
                     char buff[64];
    sprintf(buff, "anim to %d", osc::getArgAsInt(msg, 0));
    osc::debug(buff);
                     animations::currentAnimation().setType(type);
                   }
                 }

void animationRegion(OSCMessage& msg) {
                   if (osc::argIsNumber(msg, 0)) {
                     pixels::Region region = (pixels::Region) osc::getArgAsInt(msg, 0);
                     char buff[64];
    sprintf(buff, "anim region to %d", osc::getArgAsInt(msg, 0));
    osc::debug(buff);
                     animations::currentAnimation().setRegion(region);
  }
}

void log(OSCMessage& msg) {
    int val =  msg.getInt(0);
    char buff[64];
    sprintf(buff, "anim region to %d",val);
    logger::info(buff);
}

void readyToFlush(OSCMessage& msg) {
  int val =  msg.getInt(0);
  if (val) {
    osc::debug("Flush confirmed");
    logger::readyToFlush();
    Serial.println(ESP.getFreeHeap());
    logger::flush();
  }
}

void endLog(OSCMessage& msg) {
  int val =  msg.getInt(0);
  if (val) {
    osc::debug("end log confirmed");
    logger::endLog();
  }
}

}  // namespace oscCallback

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSCCALLBACKS_H_
