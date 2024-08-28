/**
 * @file network.cpp
 * @author Etienne Montenegro
 * @brief Implementation file of network.h
 * 
 */
#include "Network.h"

//#include <ArduinoLog.h>
#include <Chrono.h>


#include "communications/asyncMqtt.h"
#include "Morphose.h"
#include "Utils.h"



namespace network {


    int numActiveIPs = 0, lastAddedIPIndex = 0;

    IPAddress pcIP{192, 168, 0, 100};
#if ROBOT_ID == 1
    IPAddress mcuIP{192, 168, 0, 110};
#elif ROBOT_ID == 2
    IPAddress mcuIP{192, 168, 0, 120};
#elif ROBOT_ID == 3
    IPAddress mcuIP{192, 168, 0, 130};
#endif

     IPAddress broadcast{192, 168, 0, 255};
     IPAddress subnet(255, 255, 255, 0);
     IPAddress gateway(192, 168, 0, 1);
     WiFiUDP udp{};


    //  Holds event ids for possibility to remove them
    wifi_event_id_t event_disconnect_id = 0;
    wifi_event_id_t event_connect_id = 0;
    wifi_event_id_t event_ip_id = 0;


    //  ROUTER SSID AND PSWRD
    const char *ssid = WIFI_SSID;
    const char *pswd = WIFI_PASSWORD;

    int outgoingPort = 8001;
    const int incomingPort = 8000;

     

    bool initialize() {
        Serial.println("Initializing network interface");
        Serial.println("Trying to connect to router");

       if (!configureStation()) {
            return false;
        }

        
        if (!connectToWiFi(ssid, pswd)) {
            return false;}
        initializeUDP(incomingPort);
        showRSSI();
        return true;
    } 

    void initialize(uint8_t maxTry) {
        Serial.println("Initializing network interface");
        Serial.println("Trying to connect to router");

        if (!maybeConnectToRouter(maxTry)) {
            Serial.println("ERROR: WiFi unable to connect, Rebooting hardware.");
            utils::blinkIndicatorLed(100, 0.7, 20);
            network::removeWifiEvents();
            ESP.restart();
        }
        initializeUDP(incomingPort);
        showRSSI();
    }


    bool configureStation() {
        Serial.println("Network interface configuration for station mode");

            // delete old config
            if (WiFi.disconnect(true, true)) {
                Serial.println("Successfully delete wifi config");
            }
            WiFi.mode(WIFI_STA);
            WiFi.setSleep(false);  // enable the wifi all the time
            

        // Configures static IP address
        if (!WiFi.config(mcuIP, gateway, subnet)) {
            Serial.println("ERROR: STA Failed to configure");
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
                Serial.print(".");
            }

            if (millis() - start_time > timeout) {
                break;
            }
        }

        mcuIP = WiFi.localIP();

        Serial.println("IP: ");
        Serial.println(mcuIP);

        Serial.println(" ");
        

        if (WiFiClass::status() != WL_CONNECTED) {
            return false;
        }

        return true;
    }

    void initializeUDP(const int port) {
        udp.begin(port);
        Serial.printf("UDP initialized on port %d\n", port);
    }

    bool maybeConnectToRouter(uint8_t max) {
        bool success = false;
        uint8_t count{0};
        if (!configureStation()) {
            return false;
        }

        while (count < max) {
            Serial.printf("Try number %d\n", count + 1);
           
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
        mqtt::debug(buff);
    }

     bool isConnected() {return (WiFi.status() == WL_CONNECTED);}

// wifi events helpers

    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {

            Serial.println("Connected to network.");
    }

    void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
       
        Serial.printf("Disconnected from WiFi %d\n", info.wifi_sta_disconnected.reason);
        //logger::error("Lost wifi connection");
    }

    void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
        Serial.printf("MCU IP address: %d.%d.%d.%d\n", WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
    }

    void setWifiEvents() {
            Serial.println("Setting up wifi events");
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

