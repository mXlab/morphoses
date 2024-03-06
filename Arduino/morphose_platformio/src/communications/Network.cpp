/**
 * @file network.cpp
 * @author Etienne Montenegro
 * @brief Implementation file of network.h
 * 
 */
#include "Network.h"

#include <ArduinoLog.h>
#include <Chrono.h>

#include "communications/osc.h"
#include "Morphose.h"
#include "Utils.h"
#include "Logger.h"


namespace network {


    int numActiveIPs = 0, lastAddedIPIndex = 0;

     IPAddress pcIP{192, 168, 0, 100};
     IPAddress mcuIP{192, 168, 0, ROBOT_ID};
     IPAddress broadcast{192, 168, 0, 255};
     IPAddress subnet(255, 255, 255, 0);
     IPAddress gateway(192, 168, 0, 1);
     WiFiUDP udp{};


    //  Holds event ids for possibility to remove them
    wifi_event_id_t event_disconnect_id = 0;
    wifi_event_id_t event_connect_id = 0;
    wifi_event_id_t event_ip_id = 0;


    //  ROUTER SSID AND PSWRD
    const char *ssid = "Morphoses";
    const char *pswd = "BouleQuiRoule";

    int outgoingPort = 8000 + ROBOT_ID;
    const int incomingPort = 8000;


    void initialize(uint8_t maxTry) {
        Log.warningln("Initializing network interface");
        Log.infoln("Trying to connect to router");

        if (!maybeConnectToRouter(maxTry)) {
            Log.errorln("WiFi unable to connect, Rebooting hardware.");
            utils::blinkIndicatorLed(100, 0.7, 20);
           network::removeWifiEvents();
            ESP.restart();
        }
        initializeUDP(incomingPort);
        showRSSI();
    }


    bool configureStation() {
        Log.infoln("Network interface configuration for station mode");

            // delete old config
            if (WiFi.disconnect(true, true)) {
                Log.infoln("Successfully delete wifi config");
            }
            WiFi.mode(WIFI_STA);
            WiFi.setSleep(false);  // enable the wifi all the time
            setWifiEvents();

             WiFi.setAutoReconnect(true);



        // Configures static IP address
        if (!WiFi.config(mcuIP, gateway, subnet)) {
            Log.errorln("STA Failed to configure");
            return false;
        }
        return true;
    }


    bool connectToWiFi(const char *_ssid, const char *_pwd, uint16_t timeout) {
        Chrono anim(true);
        unsigned long start_time{millis()};
        
        WiFi.begin(_ssid, _pwd);
        WiFi.waitForConnectResult();

        while (WiFiClass::status() != WL_CONNECTED) {
            utils::blinkIndicatorLed(500);
            if (anim.hasPassed(200, true)) {
                Log.trace(".");
            }

            if (millis() - start_time > timeout) {
                break;
            }
        }

        mcuIP = WiFi.localIP();

        Serial.println("IP: ");
        Serial.println(mcuIP);

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


    void showRSSI() {
        // temp wifi debug
        char buff[64];
        sprintf(buff, "Wifi rssi : %d" , WiFi.RSSI() );
        osc::debug(buff);
    }

     bool isConnected() {return (WiFi.status() == WL_CONNECTED);}

// wifi events helpers

    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {

            Log.noticeln("Connected to network.");
    }

    void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
       
        Log.warningln("Disconnected from WiFi %d", info.wifi_sta_disconnected.reason);
        logger::error("Lost wifi connection");
    }

    void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
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

}    // namespace network

