namespace osc_callback {

void bonjour(OSCMessage& msg) {
  // Collect last part of IP address and add it to the list of destinations.
  byte destIP3 = argIsNumber(msg, 0) ? getArgAsInt(msg, 0) : udp.remoteIP()[3];
  addDestinationIPAddress(destIP3);
  bndl.add("/bonjour").add(boardName).add(destIP3);
  sendOscBundle();
}

void reboot(OSCMessage& msg) {
  ESP.restart();
}

void speed(OSCMessage& msg) {
  if (argIsNumber(msg, 0)) {
    float val = getArgAsFloat(msg, 0);
    setEngineSpeed(val);
  }  
}

void steer(OSCMessage& msg) {
  if (argIsNumber(msg, 0)) {
    float val = getArgAsFloat(msg, 0);
    setEngineSteer(val);
  }
}

void stream(OSCMessage& msg) {
  if (argIsNumber(msg, 0)) {
    bool stream = getArgAsBool(msg, 0);
    // If new value, enable/disable OSC & wake/sleep IMU.
    if (sendOSC != stream) {
      sendOSC = stream;
      if (sendOSC)
        wakeIMUs();
      else
        sleepIMUs();
    }
  }
}

void power(OSCMessage& msg) {
  bool power = (argIsNumber(msg, 0) ? getArgAsBool(msg, 0) : false);
  setEnginePower( power );
}

void navigation(OSCMessage& msg, int offset) {
  // Head in a certain direction.
  if (msg.match("/start", offset)) {
    if (argIsNumber(msg, 0)) {
      float speed = getArgAsFloat(msg, 0);
      startNavigationHeading(speed, argIsNumber(msg, 1) ? getArgAsFloat(msg, 1) : 0);
    }
  }

  // Head in a certain direction.
  else if (msg.match("/stop", offset)) {
    stopNavigationHeading();
  }
}

void calibration(OSCMessage& msg, int offset) {
  // Head in a certain direction.
  if (msg.match("/begin", offset)) {
    calibrateBeginIMUs();
  }

  // Head in a certain direction.
  else if (msg.match("/end", offset)) {
    calibrateEndIMUs();
  }


  // Head in a certain direction.
  else if (msg.match("/save", offset)) {
    calibrateSaveIMUs();
  }
}

void rgb(OSCMessage& msg, int offset) {
  if (msg.match("/all", offset)) {
    Serial.println("Base match");
    if (argIsNumber(msg, 0) && argIsNumber(msg, 1) && argIsNumber(msg, 2)) { 
      int r = getArgAsInt(msg, 0);
      int g = getArgAsInt(msg, 1);
      int b = getArgAsInt(msg, 2);
      int w = argIsNumber(msg, 3) ? getArgAsInt(msg, 3) : 0;
      setPixels(r, g, b, w);
    }
  }

  //else
  if (msg.match("/one", offset)) {
    if (argIsNumber(msg, 0) && argIsNumber(msg, 1) && argIsNumber(msg, 2) && argIsNumber(msg, 3)) { 
      int i = getArgAsInt(msg, 0);
      int r = getArgAsInt(msg, 1);
      int g = getArgAsInt(msg, 2);
      int b = getArgAsInt(msg, 3);
      int w = argIsNumber(msg, 4) ? getArgAsInt(msg, 4) : 0;
      setPixel(i, r, g, b, w);
    }

  else if (msg.match("/region", offset)) {
    if (argIsNumber(msg, 0) && argIsNumber(msg, 1) && argIsNumber(msg, 2) && argIsNumber(msg, 3)) { 
      PixelRegion region = (PixelRegion) getArgAsInt(msg, 0);
      int r = getArgAsInt(msg, 1);
      int g = getArgAsInt(msg, 2);
      int b = getArgAsInt(msg, 3);
      int w = argIsNumber(msg, 4) ? getArgAsInt(msg, 4) : 0;
      setPixelsRegion(region, r, g, b, w);
    }
  }

}
  
}

}
