#ifndef APIFUNC_H
#define APIFUNC_H

#include "ISM43362.h"
#include "mbed.h"
#include "ArduinoJson.h"
#include <cstring>
#include "QSPIFBlockDevice.h"
#include "FATFileSystem.h"



const char* findJson(const std::string &response);
void connectLocal(NetworkInterface *&localNetwork);
void closeLocal(NetworkInterface *localNetwork);
void weatherConnect(TLSSocket &socket, SocketAddress &socketAddress, NetworkInterface *localNetwork);
void weatherData(TLSSocket &s, SocketAddress &sa, float &lat, float &lon, JsonDocument &doc);
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
void weatherFunc(NetworkInterface *n, TLSSocket &s, SocketAddress &sa, JsonDocument &doc,float &lat, float &lon, float &temp, string &weather);
void newsFeed(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, std::vector<string> &titles);

//When selected, page will fetch weather data from api to update weather based on location.\
on demmand information\
refresh every 15 min ish

#endif
