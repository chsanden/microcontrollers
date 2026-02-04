#include "screen.h"
#include "string"
#include <cstddef>
#include <vector>

//Initialising lcd screen
void displayInit(DFRobot_RGBLCD1602 &lcd)
{
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setRGB(255, 255, 255);
}

//Screen to be displayed after bootup
void startScreen(HTS221Sensor &hts221, DFRobot_RGBLCD1602 &lcd, long &epoch, float &lat, float &lon, string &city)
{
    if (hts221.init(nullptr) != 0) {
        printf("HTS221 init failed!\n");
    }
    hts221.enable();
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
    ThisThread::sleep_for(2s);
}

//Weather forecast and temp screen
void forecastScreen(DFRobot_RGBLCD1602 &lcd, string &weather, float &temp)
{
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.printf("%s", weather.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("%.1f C", temp);       
}

//Screen to display day, date and time
void dateTimeScreen(DFRobot_RGBLCD1602 &lcd, string &day, string &time)
{
    lcd.clear();
    lcd.setCursor(0, 0); 
    getHHMM(time);
    lcd.printf("%s", day.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("%s", time.c_str());
}

//Screen to display news fetched from cnn rss feed
void newsScreen(DFRobot_RGBLCD1602 &lcd, vector<string> &headlines, size_t &pos)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("CNN");

    const size_t lcdLen = 16;
    if (headlines.empty() || (headlines.size() == 1 && headlines[0].empty()))
    {
        lcd.setCursor(0, 1);
        lcd.printf("No news");
        return;
    }

    // Join all headlines into one string with separators
    string full;
    for (const auto &h : headlines)
    {
        full += h;
        full += "  ---  ";
    }

    size_t len = full.size();
    if (len < lcdLen) {
        // Pad to prevent weird wrapping
        full += string(lcdLen - len, ' ');
        len = lcdLen;
    }
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

    // Advance pos, wrap if at end
    pos = (pos + 1) % len;
}
