#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"

//Declaring and configuring the LCD display
I2C i2clcd (PB_9, PB_8);
DFRobot_RGBLCD1602 lcd (&i2clcd, RGB_ADDRESS_V20_7BIT, LCD_ADDRESS_7BIT, 16, 2);
BufferedSerial pc(USBTX, USBRX);

//Declaring I/O
InterruptIn pauseButt(D3, PullDown);
InterruptIn resetButt(D4, PullDown);
InterruptIn add5Butt(D5, PullDown);
InterruptIn sub5Butt(D6, PullDown);
PwmOut buzzer(D2);

//Setting up timers and timeout
Timeout t;
Timer d;
Timer r;

//Declating variables
volatile bool isPaused = true;
volatile bool wasPaused = !isPaused;
volatile bool timeComplete = false;
volatile bool lcdReset = false;
volatile uint32_t posCount = 0;
volatile uint32_t negCount = 0;
int initialTime = 60;
int usrTime = 0;
int oldUsrTime = usrTime;
int elapsedTime = 0; 
int remainingTime = initialTime;
int oldRemainingTime = remainingTime;
const int debounceTime = 101;

//Declaring callbacks and functions
void countingCB()
{   
    buzzer.period(1.0 / 440.0); // A-note
    buzzer.write(0.5);
    timeComplete = true;
    remainingTime = 0;
}

void pauseCB()
{
    if(d.read_ms() > debounceTime)
    {
        isPaused = !isPaused;
        d.reset();
    }
}

void resetCB()
{
    if(d.read_ms() > debounceTime)
    {
        pc.write("Reset button pressed\n", 22);
        lcdReset = true;
        timeComplete = false;
        isPaused = true;
        usrTime = 0;
        buzzer.write(0.0);
        r.stop();
        r.reset();
        d.reset();
    }
}

void add5CB()
{
    if((d.read_ms() > debounceTime) && ((posCount || negCount) <= 4))
    {
        posCount++;
        d.reset();
    }
}

void sub5CB()
{
    if((d.read_ms() > debounceTime) && ((posCount || negCount) <= 4))
    {
        negCount++;
        d.reset();
    }
}

void screenPrint(int i)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("%i", i);
    lcd.printf("s");
}
void timeCalc()
{
    usrTime += 5 * posCount;
    usrTime -= 5 * negCount;
    elapsedTime = r.read();
    remainingTime = initialTime + usrTime - elapsedTime;
    if (remainingTime < 0) //Negative values could cause issues 
    {
        remainingTime = 0;
    }
    oldRemainingTime = remainingTime;
    oldUsrTime = usrTime;
}
void displayInit()
{
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setRGB(255, 255, 255);
}
void resetLCD()
{
    if(lcdReset)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.printf("Resetting...");
        lcdReset = false;
    }
}
void buttonConfig()
{
    pauseButt.fall(&pauseCB);
    resetButt.fall(&resetCB);
    add5Butt.fall(&add5CB);
    sub5Butt.fall(&sub5CB);
}
void updateTimeout()
{
    timeCalc();
    if(timeComplete || remainingTime <= 0)
    {
        t.detach();
        r.stop();
        oldRemainingTime = remainingTime;
        return;
    }
    if(isPaused)
    {
        t.detach();
        r.stop();
        if((posCount || negCount) != 0)
        {
            pc.write("Adjusting timer\n", 17);
            posCount = 0;
            negCount = 0;
        }
        return;
    }
    if((posCount || negCount) != 0)
    {
        pc.write("Adjusting timer\n", 17);
        t.detach();
        t.attach(&countingCB, std::chrono::seconds(remainingTime));
        posCount = 0;
        negCount = 0;
        return;
    }
}
int main()
{
    pc.set_baud(115200);
    pc.write("Initialising...\n", 16);
    displayInit();
    buttonConfig();
    //Starting timers
    d.start();
    r.start();
    pc.write("Initialised\n", 13);
    while (true)
    {
        screenPrint(remainingTime);
        resetLCD();
        if (timeComplete)
        {
            pc.write("Time out reached. Buzzer active.\n", 36);
            ThisThread::sleep_for(1s);
            continue;
        }
        if (isPaused && !wasPaused)//Handle pause state
        {
            pc.write("Paused\n", 8);
            r.stop();
        }
        else if (!isPaused && wasPaused)//Handle running state
        {
            pc.write("Running\n", 9);
            r.start();
        }
        updateTimeout();
        if (remainingTime <= 0)
        {
            buzzer.period(1.0 / 440.0);
            buzzer.write(0.5);
            timeComplete = true;
            t.detach();
            continue;
        }
        wasPaused = isPaused;
        ThisThread::sleep_for(500ms);
    }
}

