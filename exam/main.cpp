#include "apiFunc.h"
#include "screen.h"
#include "DFRobot_RGBLCD1602.h"
#include "HTS221Sensor.h"
#include "DevI2C.h"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <vector>
#include <chrono>

// Initialisation of all instances, variables, devices, threads, etc
I2C i2clcd (PB_9, PB_8);
DFRobot_RGBLCD1602 lcd (&i2clcd, RGB_ADDRESS_V20_7BIT, LCD_ADDRESS_7BIT, 16, 2);
DevI2C dev_i2c(PB_11, PB_10);
HTS221Sensor hts221(&dev_i2c);
PwmOut buzzer(D9);

NetworkInterface *n = nullptr;
TCPSocket t;
TCPSocket ip;
TCPSocket news;
SocketAddress ta;
SocketAddress ipa;
SocketAddress na;
JsonDocument ipDoc, weatherDoc;
Timeout debounce1, debounce2, debounce3, debounce4, debounce5;
Mutex m;
Mutex nm;

InterruptIn button1(D2, PullDown);
InterruptIn button2(D3, PullDown);
InterruptIn button3(D4, PullDown);
InterruptIn button4(D5, PullDown);
InterruptIn button5(D8, PullDown);

Thread weatherUpdateThread(osPriorityAboveNormal1, 4 * 1024, nullptr, "weather");
Thread alarmThread(osPriorityAboveNormal3, 4 * 1024, nullptr, "alarm");
Thread newsThread(osPriorityNormal, 8 * 1024, nullptr, "news");

volatile bool evt_btn1 = false;
volatile bool evt_btn2 = false;
volatile bool evt_btn3 = false;
volatile bool evt_btn4 = false;
volatile bool evt_btn5 = false;
volatile bool isActive = false;
volatile bool alarmActive = false;
volatile bool snoozeRequest = false;
volatile bool isMuted = false;
volatile bool alarmIsRunning = false;
volatile bool latLong = false;
volatile bool newsActive = false;
volatile bool newsQueued = false;
volatile bool weatherUpdateQueued = false;
volatile bool gotWeather = false;
bool newsSet = false;
bool newsStarted = false;

//Mode enumeration\
Used for the differnet screens
typedef enum {
    mode_default,
    mode_temp_hum,
    mode_alarm_edit,
    mode_geolocation_edit,
    mode_weather_display,
    mode_news_display
} modeSwitch; 

volatile modeSwitch mode = mode_default;

float lat, lon, apiTemp = NULL;
int latitudePosOnLcd[6] = { 0, 1, 3, 4, 5, 6};
int longitudePosOnLcd[7] = { 0, 1, 2, 4, 5, 6, 7};
int posOnLCD[4] = { 0, 1, 3, 4 };
int alarmDigits[4] = {2,2,0,0};
int latitudeDigits[6] = {0,0,0,0,0,0};
int longitudeDigits[7] = {0,0,0,0,0,0,0};
int alarmHour = 0, alarmMin = 0;
int editPos = 0;    
long epochTime;
string city, day, timezone, currentTime, dst, forecastWeather;
vector<string> headlines;
size_t newsPos = 0;
time_t latestNews = 0;

//Button callbacks
void btn1Press() {
    evt_btn1 = true;
    button1.disable_irq();
    debounce1.attach(callback(&button1, &InterruptIn::enable_irq), 0.10f);
}
void btn2Press() {
    evt_btn2 = true;
    button2.disable_irq();
    debounce2.attach(callback(&button2, &InterruptIn::enable_irq), 0.10f);
}
void btn3Press() {
    evt_btn3 = true;
    button3.disable_irq();
    debounce3.attach(callback(&button3, &InterruptIn::enable_irq), 0.10f);
}
void btn4Press() {
    evt_btn4 = true;
    button4.disable_irq();
    debounce4.attach(callback(&button4, &InterruptIn::enable_irq), 0.10f);
}
void btn5Press() {
    evt_btn5 = true;
    button5.disable_irq();
    debounce5.attach(callback(&button5, &InterruptIn::enable_irq), 0.10f);
}

