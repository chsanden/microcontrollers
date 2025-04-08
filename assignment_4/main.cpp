#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"


InterruptIn pauseButt (D3);
InterruptIn watchButt (D4);

I2C lcd (PB_9, PB_8);

volatile bool pauseState = true;
volatile bool watchState = false;

void pauseCB()
{
    pauseState = !pauseState;
}
void watchCB()
{
    watchState = !watchState;
}

int main()
{
    float timer = 0.00f;


    while (true) {

    }
}

