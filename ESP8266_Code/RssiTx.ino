#define OPTION_UDP_CHANNEL_ENABLE 1

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#if OPTION_UDP_CHANNEL_ENABLE
#include <WiFiUdp.h>
#endif

#include "Pinger.h"
#include "user_interface.h"

#ifndef STASSID
#define STASSID "RTES_G8T5_AP"
#define STAPSK "27Qz4DCq"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const int led = LED_BUILTIN;

char packetBuffer[10 + 1];

#if OPTION_UDP_CHANNEL_ENABLE
WiFiUDP Udp;
#endif
Pinger pinger;

void setup(void) {
    //pinMode(led, OUTPUT);
    //digitalWrite(led, 0);
    //pinMode(2, OUTPUT);
    //digitalWrite(2, 0);

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    //Serial.println();
    //Serial.printf("CPUFreq=%d\n", (uint32)system_get_cpu_freq());

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //Serial.print(".");
    }
    //Serial.printf("\nConnected to %s\nIP address: %s\n", ssid, WiFi.localIP().toString().c_str());

#if OPTION_UDP_CHANNEL_ENABLE
    Udp.begin(65088);
#endif
    pinger.SetEchoPayloadLength(0);
    
    Serial.write("\0\0\0\0", 4);
}

void loop(void) {
#if OPTION_UDP_CHANNEL_ENABLE
    static IPAddress remoteIp;
    static uint16 remotePort = 0;
    if (remotePort) {
        while (Udp.read(packetBuffer, 10));
        Udp.beginPacket(remoteIp, remotePort);
    }
#endif

    static uint32 lastUpdate = micros();
    //Serial.printf("CPU_Used=%3d\n", (micros() - lastUpdate) * 100 / 10000);
    while (micros() - lastUpdate < 10000);
    lastUpdate = micros();

    static uint32 lastLightUp = micros();
    if (micros() - lastLightUp >= 1000000) {
        lastLightUp += 1000000;
    }
    /*if (micros() - lastLightUp < 100000) {
        digitalWrite(2, 0);
    } else {
        digitalWrite(2, 1);
    }*/

    pinger.Ping(WiFi.gatewayIP(), 1, 10);

    static int32_t initialRssi = WiFi.RSSI();
    static int32_t nr;
    if (initialRssi != (nr = WiFi.RSSI())) {
        initialRssi = nr;
        //digitalWrite(led, 0);
#if OPTION_UDP_CHANNEL_ENABLE
        if (remotePort)
            Udp.printf("RSSI=%d\n", initialRssi);
#endif
    } else {
        //digitalWrite(led, 1);
    }
    Serial.write((char*)&nr, 4);
    Serial.write("\0\0\0\0", 4);

#if OPTION_UDP_CHANNEL_ENABLE
    if (remotePort)
        Udp.endPacket();

    if (!remotePort) {
        int packetSize = Udp.parsePacket();
        if (packetSize) {
            remoteIp = Udp.remoteIP();
            remotePort = Udp.remotePort();
            //Serial.printf("===============> UDP port %u\n", (uint32)remotePort);
        }
    }
#endif
}