// Cannot reconnect with custom edited lat and lon cords\
because of bug where multiple TLS connections result -3011 nsapi error\
even when trying to reconnect to the exact same host. See README for more info.
nsapi_size_or_error_t weather() 
{
    connectLocal(n);
    TLSSocket w;
    SocketAddress wa;
    static const char cert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";
    w.open(n);
    printf("Setting root cert...\n");
    w.set_root_ca_cert(cert);
    printf("Getting host address...\n");
    w.set_hostname("api.open-meteo.com");

    nsapi_size_or_error_t result = n->gethostbyname("api.open-meteo.com", &wa);
    printf("\nConnecting to target...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of target...\nError code: %i\n", result);
        return -1;
    }
    wa.set_port(443);
    result = w.connect(wa);
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to target...\nError code: %i\n", result);
        return -2;
    }
    printf("Connected to: api.open-meteo.com\n");
    char req[256];
    int len = snprintf(req, sizeof(req),
    "GET /v1/forecast?latitude=%.5f&longitude=%.5f&hourly=temperature_2m,weather_code&models=metno_seamless&forecast_days=1 HTTP/1.1\r\n"
    "Host: api.open-meteo.com\r\n"
    "Connection: close\r\n"
    "\r\n", lat, lon);

    if(len < 0 || len >= (int)sizeof(req))
    {
        error("\nError in sending GET request...\n");
        return -3;
    }
    int sent = w.send(req, len);
    if(sent < 0)
    {
        error("\nSocket error...\n\n");
        return -4;
    }
    string response;
    int size = 10;
    char buffer[size + 1];
    while (1) 
    {
        nsapi_size_or_error_t recBytes = w.recv(buffer, size);

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
        return -5;
    }
    DeserializationError jsonErr = deserializeJson(weatherDoc, body);
    if(jsonErr)
    {
        printf("JSON parse failed\nError code: %s\n", jsonErr.c_str());
        return -6;
    }
    gotWeather = true;
    w.close();   
    return 2;
}    

//Alarm and time functions
void showTime() {
    
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    char buf[20];
    strftime(buf, sizeof(buf), "%a %d %b %H:%M", t);

    m.lock();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("%s", buf);

    lcd.setCursor(0, 1);
    if (alarmActive) {
        if (snoozeRequest) {
            lcd.printf("Alarm (S): %02d:%02d", alarmHour, alarmMin);
        } else {
            lcd.printf("Alarm: %02d:%02d", alarmHour, alarmMin);
        }
    }
    m.unlock();
}
void showTempHum() {
    m.lock();
    float temp, hum;
    lcd.clear();
    hts221.get_temperature(&temp);
    lcd.printf("Temp: %.1f C", temp);
    lcd.setCursor(0, 1);
    hts221.get_humidity(&hum);
    lcd.printf("Hum:  %.0f %%", hum);
    m.unlock();
}
void fetchLocation() {
    m.lock();
    lcd.clear();
    if (!latLong) {
        lcd.printf("Latitude:");
        lcd.setCursor(0,1);
        lcd.printf("%d%d.%d%d%d%d", latitudeDigits[0], latitudeDigits[1], latitudeDigits[2], latitudeDigits[3], latitudeDigits[4], latitudeDigits[5]);
        lcd.setCursor(latitudePosOnLcd[editPos], 1);
        lcd.cursor();
    } else {
        lcd.printf("Longitude:");
        lcd.setCursor(0,1);
        lcd.printf("%d%d%d.%d%d%d%d", longitudeDigits[0], longitudeDigits[1], longitudeDigits[2], longitudeDigits[3], longitudeDigits[4], longitudeDigits[5], longitudeDigits[6]);

        lcd.setCursor(longitudePosOnLcd[editPos], 1);
        lcd.cursor();
    }
    m.unlock();
}
void setLocation() {
    lat = latitudeDigits[0] * 10 + latitudeDigits[1] + latitudeDigits[2] * 0.1 + latitudeDigits[3] * 0.01 + latitudeDigits[4] * 0.001 + latitudeDigits[5] * 0.0001;
    lon = longitudeDigits[0] * 100 + longitudeDigits[1] * 10 + longitudeDigits[2] + longitudeDigits[3] * 0.1 + longitudeDigits[4] * 0.01 + longitudeDigits[5] * 0.001 + longitudeDigits[6] * 0.0001;
}
void alarmEdit() {
    m.lock();
    lcd.clear();
    lcd.printf("%d%d:%d%d", alarmDigits[0], alarmDigits[1], alarmDigits[2], alarmDigits[3]);
    lcd.setCursor(0,1);
    lcd.printf(alarmActive ? "Active" : "Inactive");
    lcd.setCursor(posOnLCD[editPos], 0);
    lcd.cursor();
    m.unlock();
}
void setAlarm() {
    alarmHour = alarmDigits[0] * 10 + alarmDigits[1];
    alarmMin  = alarmDigits[2] * 10 + alarmDigits[3];
    m.lock();
    lcd.noCursor();
    lcd.clear();
    ThisThread::sleep_for(1500ms);
    showTime();
    m.unlock();
}
void soundAlarm() {
    buzzer = 0.5f;
    ThisThread::sleep_for(500ms);
    buzzer = 0.0f;
    ThisThread::sleep_for(250ms);  
}
void alarmActivation() {
    alarmActive = !alarmActive;
}

