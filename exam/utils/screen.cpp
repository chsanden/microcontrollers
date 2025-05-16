#include "screen.h"
#include "string"


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
