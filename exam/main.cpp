#include "apiFunc.h"
#include "screen.h"
#include "DFRobot_RGBLCD1602.h"

I2C i2clcd (PB_9, PB_8);
DFRobot_RGBLCD1602 lcd (&i2clcd, RGB_ADDRESS_V20_7BIT, LCD_ADDRESS_7BIT, 16, 2);

NetworkInterface *n = nullptr;
TLSSocket w;
TCPSocket t;
TCPSocket ip;
TCPSocket news;
SocketAddress wa;
SocketAddress ta;
SocketAddress ipa;
SocketAddress na;
JsonDocument ipDoc;
JsonDocument weatherDoc;

long epochTime;
float lat = NULL;
float lon = NULL;
float apiTemp = NULL;
string city;
string day;
string timezone;
string dst;
string forecastWeather;
vector<string> headlines;

int main()
{
    ipFunc(n, ip, ipa, ipDoc, lat, lon, city);
    timeConnect(t, ta, n, lat, lon, epochTime, day, timezone, dst);
    setTime(epochTime);
    displayInit(lcd);
    startScreen(lcd, epochTime, lat, lon, city);




    // weather thread update every 15 min ish

    while (true) 
    {
        weatherFunc(n, w, wa, weatherDoc, lat, lon, apiTemp, forecastWeather);
        forecastScreen(lcd, forecastWeather, apiTemp);

        newsFeed(news, na, n, headlines);
        for (int i = 0; i < headlines.size(); i++)
        {
            printf("%s\n", headlines[i].c_str());
        }
        
        printf("\nTemp: %.1f\nWeather: %s\nCity: %s\n", apiTemp, forecastWeather.c_str(), city.c_str());
        printf("%s\nlat: %.4f\nlon: %.4f\nEpoch: %li\n%s\n\n",day.c_str(), lat, lon, epochTime, dst.c_str());
        break;
        
    }
    return 1;
}
