#include "functions.h"
#include "ISM43362Interface.h"
#include <cstddef>
#include <string>
#include "HTS221Sensor.h"
#include "DevI2C.h"
#include "TCPSocket.h"
#include <cstdio>
#include "ArduinoJson.h"

static constexpr char THINGSBOARD_HOST[] = "10.0.0.90";
static constexpr uint16_t THINGSBOARD_PORT = 9090;
static constexpr chrono::milliseconds MEAS_INTERVAL{5000};
static constexpr char  THINGSBOARD_TOKEN[]  = "n0a41lfak8x3yks4h0zp";

static DevI2C i2cSensor(PB_11, PB_10);
static HTS221Sensor hts221(&i2cSensor);

bool readSensor(float &temp, float &hum){
    if(hts221.get_temperature(&temp) != 0){
        return false;
    }
    if(hts221.get_humidity(&hum) != 0){
        return false;
    }
    return true;
}

std::string buildJson(float temp, float hum){
    JsonDocument doc;
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    char buffer[96];
    size_t n = serializeJson(doc, buffer, sizeof(buffer));
    return std::string(buffer, n);
}
bool resolveAddress(NetworkInterface *net, SocketAddress &addr)
{
    if (int r = net->gethostbyname(THINGSBOARD_HOST, &addr)) {
        printf("DNS lookup failed: %d\n", r);
        return false;
    }
    addr.set_port(THINGSBOARD_PORT);
    return true;
}
bool openAndConnect(TCPSocket &sock, NetworkInterface *net, const SocketAddress &addr)
{
    if (int r = sock.open(net)) {
        printf("Socket open() failed: %d\n", r);
        return false;
    }
    sock.set_blocking(true);
    sock.set_timeout(10000);
    int r = sock.connect(addr);
    if (r) {
        printf("Socket connect() failed: %d\n", r);
        sock.close();
        return false;
    }
    printf("Socket connected\n");
    return true;
}
bool hostConnect(TCPSocket &socket, SocketAddress &address, NetworkInterface *net)
{
   
   printf("Connecting %s:%u …\n", address.get_ip_address(), address.get_port());
    socket.set_blocking(true);
    socket.set_timeout(10000);
    int r = socket.connect(address);
    if (r == NSAPI_ERROR_IN_PROGRESS) {
        do {
            r = socket.connect(address);
        } while (r == NSAPI_ERROR_IN_PROGRESS);
    }
    if (r) {
        printf("Socket connect() failed: %d\n", r);
        return false;
    }
    printf("Socket connected\n");
    return true;
}
bool sendTelemetry(TCPSocket &socket, float temp, float hum)
{
    char body[64];
    int body_len = snprintf(body, sizeof(body),
    "{\"temperature\":%.2f,\"humidity\":%.2f}",
    temp, hum);
    if (body_len <= 0 || body_len >= (int)sizeof(body)) {
        printf("Body snprintf overflow\n");
        return false;
    }
    char request[256];
    int req_len = snprintf(request, sizeof(request),
        "POST /api/v1/%s/telemetry HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n"
        "%.*s",
        THINGSBOARD_TOKEN, THINGSBOARD_HOST, body_len, body_len, body);

    if (req_len <= 0 || req_len >= (int)sizeof(request)) {
        printf("Request snprintf overflow\n");
        return false;
    }
    nsapi_size_t sent = 0;
    while (sent < req_len) {
        nsapi_size_or_error_t n = socket.send(request + sent, req_len - sent);
        if (n < 0) {
            printf("socket.send() failed: %d\n", n);
            return false;
        }
        sent += n;
    }
    printf("Sent %u bytes HTTP telemetry\n", sent);
    return true;
}

void telemetryLoop(NetworkInterface *net, TCPSocket &socket)
{
    hts221.init(nullptr);
    hts221.enable();

    if (!net) return;

    SocketAddress addr;
    if (!resolveAddress(net, addr)) return;

    if (!openAndConnect(socket, net, addr)) return;

    while (true) {
        float t, h;
        if (!readSensor(t, h)) {
            printf("Sensor read error – skip\n");
            ThisThread::sleep_for(MEAS_INTERVAL);
            continue;
        }
        if (!sendTelemetry(socket, t, h)) {
                printf("Send failed – reconnecting…");
                socket.close();                         
                if (!openAndConnect(socket, net, addr)) {  
                    printf("Reconnect failed – abort");
                    break;
                }
            continue;      
        }
        
        if (!sendTelemetry(socket, t, h)) {
            printf("[APP] Second send failed – abort\n");
            break;
        }
        socket.close();
        openAndConnect(socket, net, addr);
        ThisThread::sleep_for(MEAS_INTERVAL);
    }
}
    



