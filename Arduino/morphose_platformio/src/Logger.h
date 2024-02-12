#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_LOGGER_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_LOGGER_H_

#include <ESPLogger.h>
#include <ArduinoLog.h>


/*
 - Log errors
 - log initialization steps 
 - log loop steps 
 - limit log size
 - add serial command to print file content into serial port

*/
namespace logger {
    extern ESPLogger logFile;

    const int limit{2048};

    bool error(const char* message);
    bool info(const char* message);
    void initialize();
    void update();
    void readyToFlush();
     void endLog();
    void event();
    void flush();
    bool sendOSC(const char *buffer, int n);
    bool flushHandler(const char *buffer, int n);


}  // namespace logger








#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_LOGGER_H_