//Thread functions
void alarmThreadFunc() {
    while (true) {
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);

        if (mode != mode_alarm_edit &&
            t->tm_hour == alarmHour &&
            t->tm_min  == alarmMin &&
            alarmActive) {

            snoozeRequest = false;
            isMuted = false;
            time_t alarmStart = now;
            
            while (!isMuted) {
                int minUntilAutoMute = 10;
                do {
                    soundAlarm();
                    alarmIsRunning = true;
                    if((time(nullptr) - alarmStart) >= minUntilAutoMute * 60) {
                        isMuted = true;
                    }
                } while (!snoozeRequest && !isMuted);
                alarmIsRunning = false;
                buzzer = 0.0f;

                if(isMuted) {
                    break;
                }
                ThisThread::sleep_for(5min);
                snoozeRequest = false;
            }
            isMuted = false;
        } else {
            ThisThread::sleep_for(100ms);
        }
    }
    ThisThread::sleep_for(200ms);
}
void weatherUpdateThreadFunc()
{
    time_t last = 0;
    while (true)
    {
        if (weatherUpdateQueued)
        {
            nm.lock();
            int ret = weather();
            nm.unlock();
            if (ret >= 0)
            {

                setWeatherData(apiTemp, forecastWeather, weatherDoc);
                gotWeather = true;
            }
            else 
            {
                gotWeather = false;
            }
            weatherUpdateQueued = false;
        }
        time_t now = time(nullptr);
        if ((now - last) > 30 * 60)
        {
            nm.lock();
            int ret = weather();
            nm.unlock();
            if (ret >= 0)
            {
                setWeatherData(apiTemp, forecastWeather, weatherDoc);
                gotWeather = true;
            }
            else
            {
            gotWeather = false;
            }
            last = now;
        }
        ThisThread::sleep_for(500ms);
    }
}
void newsThreadFunc()
{
    while (true)
    {
        time_t now = time(nullptr);
        while (!gotWeather)
        {
            //To avoid simultanious socket use w/ weather
            ThisThread::sleep_for(1s);
        }

        if (((now - latestNews) > 5 * 60) || newsQueued)
        {
            nm.lock();
            newsFeed(news, na, n, headlines);
            nm.unlock();

            latestNews = now;
            newsQueued = false;
        }
        ThisThread::sleep_for(1s);
    }
    
    
}

