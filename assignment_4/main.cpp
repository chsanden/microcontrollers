#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"
#include <cstdint>


InterruptIn pauseButt(D3, PullDown);
InterruptIn watchButt(D4, PullDown);

I2C i2clcd (PB_9, PB_8);

volatile bool pauseState = true;
char buf[16];

constexpr uint32_t WD_TIMEOUT_10S = 10000;

Timer debounceTimer;
Timer stopwatch;

DFRobot_RGBLCD1602 lcd (&i2clcd, RGB_ADDRESS_V20_7BIT, LCD_ADDRESS_7BIT, 16, 2);


void pauseCB()
{
    if(debounceTimer.read_ms() > 150)
    {
    pauseState = !pauseState;
    debounceTimer.reset();
    }
}
void watchCB()
    {
        if(debounceTimer.read_ms() > 150)
        {
        Watchdog::get_instance().kick();
        debounceTimer.reset();
        }
    }

    void screenPrint()
    {
       float timer = stopwatch.read();

        int intPart = (int) timer;
        int fracPart = (int)((timer - intPart) * 100);
        
        char buf[16];
        sprintf(buf, "%d.%02d", intPart, fracPart);
        lcd.setCursor(0, 0);
        lcd.printf("%s", buf);
    }

int main()
{
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setRGB(255, 255, 255);
    
    pauseButt.rise(&pauseCB);
    watchButt.rise(&watchCB);

    printf("Starting...\n");
    Watchdog &watchdog = Watchdog::get_instance();    
    stopwatch.start();
    debounceTimer.start();
    while (true) 
    {
        if(pauseState == true)
        {
            stopwatch.stop();
            if(watchdog.is_running() == true)
            {
                watchdog.stop();
                thread_sleep_for(50);
            }
        }
        
        
        thread_sleep_for(50);
        watchdog.start(WD_TIMEOUT_10S);
        stopwatch.start();
        
        while(pauseState == false)
        {
            if(watchdog.is_running() == false)
            {
                watchdog.start(WD_TIMEOUT_10S);
            }

            screenPrint();
            thread_sleep_for(10);
        }
    }
}

