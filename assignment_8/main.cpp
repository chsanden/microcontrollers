#include "mbed.h"

struct Time {
    Mutex mutex;
    int seconds = 0;
    int minutes = 0;
};


void readFunc(Time *time)
{
    while(true)
    {
        ThisThread::sleep_for(500ms);
        time->mutex.lock();
        int m = time->minutes;
        int s = time->seconds;
        printf("%02d:%02d\r\n", m, s);
        time->mutex.unlock();
    }
}

void addMinutes(Time *time)
{
    while(true)
    {
        ThisThread::sleep_for(60s);
        time->mutex.lock();
        time->minutes++;
        time->seconds = 0;
        time->mutex.unlock();
    }
}

void addseconds(Time *time)
{
    while(true)
    {
        ThisThread::sleep_for(1s);
        time->mutex.lock();
        time->seconds++;
        time->mutex.unlock();
    }
}



int main()
{
    Time timeInstance;

    Thread secondsThread(osPriorityHigh, 2048, nullptr, "seconds");
    Thread minutesThread(osPriorityAboveNormal, 2048, nullptr, "minutes");
    Thread readTimeThread(osPriorityNormal, 2048, nullptr, "read");


    secondsThread.start(callback(addseconds, &timeInstance));
    minutesThread.start(callback(addMinutes, &timeInstance));
    readTimeThread.start(callback(readFunc, &timeInstance));



    while (true) 
    {
        
        
    }
}
