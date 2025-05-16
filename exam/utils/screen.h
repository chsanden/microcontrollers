#ifndef SCREEN_H
#define SCREEN_H

#include "HTS221Sensor.h"
#include "TempSensor.h"
#include "HumiditySensor.h"
#include "DFRobot_RGBLCD1602.h"
#include "apiFunc.h"



void startScreen(DFRobot_RGBLCD1602 &lcd, long &epoch, float &lat, float &lon, string &city);
void displayInit(DFRobot_RGBLCD1602 &lcd);
void forecastScreen(DFRobot_RGBLCD1602 &lcd, string &weather, float &temp);



#endif