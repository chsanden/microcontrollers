#include "apiFunc.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include "time.h"
#include "DFRobot_RGBLCD1602.h"
#include "screen.h"


//Used to find start of Json segment of HTTP(S) responses
const char* findJson(const string &response)
{
    const char *sep = "\r\n\r\n";
    const char *EOhdr = strstr(response.c_str(), sep);
    if(!EOhdr)
    {
        return nullptr;
    }
    const char *bodyStart = EOhdr + strlen(sep);
    const char *jsonStart = strchr(bodyStart, '{');

    return jsonStart;
    
}

//Connecting to local network\
Remember to change SSID and password in mbed_app.json when testing
void connectLocal(NetworkInterface *&localNetwork)
{   
    while(!localNetwork)
    {
        localNetwork = NetworkInterface::get_default_instance();
        if(!localNetwork)
        {
            printf("\nFailed to get local network\n");
            continue;
        }
        printf("---------------------------------\n");
        printf("Connecting to local network...\n");

        int result;
        result = localNetwork->connect();

        if(result != 0)
        {
            printf("Failed to connect to local network\n\n");
            printf("Error code: %i", result);
            continue;
        }
    }
    printf("---------------------------------\n");
    printf("Connected to local network\n");
    printf("---------------------------------\n");
}

//Function to close network interface\
Not used but there in case
void closeLocal(NetworkInterface *localNetwork)
{
    if(localNetwork == 0)
    {
        printf("No local network detected\n\n");
        return;
    }

    if(localNetwork == NetworkInterface::get_default_instance())
    {
        localNetwork->disconnect();
        printf("Disconnected from local network\n\n");
        return;
    }
}

//Set data from weather api to global variables
void setWeatherData(float &temp, std::string &weather, JsonDocument &doc)
{
    JsonArray temps = doc["hourly"] ["temperature_2m"];
    JsonArray weatherCode = doc["hourly"] ["weather_code"];
    float sum = 0;
    size_t count = 0;
    size_t skip = 8;
    size_t i = 0;
    //Calculates the average temp, skipping the first 8 reading of the day  
    for(float t : temps)
    {
        if (i < skip)
        {
            i++;
            continue;
        }
        sum += t;
        count++;
        i++;
    }
    float avgTemp = (count > 0) ? sum / count : NAN;
    temp = avgTemp;
    //Sets the largest weather code as the one for the entire day\
    You'd rather prepare for rain and it not come than the other way around
    int maxCode = 0;
    for (int c : weatherCode)
    {
        if (c > maxCode)
        {
            maxCode = c;
        }
    }
    switch (maxCode) 
    {
        case 0: weather = "Clear sky"; break;
        case 1: weather = "Mainly clear"; break;
        case 2: weather = "Partly cloudy"; break;
        case 3: weather = "Overcast"; break;
        case 45: weather = "Fog"; break;
        case 48: weather = "Depositing rime fog"; break;
        case 51: weather = "Light drizzle"; break;
        case 53: weather = "Moderate drizzle"; break;
        case 55: weather = "Dense drizzle"; break;
        case 56: weather = "Light freezing drizzle"; break;
        case 57: weather = "Dense freezing drizzle"; break;
        case 61: weather = "Slight rain"; break;
        case 63: weather = "Moderate rain"; break;
        case 65: weather = "Heavy rain"; break;
        case 66: weather = "Light freezing rain"; break;
        case 67: weather = "Heavy freezing rain"; break;
        case 71: weather = "Slight snow fall"; break;
        case 73: weather = "Moderate snow fall"; break;
        case 75: weather = "Heavy snow fall"; break;
        case 77: weather = "Snow grains"; break;
        case 80: weather = "Slight rain showers"; break;
        case 81: weather = "Moderate rain showers"; break;
        case 82: weather = "Violent rain showers"; break;
        case 85: weather = "Slight snow showers"; break;
        case 86: weather = "Heavy snow showers"; break;
        case 95: weather = "Thunderstorm"; break;
        case 96: weather = "Thunderstorm w/ hail"; break;
        case 99: weather = "Thunderstorm w/ hail"; break;
        default: weather = "Unknown"; break;
    }
}

//Connect to geodata api
void ipGeoConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n)
{
    s.open(n);
    nsapi_size_or_error_t result = n->gethostbyname("ip-api.com", &sa);
    printf("\nConnecting to target...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of target...\nError code: %i\n", result);
        return;
    }
    sa.set_port(80);
    result = s.connect(sa);
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to target...\nError code: %i\n", result);
        return;
    }
    printf("Connected to: ip-api.com\n");
}

