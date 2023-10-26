/**
 * @file network.cpp
 * @author Etienne Montenegro
 * @brief Implementation file of network.h
 * 
 */
#include "Network.h"
#include <ArduinoLog.h>
#include "Morphose.h"
#include <Chrono.h>
#include <Utils.h>


namespace network{



     IPAddress pcIP{192,168,0,199};
     IPAddress mcuIP;
     IPAddress broadcast{192,168,0,255};
     IPAddress subnet(255, 255, 255, 0);
     WiFiUDP udp{};


    //Holds event ids for possibility to remove them
    wifi_event_id_t event_disconnect_id = 0;
    wifi_event_id_t event_connect_id = 0;
    wifi_event_id_t event_ip_id = 0;


    // ROUTER SSID AND PSWRD
    const char *ssid = "Morphoses";
    const char *pswd = "BouleQuiRoule";

    const int outgoingPort = 8130;
    const int incomingPort = 8000;


    void initialize(uint8_t maxTry) {

        Log.warningln("Initializing network interface");
        

        Log.infoln("Trying to connect to router");
        if (!maybeConnectToRouter(maxTry)) {
            Log.errorln("WiFi unable to connect, Rebooting hardware.");
            utils::blinkIndicatorLed(100, 0.7, 20);
            ESP.restart();
        }

        // on disconnect callback
        //TODO : Verify why not in setWifiEvents
       

        initializeUDP(incomingPort);
        showRSSI();
    }


    bool configureStation() {
        Log.infoln("Network interface configuration for station mode");


            // delete old config
            WiFi.disconnect(true);
            WiFiClass::mode(WIFI_STA);

             WiFi.setSleep(false); // enable the wifi all the time
             setWifiEvents();
             //TODO : test autoreconnect function
             //WiFi.setAutoReconnect(true);

        //TODO : Maybe change hostname for robot+id
        //WiFiClass::setHostname(getApSSID());
        

        //TODO : See with Sofian if static ip on esp is preferred
        // Configures static IP address
        // if (!WiFi.config(MisBKit::networkConfig.localIP, MisBKit::networkConfig.gateway, subnet)) {
        //     Log.errorln("STA Failed to configure");
        //     return false;
        // }
        return true;
    }


    bool connectToWiFi(const char *_ssid, const char *_pwd, uint16_t timeout) {
        Chrono anim(true);
        unsigned long start_time{millis()};
        WiFi.begin(_ssid, _pwd);

        while (WiFiClass::status() != WL_CONNECTED) {
            utils::blinkIndicatorLed(500);
            if (anim.hasPassed(200, true)) {
                Log.trace(".");
            }

            if (millis() - start_time > timeout) {
                break;
            }

        }

        Log.traceln(" ");
        Log.setShowLevel(true);

        if (WiFiClass::status() != WL_CONNECTED) {
            return false;
        }

        return true;
    }

    void initializeUDP(const int port) {
        udp.begin(port);
        Log.noticeln("UDP initialized on port %d", port);
    }

    bool maybeConnectToRouter(uint8_t max) {
        bool success = false;
        uint8_t count{0};
        if (!configureStation()) {
            return false;
        }

        while (count < max) {
            Log.info("Try number %d", count + 1);
            Log.setShowLevel(false);
            if (!connectToWiFi(ssid, pswd)) {
                count++;
            } else {
                success = true;
                break;
            }
        }

        if (success) {

            return true;
        } else {

            return false;
        }
    }

    void setHostIP(const IPAddress &_pcIP) {
        pcIP = _pcIP;
    }


    void showRSSI(){
        //temp wifi debug
        char buff[64];
        sprintf(buff,"Wifi rssi : %d" ,WiFi.RSSI() );
        utils::debug(buff);
    }

    IPAddress getMcuIP(){
        return mcuIP;
        }

     bool isConnected(){return (WiFi.status() == WL_CONNECTED);}

// wifi events helpers

    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
            auto _ssid = WiFi.SSID().c_str();
            Log.noticeln("");
            Log.setShowLevel(true);
            Log.noticeln("Connected to network %s successfully.", _ssid);
            Log.setShowLevel(false);
    }

    void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
        
        Log.warningln("Disconnected from WiFi");
        //TODO : print disconnect reason as a string
        Log.infoln("Trying to Reconnect");
        maybeConnectToRouter(3);
        
    }

    void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
        mcuIP = WiFi.localIP();
        Log.noticeln("MCU IP address: [%p]", WiFi.localIP());
    
    }

    void setWifiEvents() {
            Log.noticeln("Setting up wifi events");
            event_connect_id = WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
            event_ip_id = WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
            event_disconnect_id = WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    }

    void removeWifiEvents() {
            WiFi.removeEvent(event_connect_id);
            WiFi.removeEvent(event_disconnect_id);
            WiFi.removeEvent(event_ip_id);
    }

}//namespace network

