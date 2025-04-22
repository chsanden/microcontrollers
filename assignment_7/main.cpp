#include "mbed.h"
#include "functions.h"
NetworkInterface *network = nullptr;
nsapi_size_or_error_t result = NULL;
TCPSocket socket;
void networkConnect()
{
    network = NetworkInterface::get_default_instance();
    if(!network)
    {
        printf("Failed to get default network interaface\n");
        while(1);
    }
    printf("Connecting to local network...\n");
    result = network->connect();
    if(result != 0)
    {
        printf("Failed to connect to local network...\n");
        printf("Error code: %i", result);
        while(1);
    }
    printf("Connected to local network\n");
    result = 0;
    ThisThread::sleep_for(1s);
}
int main()
{
    networkConnect();
    while (true)
    {
        telemetryLoop(network, socket);
    }
}
