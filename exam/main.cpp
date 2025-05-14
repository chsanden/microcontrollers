#include "utils.h"

//Initialising on-board flash storage modules due to large weather HTTPS response
static QSPIFBlockDevice qspi( QSPI_FLASH1_IO0, QSPI_FLASH1_IO1, QSPI_FLASH1_IO2, QSPI_FLASH1_IO3, QSPI_FLASH1_SCK, QSPI_FLASH1_CSN, QSPIF_POLARITY_MODE_0);
static FATFileSystem fs("qspi");


NetworkInterface *n = nullptr;
TLSSocket w;
TCPSocket ip;
SocketAddress wa;
SocketAddress ipa;
JsonDocument ipDoc;
FILE *f;

float lat = NULL;
float lon = NULL;
char city = NULL;
float apiTemp = NULL;
string weather;
int main()
{

    connectLocal(n);
    ipGeoConnect(ip, ipa, n);
    ipGeoData(ip, ipa, n, ipDoc);
    setCords(ipDoc, lat, lon);




// weather thread update every 15 min ish

    while (true) 
    {
        weatherConnect(w, wa, n);
        weatherData(w, wa , lat, lon, fs, qspi, f);
        setWeatherValues(apiTemp, weather);
        printf("\nTemp: %.1f\nWeather: %s\n", apiTemp, weather.c_str());
        
        printf("lat: %.4f\nlon: %.4f\n\n", lat, lon);
        break;
        
    }
    return 1;
}
