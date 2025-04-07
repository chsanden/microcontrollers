#include "mbed.h"
#include "HTS221Sensor.h"
#include "HTS221_driver.h"
#include "DFRobot_RGBLCD1602.h"
#include "DevI2C.h"

DigitalIn button(D3, PullDown);

I2C i2clcd(PB_9, PB_8);
DevI2C i2cSensor(PB_11, PB_10);

DFRobot_RGBLCD1602 lcd (&i2clcd, RGB_ADDRESS_V20_7BIT, LCD_ADDRESS_7BIT, 16, 2);
HTS221Sensor hts221 (&i2cSensor);

bool showTemp = true;

Timer debounceTimer;


int main()
{
    //initialiserer sensor og skjerm
    printf("Starting...\n\r");
    hts221.init(NULL);
    hts221.enable();

    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setRGB(255,255,255);

    debounceTimer.start();

    while(true)
    {
        float temp, hum;
        hts221.get_temperature(&temp);
        hts221.get_humidity(&hum);

        if(button.read() == 1 && debounceTimer.read_ms() > 150)
        {
            showTemp = !showTemp;
            debounceTimer.reset();

        }
        lcd.setCursor(0,0);

        if(showTemp)
        {
            char tempStr[20];
            sprintf(tempStr, "Temp: %.1f C", temp);
            lcd.printf(tempStr);

            //Setter bakgrunnsfarge etter temp avlesninger
            if(temp < 20.0f)
            {
                lcd.setRGB(0, 0, 255);
            }

            else if(temp <= 24.0f)
            {
                lcd.setRGB(255, 165, 0);
            }

            else 
            {
                lcd.setRGB(255, 0, 0);
            }
        }

        else if(!showTemp)
        {
            char humStr[20];
            sprintf(humStr, "Hum: %.1f %%  ", hum);
            lcd.printf(humStr);

            //Lager prosentverdier ut av hum variablen
            float perc = hum / 100.0f;
            if(perc > 1.0f)
            {
                perc = 1.0f;
            }
            if(perc < 0.0f)
            {
                perc = 0.0f;
            }

            //Lager variabler slik at bakgrunnsfargen gradvis forandres i takt med fuktigheten
            int r = (int)(255 * (1.0f - perc));
            int g = (int)(255 * (1.0f - perc));
            int b = 255;

            //Legger inn variablene til fargefunksjonen
            lcd.setRGB(r, g, b);

            
        }

        //Avleser hvert sekund
        thread_sleep_for(1000);
        



    }
}
