#include "apiFunc.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include "time.h"



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

void connectLocal(NetworkInterface *&localNetwork)
{
    localNetwork = NetworkInterface::get_default_instance();
    if(!localNetwork)
    {
        printf("\nFailed to get local network\n");
        return;
    }
    printf("---------------------------------\n");
    printf("Connecting to local network...\n");

    int result;
    result = localNetwork->connect();

    if(result != 0)
    {
        printf("Failed to connect to local network\n\n");
        printf("Error code: %i", result);
        return;
    }

    printf("Connected to local network\n");
    printf("---------------------------------\n");
}

void closeLocal(NetworkInterface *localNetwork)
{
    if(localNetwork == 0)
    {
        printf("No local network detected\n\n");
        return;
    }

    if(localNetwork == NetworkInterface::get_default_instance())
    {
        localNetwork = nullptr;
        printf("Disconnected from local network\n\n");
        return;
    }
}

void weatherConnect(TLSSocket &socket, SocketAddress &socketAddress, NetworkInterface *localNetwork)
{
    socket.open(localNetwork);
    const char SSL_CERT[]=
    "    -----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

    socket.set_hostname("api.open-meteo.com");
    socket.set_root_ca_cert(SSL_CERT);
    nsapi_size_or_error_t result = localNetwork->gethostbyname("api.open-meteo.com", &socketAddress);
    printf("\nConnecting to target...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of target...\nError code: %i\n", result);
        return;
    }

    socketAddress.set_port(443);
    result = socket.connect(socketAddress);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to target...\nError code: %i\n", result);
        return;
    }


    printf("Connected to: api.open-meteo.com\n");
}

void weatherData(TLSSocket &s, SocketAddress &sa, float &lat, float &lon, JsonDocument &doc)
{
    char req[256];

    int len = snprintf(req, sizeof(req),
    "GET /v1/forecast?latitude=%.5f&longitude=%.5f&hourly=temperature_2m,weather_code&models=metno_seamless&forecast_days=1 HTTP/1.1\r\n"
    "Host: api.open-meteo.com\r\n"
    "Connection: close\r\n"
    "\r\n", lat, lon);

    if(len < 0 || len >= (int)sizeof(req))
    {
        error("\nError in sending GET request...\n");
        return;
    }

    int sent = s.send(req, len);
    if(sent < 0)
    {
        error("\nSocket error...\n\n");
        return;
    }
    string response;
    int size = 10;
    char buffer[size + 1];

    while (1) 
    {
        nsapi_size_or_error_t recBytes = s.recv(buffer, size);

        if(recBytes == 0)
        {
            printf("\nResponse received\n");
            break;
        }
        if(recBytes < 0)
        {
            error("\nError in receiving response...\nReceived bytes: %i\nBreaking...\n\n", recBytes);
            break;
        }
        buffer[recBytes] = '\0';
        printf("%s", buffer);
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

void setCords(JsonDocument &doc, float &lat, float &lon, string &city)
{
    if(lat == NULL || lon == NULL)
    {
        //In an if loop to avoid overwriting user's manually set values
        lat = doc["lat"];
        lon = doc["lon"];
    }
    city = doc["city"].as<string>();
}

void timeConnect(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, float &lat, float &lon, 
long &epochSec, string &day, string &timezone, string &dst)
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
    
    timezone = doc["abbreviaton"].as<string>();
    epochSec = doc["timestamp"];
    s.close();
}

void setTime(long &epoch)
{
    //Sets RTC time from api fetched unix epoch
    rtc_init();
    rtc_write(epoch);
    set_time(static_cast<time_t>(epoch));
}

void getHHMM(string &currentTimeString)
{
    time_t now = time(nullptr);
    struct tm *tm = localtime(&now);
    char buf[6];

    strftime(buf, sizeof(buf), "%H:%M", tm);
    currentTimeString = buf;
}

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

void ipFunc(NetworkInterface *&n, TCPSocket &s, SocketAddress &sa, JsonDocument &doc, float &lat, float &lon, string &city)
{
    connectLocal(n);
    ipGeoConnect(s, sa, n);
    ipGeoData(s, sa, n, doc);
    setCords(doc, lat, lon, city);
}

void weatherFunc(NetworkInterface *n, TLSSocket &s, SocketAddress &sa, JsonDocument &doc,float &lat, float &lon, float &temp, string &weather)
{
    weatherConnect(s, sa, n);
    weatherData(s, sa, lat, lon, doc);
    setWeatherData(temp, weather, doc);
}

void newsFeed(TCPSocket &s, SocketAddress &sa, NetworkInterface *n, std::vector<string> &headlines)
{
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
    //Loop to dynamically extract the thre first stories / items
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
    s.close();
}