//Get data from geodata api
void ipGeoData(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, JsonDocument &doc)
{
    char req[256];
    int len = snprintf(req, sizeof(req),
    "GET /json/ HTTP/1.1\r\n"
    "Host: ip-api.com\r\n"
    "Connection: close\r\n"
    "\r\n");
    if(len < 0 || len >= (int)sizeof(req))
    {
        printf("Error in sending GET request...\n");
        return;
    }
    int sent = s.send(req, len);
    if(sent < 0)
    {
        printf("Socket error...\n\n");
        return;
    }
    char buffer[512];
    int received;
    nsapi_size_or_error_t recBytes;
    string response;
    while ((recBytes = s.recv(buffer, sizeof(buffer)) )> 0) 
    {
        if(recBytes == 0)
        {
            printf("\nResponse received\n\n");
            break;
        }
        if(recBytes < 0)
        {
            printf("\nError in receiving response...\nReceived bytes: %i\nBreaking...\n\n", recBytes);
            break;
        }
        buffer[recBytes] = '\0';
        printf("%s\n\n", buffer);
        response.append(buffer, recBytes);
    }
    const char *body = findJson(response);
    if(!body)
    {
        printf("Error: could not find HTTP body in response from ip-api.com\n");
    }
    DeserializationError jsonErr = deserializeJson(doc, body);
    if(jsonErr)
    {
        printf("JSON parse failed\nError code: %s\n", jsonErr.c_str());
        return;
    }
    s.close();
}

//Sets lat and lon based on geodata api returns
void setCords(JsonDocument &doc, float &lat, float &lon)
{
    if(lat == NULL || lon == NULL)
    {
        //In an if loop to avoid overwriting user's manually set values
        lat = doc["lat"];
        lon = doc["lon"];
    }
}

//Connect to and write data from timezone api to global variables
void timeConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, float &lat, float &lon, 
long &epochSec, string &day, string &timezone, string &dst, string &city)
{
    s.open(n);
    nsapi_size_or_error_t result = n->gethostbyname("api.timezonedb.com", &sa);
    printf("\nConnecting to target...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of target...\nError code: %i\n", result);
        return;
    }
    sa.set_port(80);
    result = s.connect(sa);
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to target...\nError code: %i\n", result);
        return;
    }
    printf("Connected to: api.timezonedb.com\n");
    char req[256];
    int len = snprintf(req, sizeof(req),
    "GET /v2.1/get-time-zone?key=6RG3GTG10W28&format=json&by=position&lat=%.5f&lng=%.5f HTTP/1.1\r\n"
    "Host: api.timezonedb.com\r\n"
    "Connection: close\r\n"
    "\r\n", lat, lon);
    if(len < 0 || len >= (int)sizeof(req))
    {
        printf("Error in sending GET request...\n");
        return;
    }
    int sent = s.send(req, len);
    if(sent < 0)
    {
        printf("Socket error...\n\n");
        return;
    }
    char buffer[512];
    int received;
    nsapi_size_or_error_t recBytes;
    string response;
    while ((recBytes = s.recv(buffer, sizeof(buffer)) )> 0) 
    {
        if(recBytes == 0)
        {
            printf("\nResponse received\n\n");
            break;
        }
        if(recBytes < 0)
        {
            printf("\nError in receiving response...\nReceived bytes: %i\nBreaking...\n\n", recBytes);
            break;
        }
        buffer[recBytes] = '\0';
        printf("%s\n\n", buffer);
        response.append(buffer, recBytes);
    }
    day = extractDate(response);
    JsonDocument doc;
    const char *body = findJson(response);
    if(!body)
    {
        printf("Error: could not find HTTP body in response from ip-api.com\n");
    }
    DeserializationError jsonErr = deserializeJson(doc, body);
    if(jsonErr)
    {
        printf("JSON parse failed\nError code: %s\n", jsonErr.c_str());
        return;
    }
    int dstBool = doc["dst"];
    if(dstBool == 0)
    {
        dst = "DST Inactive";
    }
    if (dstBool == 1)
    {
        dst = "DST Active";
    }
    city = doc["cityName"].as<string>();
    timezone = doc["abbreviaton"].as<string>();
    epochSec = doc["timestamp"];
    s.close();
}

