

/**
 * Log an event every 1.5 second on the internal flash memory and flush it every 20 seconds, the log
 * is flushed over serial port. You can see that after a few records the available space ends and
 * the logger refuses to log more records until it is flushed or reset.
 *
 * NOTE: you should format the flash memory the first time you run this sketch or when you switch
 * file system. Use <YourFS>.format().
 *
 * Your log files should be placed in data/file.log at the of the project.
 * ref : https://community.platformio.org/t/esp32-little-fs-implementation/28803/6
 *
 * error cant open file seem normal according to this thread
 * ref : https://community.platformio.org/t/espasyncwebserver-littlefs-file-does-not-exist-no-permits-for-creation/35949
 */

#include "Logger.h"

#include <Arduino.h>
#include <Chrono.h>

#include "communications/osc.h"

namespace logger
{

  ESPLogger logFile("/data.log");
  Chrono flushTimer(true);

  // Event period, in milliseconds
  unsigned int eventPeriod = 1500;
  // Flush period, in milliseconds
  unsigned int flushPeriod = 20000;

  // Variable to be logged
  static int eventCounter = 0;

  bool flushConfirm = false;



  bool error(const char *message) {
    char buff[128];
    sprintf(buff, "E: %s", message);

    if (!logFile.isFull()) {
      logFile.append(buff, true);
      eventCounter++;
    } else {
      Log.errorln("Cant write to flash because file is full");
    }
    return true;
  }

  bool info(const char *message) {
    char buff[128];

    sprintf(buff, "I: %s", message);

    if (!logFile.isFull()) {
      logFile.append(buff, true);
      eventCounter++;
    } else {
      Log.errorln("Cant write to flash because file is full");
    }
    return true;
  }


  void initialize()
  {
    Log.infoln("ESPLogger initalization");

    // You may need to format the flash before using it


    if (LittleFS.begin(true))
    {
      Log.infoln("File system mounted");
    }
   
   
    // Set the space reserved to the log (in bytes)
    logFile.setSizeLimit(limit);
    logFile.setChunkSize(128);
    logFile.setFlushCallback(sendOSC);

    if (logFile.begin())
    {
      Log.infoln("Starting to log...");
      logFile.append("Log init ok", true);
    }
    else
    {
      Log.errorln("cant start log file...");
    }
  }

  unsigned long previousFlushTime = 0;
  unsigned long previousEventTime = 0;

  void flush(){

    bool val = logFile.flush();


    osc::send("/endLog",1);
    while(flushConfirm) {
      osc::update();
    }
    flushTimer.restart();
  }

  void update() {
    while (Serial.available())
    {
      String message = Serial.readStringUntil('\n');
      Serial.println(message);
      if (message == "dumpflash")
      {
        logFile.print();
      }
    }
    // Check if it's time to flush

    if(logFile.getSize() >= logFile.getSizeLimit()*0.9){
      Serial.println(" Log file almost full");
    }


    if (flushTimer.hasPassed( flushPeriod) && flushTimer.isRunning()) 
    { 

      if(eventCounter == 0) {
        flushTimer.restart();
        osc::debug("Nothing to flush");
        return;
      }

      //previousFlushTime += flushPeriod;
      if (!flushConfirm) {
        osc::send("/flush", 1);
        flushTimer.stop();
        return;
      }
    }
  }

   void readyToFlush() {
    flushConfirm = true;
   }

  void endLog() {
    flushConfirm = false;
    eventCounter = 0;
    logFile.append(" ");
   }

  // TODO(Etienne): Verify if can remove commented code
  bool sendOSC(const char *buffer, int n) {
    int index = 0;
    Serial.println(n);
    // Serial.println(buffer);
    //   Serial.println(sizeof(buffer));
    // Serial.println(n);
    // Print a record at a time
    // Log.errorln(buffer);
    // Serial.println("inside send osc");
    // OSCMessage msg("/log");
    // msg.add(buffer);
    // osc::send(msg);

    // for(int i{0}; i<n,i++){

    // }

    // osc::send("/log",buffer);

    // int size =strlen(&buffer[index]);
    // Log.infoln(" index : %d, n = %d, size = %d", index, n, size);
    // Log.infoln(&buffer[index]);

    while (index < n && strlen(&buffer[index]) > 0) {
      osc::send("/log", &buffer[index]);

      int size = strlen(&buffer[index]);
      // Log.infoln(" index : %d, n = %d, size = %d", index, n, size);
      // Log.infoln(&buffer[index]);
      //  Serial.println("-------");
      //   Serial.println(index);
      //    Serial.println(&buffer[index]);
      // osc::send("/log",&buffer[index]);
      // +1, since '\0' is processed
      index += size + 1;
    }

    return true;
  }

 

}  // namespace logger

