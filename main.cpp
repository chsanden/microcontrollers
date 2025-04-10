/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "ISM43362Interface.h"
#include "ArduinoJson.h"
#include <cstdio>
#include <cstring>
//Declaring interrupt flag
volatile bool initiateFlag = false;
//Declaring button to start sequence
InterruptIn button(PC_13, PullNone);
//Initialising socket and network
TLSSocket socket;
NetworkInterface *network = nullptr;
nsapi_size_or_error_t result = NULL;
//De-bounce timer for button
Timer d;
//Functions and callbacks
void initiateCB()
{
    if(d.read_ms() > 1000)
    {
        initiateFlag = true;
        d.reset();
    }
}
const char* findJson(const std::string &response)
{
    const char *sep = "\r\n\r\n";
    size_t pos = response.find(sep);
    if(pos == std::string::npos)
    {
        return nullptr;
    }
    return response.c_str() + pos + strlen(sep);
}
void timeNow()
{
    double now = std::chrono::duration_cast<std::chrono::seconds> 
    (std::chrono::system_clock::now().time_since_epoch()).count();
    set_time(now);
    printf("Time set\n");
}
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
    socket.open(network);
    ThisThread::sleep_for(1s);
}
void hostConnect(SocketAddress input)
{
    //Certificates for TLS handshake
    const char SSL_CA_PEM[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICnzCCAiWgAwIBAgIQf/MZd5csIkp2FV0TttaF4zAKBggqhkjOPQQDAzBHMQsw\n"
    "CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
    "MBIGA1UEAxMLR1RTIFJvb3QgUjQwHhcNMjMxMjEzMDkwMDAwWhcNMjkwMjIwMTQw\n"
    "MDAwWjA7MQswCQYDVQQGEwJVUzEeMBwGA1UEChMVR29vZ2xlIFRydXN0IFNlcnZp\n"
    "Y2VzMQwwCgYDVQQDEwNXRTEwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARvzTr+\n"
    "Z1dHTCEDhUDCR127WEcPQMFcF4XGGTfn1XzthkubgdnXGhOlCgP4mMTG6J7/EFmP\n"
    "LCaY9eYmJbsPAvpWo4H+MIH7MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggr\n"
    "BgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQU\n"
    "kHeSNWfE/6jMqeZ72YB5e8yT+TgwHwYDVR0jBBgwFoAUgEzW63T/STaj1dj8tT7F\n"
    "avCUHYwwNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzAChhhodHRwOi8vaS5wa2ku\n"
    "Z29vZy9yNC5jcnQwKwYDVR0fBCQwIjAgoB6gHIYaaHR0cDovL2MucGtpLmdvb2cv\n"
    "ci9yNC5jcmwwEwYDVR0gBAwwCjAIBgZngQwBAgEwCgYIKoZIzj0EAwMDaAAwZQIx\n"
    "AOcCq1HW90OVznX+0RGU1cxAQXomvtgM8zItPZCuFQ8jSBJSjz5keROv9aYsAm5V\n"
    "sQIwJonMaAFi54mrfhfoFNZEfuNMSQ6/bIBiNLiyoX46FohQvKeIoJ99cx7sUkFN\n"
    "7uJW\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
    "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
    "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
    "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
    "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
    "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
    "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
    "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
    "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
    "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
    "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
    "-----END CERTIFICATE-----\n";
    socket.set_hostname("catfact.ninja");
    socket.set_root_ca_cert(SSL_CA_PEM);
    result = network->gethostbyname("catfact.ninja", &input);
    printf("Connecting to host...\n");
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of host...\n");
        printf("Error code: %i\n", result);
        while(1);
    }
    input.set_port(443);
    result = socket.connect(input);
    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to server...\n");
        printf("Error code: %i \n", result);
        while(1);
    }
    printf("Connected to host\n");
    result = 0;
    ThisThread::sleep_for(1s);
}
void sendRequest(SocketAddress input)
{
    result = network->gethostbyname("catfact.ninja", &input);
    char request[] = "GET /fact HTTP/1.1\r\n"
    "Host: catfact.ninja\r\n"
    "Connection: close\r\n"
    "\r\n";
    nsapi_size_t bytesToSend = strlen(request);
    nsapi_size_or_error_t bytesSent = 0;    
    while (bytesToSend)
    {
        bytesSent = socket.send(request + bytesSent, bytesToSend);
        if(bytesSent <= 0)
        {
            printf("Error in sending request...\n");
            printf("Current request: \n%s", request);
        }
        else
        {
            printf("Bytes sent: %i\n", bytesSent);
        }
        bytesToSend -= bytesSent;
    }
    printf("Request sent\n");
    result = 0;
    ThisThread::sleep_for(1s);

}
void jsonPrint(char input[], JsonDocument doc, int MAX_INPUT_LENGTH)
{
    // char input[MAX_INPUT_LENGTH];
    while (input != 0)
    {
        DeserializationError error = deserializeJson(doc, input, MAX_INPUT_LENGTH);

        if (error) 
        {
            printf("deserializeJson() failed: %s", error.c_str());
            break;
        }

        const char* fact = doc["fact"]; // "A domestic cat can run at speeds of 30 mph."
        int length = doc["length"]; // 43
        printf("Fact: %s\n", fact);
        printf("Length: %i\n", length);
    }



}void hostResponse()
{
    const int chunkSize = 10;
    char buffer[chunkSize + 1];
    std::string response;
    while(true)
    {  
        nsapi_size_or_error_t recBytes = socket.recv(buffer, chunkSize);
        if(recBytes == 0)
        {   
            printf("\n\nTransmission completed\n");
            break;
        }
        if(recBytes < 0)
        {
            printf("Error in receiving response...\n");
            printf("%i\nBreaking...", recBytes);
            break;
        }

        buffer[recBytes] = '\0';
        printf("%s", buffer);
        printf("...");
        response += buffer;
    }
    const char *body = findJson(response);
    if(!body)
    {
        printf("Error: could not find HTTPS body in response...\n");
    }
    JsonDocument doc;
    DeserializationError jsonErr = deserializeJson(doc, body);
    if(jsonErr)
    {
        printf("JSON parse failed: %s\n", jsonErr.c_str());
    }
    const char* fact = doc["fact"];
    int length = doc["length"];
    printf("\n\nParsed JSON:\n");
    printf("Fact: %s\n", fact);
    printf("Length: %i\n", length);
    socket.close();
    network = nullptr;
    ThisThread::sleep_for(1s);
}
//-------------------
//-------------------
//-------------------
int main()
{   
    ThisThread::sleep_for(5s);
    timeNow();
    SocketAddress address;
    button.fall(&initiateCB);
    d.start();
    printf("Ready...\n");
    while(true)
    {
        if(initiateFlag)
        {
            networkConnect();
            hostConnect(address);
            sendRequest(address);
            hostResponse();
            printf("\n\nSequence completed...\n");
            initiateFlag = false;
            ThisThread::sleep_for(5s);
            break;
        }
        ThisThread::sleep_for(2s);
    }
    return 0;
}