//Sets RTC time on microcontroller using unix epoch time\
fetched from time api
void setTime(long &epoch)
{
    //Sets RTC time from api fetched unix epoch
    rtc_init();
    rtc_write(epoch);
    set_time(static_cast<time_t>(epoch));
}

//Get timestamp values as string to display later
void getHHMM(string &currentTimeString)
{
    time_t now = time(nullptr);
    struct tm *tm = localtime(&now);
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", tm);
    currentTimeString = buf;
}

//Get date string to display later
string extractDate(const string &hdr) 
{
    auto pos = hdr.find("Date:");
    if (pos == string::npos) 
    {
        return "Error finding date\n";
    }
    pos += 5;
    while (pos < hdr.size() && (hdr[pos] == ' '|| hdr[pos] == '\t')) 
    {
        pos++;
    }
    auto end = hdr.find('\n', pos);
    if (end == std::string::npos) end = hdr.size();
    {
        string dateLine = hdr.substr(pos, end - pos);
        if (dateLine.size() >= 12) 
        {
            return dateLine.substr(0, 12);
        }
        return dateLine;
    }
}

//All ip-geodata functions baked into one func (with a lot of parameters)
void ipFunc(NetworkInterface *&n, TCPSocket &s, SocketAddress &sa, JsonDocument &doc, float &lat, float &lon)
{
    connectLocal(n);
    ipGeoConnect(s, sa, n);
    ipGeoData(s, sa, n, doc);
    setCords(doc, lat, lon);
}

//Using this feed because it is the only TCP connection of an \
international rss feed available. Cannot use multiple TLS connections.\
See main.cpp weather() comment and README for more info
void newsFeed(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, std::vector<string> &headlines)
{
    connectLocal(n);
    s.open(n);
    nsapi_size_or_error_t result = n->gethostbyname("rss.cnn.com", &sa);
    printf("\nConnecting to target...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of target...\nError code: %i\n", result);
        return;
    }
    sa.set_port(80);
    result = s.connect(sa);
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to target...\nError code: %i\n", result);
        return;
    }
    printf("Connected to: rss.cnn.com\n\n\n");
    char req[256];
    int len = snprintf(req, sizeof(req),
    //1.0 to avoid chunking
    "GET /rss/cnn_world.rss HTTP/1.0\r\n"
    "Host: rss.cnn.com\r\n"
    "Connection: close\r\n"
    "\r\n");
    int sent = s.send(req, len);
    if (sent < 0)
    {
        printf("Send error: %i\n", sent);
        return;
    }
    char buffer[128];
    string window;
    int itemsFound = 0;
    headlines.clear();
    //Loop to dynamically extract the three first stories / items
    while (itemsFound < 3) 
    {
        nsapi_size_or_error_t rec = s.recv(buffer, sizeof(buffer));
        if (rec <= 0)
        {
            break;
        }
        window.append(buffer, rec);
        size_t pos = 0;
        while (itemsFound < 3)
        {
            size_t itemPos = window.find("<item>", pos);
            size_t t0 = window.find("<title>", itemPos);
            size_t t1 = window.find("</title>", t0);
            if (itemPos == string::npos || t0 == string::npos || t1 == string::npos)
            {
                break;
            }
            //Extract only title text
            string title = window.substr(t0 + 7, t1 - (t0 + 7));
            const string cstart = "<![CDATA[";
            const string cend = "]]>";
            if (title.rfind(cstart, 0) == 0 && title.size() >= cstart.size() + cend.size())
            {
                //Removing data formatting wrapper around headlines
                title = title.substr(cstart.size(), title.size() - cstart.size() - cend.size());
            }
            headlines.push_back(title);
            itemsFound++;
            //Free up memory
            window.erase(0, t1 + 8);
            pos = 0;
        }
        //Dynamically erases old, irrelevant data from memory
        if (window.size() > 512)
        {
            window.erase(0, window.size() - 256);
        }
    }
    printf("Response received\n\n");
    s.close();
}

//All bootup functions baked into one (with even more parameters)
void bootUp(DFRobot_RGBLCD1602 &lcd, TCPSocket &ip, TCPSocket &t, SocketAddress &sa, SocketAddress &sa2, NetworkInterface *&n, JsonDocument &doc, float &lat, float &lon,
long &epoch, string &day, string &timezone, string &dst, string &city)
{    
    displayInit(lcd);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.printf("Booting...");
    ipFunc(n, ip, sa, doc, lat, lon);
    timeConnect(t, sa2, n, lat, lon, epoch, day, timezone, dst, city);
    setTime(epoch);
}

