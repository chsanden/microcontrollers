#ifndef UTILS_H
#define UTILS_H

#include "ISM43362.h"
#include "HTS221Sensor.h"
#include "TempSensor.h"
#include "HumiditySensor.h"
#include "DFRobot_RGBLCD1602.h"
#include "mbed.h"
#include "ArduinoJson.h"
#include <cstring>
#include "QSPIFBlockDevice.h"
#include "FATFileSystem.h"



const char* findJson(const std::string &response);
void connectLocal(NetworkInterface *&localNetwork);
void closeLocal(NetworkInterface *localNetwork);
void weatherConnect(TLSSocket &socket, SocketAddress &socketAddress, NetworkInterface *localNetwork);
void weatherData(TLSSocket &s, SocketAddress &sa, float &lat, float &lon, FATFileSystem &fs, QSPIFBlockDevice &qspi, FILE *f);
void ipGeoConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n);
void ipGeoData(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, JsonDocument &doc);
void setCords(JsonDocument &doc, float &lat, float &lon);
void setWeatherValues(FILE *f, float &temp, string &weatherCode);
bool extractForecast(TLSSocket &s, float &temp, char &weather);
void setWeatherValues(float &temp, string &weatherCode);
void setWeather(float &temp, string &weather, JsonDocument &doc);


//When selected, page will fetch weather data from api to update weather based on location.\
on demmand information\
refresh every 15 min ish

#endif
