/**
 * @file network.h
 * @author Etienne Montenegro
 * @brief File containing all functions and variables related to network communication between MCU and Computer
 * 
 */
#ifndef NETWORK_H
#define NETWORK_H
#pragma once

#include <WiFiUdp.h>
#include <WiFi.h>

namespace network {

    extern IPAddress pcIP;              // Computer controlling the kit's IP
    extern IPAddress mcuIP;
    extern IPAddress broadcast;
    extern IPAddress subnet;
    extern WiFiUDP udp;
    extern int outgoingPort;

    // TODO(Etienne) : Remove if addDestinationIPAddress is not used anymore
    // #define MAX_DEST_IPS 4
    // extern IPAddress ipList[MAX_DEST_IPS];
    // extern byte destIPs[MAX_DEST_IPS];
    // extern int numActiveIPs, lastAddedIPIndex;



// wifi helpers
    
    // TODO(Etienne) : Verify if still needed
    // void addDestinationIPAddress(byte ip3);

    void showRSSI();

    /// @brief connects to the wifi network using the provided information
    /// @param ssid network name to connect to
    /// @param pwd password of the network
    /// @param timeout number of ms it tries before breaking out
    /// @return true if connection successful, else return false
    bool connectToWiFi(const char *ssid, const char *pwd, uint16_t timeout = 10000);



    /// @brief initialize udp connection
    void initializeUDP(const int port);

    /// @brief  network initialization routine
    void initialize(uint8_t maxTry = 1);



    /// @brief tries to connect to router
    /// @param max maximum of try before failing
    /// @return true if connection to router successful, else return false
    bool maybeConnectToRouter(uint8_t max);

    bool isConnected();

    /**
     * @brief Configure the MCU tu be used une station mode
     * 
     * @return true Configured successfully
     * @return false Failed to configure
     */
    bool configureStation();

    /**
     * @brief links wifi events with callbacks
     */
    void setWifiEvents();

    /**
     * @brief Removed linked wifi event. Needed when rebooting MCU to prevent falling in disconnecting/reconnecting loop
     */
    void removeWifiEvents();
// Wifi event callbacks declarations
    /**
     * @brief Wrapper of stationConnection() for ESP32 Connected Wifi event
     */
    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);

    /**
     * @brief Wrapper of stationDisconnected() for ESP32 Disconnected Wifi event
     * 
     */
    void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

    /**
     * @brief Wrapper of stationGotIP() for ESP32 IP attribution Wifi event
     * 
     */
    void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);



}   // namespace network

#endif
