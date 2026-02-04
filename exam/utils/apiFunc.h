#ifndef APIFUNC_H
#define APIFUNC_H

#include "ISM43362.h"
#include "mbed.h"
#include "ArduinoJson.h"
#include <cstring>
#include "QSPIFBlockDevice.h"
#include "FATFileSystem.h"
#include "DFRobot_RGBLCD1602.h"
#include "HTS221Sensor.h"


//All descriptions in .cpp

const char* findJson(const std::string &response);
void connectLocal(NetworkInterface *&localNetwork);
void closeLocal(NetworkInterface *localNetwork);
void setWeatherData(float &temp, std::string &weather, JsonDocument &doc);
void ipGeoConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n);
void ipGeoData(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, JsonDocument &doc);
void setCords(JsonDocument &doc, float &lat, float &lon);
void timeConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, float &lat, float &lon, 
long &epochSec, string &day, string &timezone, string &dst, string &city);
void setTime(long &epoch);
void getHHMM(string &currentTimeString);
string extractDate(const string &hdr);
void ipFunc(NetworkInterface *&n, TCPSocket &s, SocketAddress &sa, JsonDocument &doc, float &lat, float &lon);
void newsFeed(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, std::vector<string> &titles);
void bootUp(DFRobot_RGBLCD1602 &lcd, TCPSocket &ip, TCPSocket &t, SocketAddress &sa, 
SocketAddress &sa2, NetworkInterface *&n, JsonDocument &doc, float &lat, float &lon,
long &epoch, string &day, string &timezone, string &dst, string &city);

//When selected, page will fetch weather data from api to update weather based on location.\
on demmand information\
refresh every 15 min ish

#endif