//Button handling
void handleBtn1() {
    switch (mode) {

        case mode_alarm_edit: 
        alarmActivation(); 
        alarmEdit(); 
        break;

        case mode_default: mode = mode_temp_hum; 
        showTempHum(); 
        break;

        case mode_temp_hum:
        mode = mode_weather_display; 
        if(gotWeather == false)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.printf("No weather set,");
            lcd.setCursor(0, 1);
            lcd.printf("please refresh");
        }
        forecastScreen(lcd, forecastWeather, apiTemp);
        break;

        case mode_weather_display: 
        mode = mode_news_display;
        newsScreen(lcd, headlines, newsPos);
        newsQueued = true;
        newsActive = true;
        break;

        case mode_news_display: mode = mode_default; 
        newsActive = false; 
        showTime(); 
        break;

        default: break;
    }
}
void handleBtn2() {
    switch (mode) {
        case mode_default: mode = mode_alarm_edit; editPos = 0; alarmEdit(); break;
        case mode_alarm_edit: mode = mode_default; setAlarm(); break;
        case mode_geolocation_edit: latLong = !latLong; editPos = 0; fetchLocation(); break;
        default: break;
    }
}
void handleBtn3() {
    switch (mode) {
        case mode_default: if(alarmIsRunning) snoozeRequest = true; break;
        case mode_alarm_edit: editPos = (editPos == 3) ? 0 : (editPos + 1); alarmEdit(); break;
        case mode_geolocation_edit:
            if (!latLong) editPos = (editPos == 5) ? 0 : (editPos + 1);
            else editPos = (editPos == 6) ? 0 : (editPos + 1);
            fetchLocation();
            break;
        default: break;
    }
}
void handleBtn4() {
    switch (mode) {
        case mode_default:
            if (alarmIsRunning) isMuted = true;
            break;
        case mode_alarm_edit: {
            int &d = alarmDigits[editPos];
            int max;
            switch (editPos) {
                case 0: max = 2; break;
                case 1: max = (alarmDigits[0] == 2) ? 3 : 9; break;
                case 2: max = 5; break;
                case 3: max = 9; break;
            }
            d = (d == max ? 0 : d + 1);
            if (editPos == 0 && alarmDigits[0] == 2 && alarmDigits[1] > 3) alarmDigits[1] = 0;
            alarmEdit();
            break;
        }
        case mode_geolocation_edit:
            if (!latLong) {
                latitudeDigits[editPos] = (latitudeDigits[editPos] + 1) % 10;
            } else {
                longitudeDigits[editPos] = (longitudeDigits[editPos] + 1) % 10;
            }
            fetchLocation();
            break;
        default: break;
    }
}
void handleBtn5() {
    switch (mode) {
        case mode_default: mode = mode_geolocation_edit; editPos = 0; fetchLocation(); break;
        case mode_geolocation_edit:
        mode = mode_default; 
        setLocation();
        weatherUpdateQueued = true;
        default: break;
    }
}



int main()
{
    //Baked all initialisations and start screens into one function\
    Then start alarm and weather tread along with showing the start screen
    bootUp(lcd, ip, t, ipa, ta, n, ipDoc, lat, lon, epochTime, day, timezone, dst, city);
    alarmThread.start(alarmThreadFunc);
    weatherUpdateThread.start(weatherUpdateThreadFunc); 
    startScreen(hts221, lcd, epochTime, lat, lon, city);
    
    //Button activation, activate on button rise
    button1.rise(&btn1Press);
    button2.rise(&btn2Press);
    button3.rise(&btn3Press);
    button4.rise(&btn4Press); 
    button5.rise(&btn5Press);     

    mode = mode_default;

    int lastSec = -1, lastMin = -1;
    uint64_t lastNewsScroll = 0;
    const uint32_t newsScrollInterval = 250;
    while(true) {
        
        if (gotWeather && !newsStarted)
        {
            //To avoid clashing w/ weather thread\
            using simultanious sockets
            newsThread.start(newsThreadFunc);
            newsStarted = true;
            newsQueued = true;
        }

        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        if (evt_btn1) { evt_btn1 = false; handleBtn1(); }
        if (evt_btn2) { evt_btn2 = false; handleBtn2(); }
        if (evt_btn3) { evt_btn3 = false; handleBtn3(); }
        if (evt_btn4) { evt_btn4 = false; handleBtn4(); }
        if (evt_btn5) { evt_btn5 = false; handleBtn5(); }
        if (t->tm_sec != lastSec) {
            lastSec = t->tm_sec;
            if (mode == mode_default) showTime();
        }
        if (t->tm_min != lastMin) {
            lastMin = t->tm_min;
        }

        //News scroll function
        if (mode == mode_news_display) 
        {
            uint64_t msNow = Kernel::get_ms_count();
            if (msNow - lastNewsScroll >= newsScrollInterval) {
                newsScreen(lcd, headlines, newsPos);
                lastNewsScroll = msNow;
            }
        } 
        else 
        {
            // Resett scrollpos and scrolltimer if mdoe change 
            newsPos = 0;
            lastNewsScroll = Kernel::get_ms_count();
        }
        ThisThread::sleep_for(20ms); // Short delay
    }
    return 1;
}
