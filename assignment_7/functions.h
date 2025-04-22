#ifndef FUNCITONS_H
#define FUNCITONS_H

#include "mbed.h"
#include "ISM43362Interface.h"
#include "ArduinoJson.h"

bool readSensor(float &temp, float &hum);

std::string buildJson(float temp, float hum);

bool hostConnect(TCPSocket &socket, SocketAddress &address, NetworkInterface *net);

bool sendTelemetry(TCPSocket &socket, float temp, float hum);

void telemetryLoop(NetworkInterface *net, TCPSocket &socket);



#endif
