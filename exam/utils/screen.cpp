#include "screen.h"
#include "string"
#include <cstddef>
#include <vector>


void displayInit(DFRobot_RGBLCD1602 &lcd)
{
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setRGB(255, 255, 255);
}

void startScreen(DFRobot_RGBLCD1602 &lcd, long &epoch, float &lat, float &lon, string &city)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Unix epoch time:");
    lcd.setCursor(0, 1);
    lcd.printf("%li", epoch);
    
    ThisThread::sleep_for(2s);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Lat: %0.5f", lat);
    lcd.setCursor(0, 1);
    lcd.printf("Lon: %.5f", lon);

    ThisThread::sleep_for(2s);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("City:");
    lcd.setCursor(0, 1);
    lcd.printf("%s", city.c_str());

}

void forecastScreen(DFRobot_RGBLCD1602 &lcd, string &weather, float &temp)
{
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.printf("%s", weather.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("%.1f C", temp);       
}

void dateTimeScreen(DFRobot_RGBLCD1602 &lcd, string &day, string &time)
{
    lcd.clear();
    lcd.setCursor(0, 0); 
    getHHMM(time);
    lcd.printf("%s", day.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("%s", time.c_str());
}

void newsScreen(DFRobot_RGBLCD1602 &lcd, vector<string> &headlines)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("CNN");

    string full;
    const size_t lcdLen = 16;
    for (const auto &h : headlines)
    {
        full += h;
        full += "  ---  ";
    }
    size_t len = full.size();
    size_t pos = 0;
    while (true)
    {
        string slice;
        if (pos + lcdLen <= len)
        {
            slice = full.substr(pos, lcdLen);
        }
        else 
        {
            size_t tail = len - pos;
            slice = full.substr(pos, tail) + full.substr(0, lcdLen - tail);
        }
    lcd.setCursor(0, 1);
    lcd.printf("%s", slice.c_str());
    pos = (pos + 1) % len;
    ThisThread::sleep_for(300ms);
    }
}
