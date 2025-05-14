#include "utils.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>


const char* findJson(const string &response)
{
    const char *sep = "\r\n\r\n";
    size_t pos = response.find(sep);
    if(pos == std::string::npos)
    {
        return nullptr;
    }
    return response.c_str() + pos + strlen(sep);
}

//Find needle in haystack function\
used to find n-th instance of temp and weather\
in HTTP response from api.met.no
const char* find_nth(const char *haystack, const char *needle, int n)
{
    const char *p = haystack;
    int count = 0;
    size_t len = strlen(needle);

    while ((p = strstr(p, needle)) != nullptr) 
    {
        count++;
        if (count == n) 
        {
            return p;
        }
        p += len;
    }
    //Return nullptr if n-th instance doesn't exist
    return nullptr;
}

//Extracts values of interes from weather HTTP response\
Uses find_nth function decclared above
void extract_values(const char *body, float &temp, string &weatherCode)
{
    //Second instance of "air_Temperature"
    const char *air_p = find_nth(body, "air_temperature", 2);
    if (air_p) 
    {
        // move to the ':' after the key…
        const char *colon = strchr(air_p, ':');
        if (colon)
        {
            // read the numeric text up to the next comma
            const char *end = strchr(colon + 1, ',');
            if (end) 
            {
                char temp_txt[16] = { 0 };
                size_t n = end - (colon + 1);
                if (n < sizeof(temp_txt)) 
                {
                    memcpy(temp_txt, colon + 1, n);
                    float temp_val = atof(temp_txt);
                    printf("2nd air_temperature = %.1f\n", temp_val);
                    temp = temp_val;
                }
            }
        }
    } 
    else 
    {
        printf("air_temperature #2 not found\n");
    }

    // 2) First instance of "symbol_code"
    const char *sym_p = find_nth(body, "symbol_code", 1);
    if (sym_p) 
    {
        // find the opening quote after the key
        const char *q1 = strchr(sym_p, '\"');
        if (q1) 
        {
            // find the closing quote
            const char *q2 = strchr(q1 + 1, '\"');
            if (q2) 
            {
                char code_txt[32] = { 0 };
                size_t n = q2 - (q1 + 1);
                if (n < sizeof(code_txt)) 
                {
                    memcpy(code_txt, q1 + 1, n);
                    printf("symbol_code #1 = %s\n", code_txt);
                    weatherCode = code_txt;
                }
            }
        }
    } 
    else 
    {
        printf("symbol_code #1 not found\n");
    }
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
   " -----BEGIN CERTIFICATE-----\n"
    "MIIG5TCCBM2gAwIBAgIRANpDvROb0li7TdYcrMTz2+AwDQYJKoZIhvcNAQEMBQAw\n"
    "gYgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpOZXcgSmVyc2V5MRQwEgYDVQQHEwtK\n"
    "ZXJzZXkgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMS4wLAYD\n"
    "VQQDEyVVU0VSVHJ1c3QgUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTIw\n"
    "MDIxODAwMDAwMFoXDTMzMDUwMTIzNTk1OVowRDELMAkGA1UEBhMCTkwxGTAXBgNV\n"
    "BAoTEEdFQU5UIFZlcmVuaWdpbmcxGjAYBgNVBAMTEUdFQU5UIE9WIFJTQSBDQSA0\n"
    "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEApYhi1aEiPsg9ZKRMAw9Q\n"
    "r8Mthsr6R20VSfFeh7TgwtLQi6RSRLOh4or4EMG/1th8lijv7xnBMVZkTysFiPmT\n"
    "PiLOfvz+QwO1NwjvgY+Jrs7fSoVA/TQkXzcxu4Tl3WHi+qJmKLJVu/JOuHud6mOp\n"
    "LWkIbhODSzOxANJ24IGPx9h4OXDyy6/342eE6UPXCtJ8AzeumTG6Dfv5KVx24lCF\n"
    "TGUzHUB+j+g0lSKg/Sf1OzgCajJV9enmZ/84ydh48wPp6vbWf1H0O3Rd3LhpMSVn\n"
    "TqFTLKZSbQeLcx/l9DOKZfBCC9ghWxsgTqW9gQ7v3T3aIfSaVC9rnwVxO0VjmDdP\n"
    "FNbdoxnh0zYwf45nV1QQgpRwZJ93yWedhp4ch1a6Ajwqs+wv4mZzmBSjovtV0mKw\n"
    "d+CQbSToalEUP4QeJq4Udz5WNmNMI4OYP6cgrnlJ50aa0DZPlJqrKQPGL69KQQz1\n"
    "2WgxvhCuVU70y6ZWAPopBa1ykbsttpLxADZre5cH573lIuLHdjx7NjpYIXRx2+QJ\n"
    "URnX2qx37eZIxYXz8ggM+wXH6RDbU3V2o5DP67hXPHSAbA+p0orjAocpk2osxHKo\n"
    "NSE3LCjNx8WVdxnXvuQ28tKdaK69knfm3bB7xpdfsNNTPH9ElcjscWZxpeZ5Iij8\n"
    "lyrCG1z0vSWtSBsgSnUyG/sCAwEAAaOCAYswggGHMB8GA1UdIwQYMBaAFFN5v1qq\n"
    "K0rPVIDh2JvAnfKyA2bLMB0GA1UdDgQWBBRvHTVJEGwy+lmgnryK6B+VvnF6DDAO\n"
    "BgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHSUEFjAUBggr\n"
    "BgEFBQcDAQYIKwYBBQUHAwIwOAYDVR0gBDEwLzAtBgRVHSAAMCUwIwYIKwYBBQUH\n"
    "AgEWF2h0dHBzOi8vc2VjdGlnby5jb20vQ1BTMFAGA1UdHwRJMEcwRaBDoEGGP2h0\n"
    "dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9u\n"
    "QXV0aG9yaXR5LmNybDB2BggrBgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6\n"
    "Ly9jcnQudXNlcnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAl\n"
    "BggrBgEFBQcwAYYZaHR0cDovL29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0B\n"
    "AQwFAAOCAgEAUtlC3e0xj/1BMfPhdQhUXeLjb0xp8UE28kzWE5xDzGKbfGgnrT2R\n"
    "lw5gLIx+/cNVrad//+MrpTppMlxq59AsXYZW3xRasrvkjGfNR3vt/1RAl8iI31lG\n"
    "hIg6dfIX5N4esLkrQeN8HiyHKH6khm4966IkVVtnxz5CgUPqEYn4eQ+4eeESrWBh\n"
    "AqXaiv7HRvpsdwLYekAhnrlGpioZ/CJIT2PTTxf+GHM6cuUnNqdUzfvrQgA8kt1/\n"
    "ASXx2od/M+c8nlJqrGz29lrJveJOSEMX0c/ts02WhsfMhkYa6XujUZLmvR1Eq08r\n"
    "48/EZ4l+t5L4wt0DV8VaPbsEBF1EOFpz/YS2H6mSwcFaNJbnYqqJHIvm3PLJHkFm\n"
    "EoLXRVrQXdCT+3wgBfgU6heCV5CYBz/YkrdWES7tiiT8sVUDqXmVlTsbiRNiyLs2\n"
    "bmEWWFUl76jViIJog5fongEqN3jLIGTG/mXrJT1UyymIcobnIGrbwwRVz/mpFQo0\n"
    "vBYIi1k2ThVh0Dx88BbF9YiP84dd8Fkn5wbE6FxXYJ287qfRTgmhePecPc73Yrzt\n"
    "apdRcsKVGkOpaTIJP/l+lAHRLZxk/dUtyN95G++bOSQqnOCpVPabUGl2E/OEyFrp\n"
    "Ipwgu2L/WJclvd6g+ZA/iWkLSMcpnFb+uX6QBqvD6+RNxul1FaB5iHY=\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
    "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
    "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
    "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
    "MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
    "BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
    "aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
    "dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
    "AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
    "3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
    "tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
    "Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
    "VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
    "79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
    "c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
    "Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
    "c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
    "UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
    "Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
    "BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
    "A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
    "Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
    "VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
    "ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
    "8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
    "iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
    "Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
    "XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
    "qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
    "VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
    "L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
    "jjxDah2nGN59PRbxYvnKkKj9\n"
    "-----END CERTIFICATE-----\n";

    socket.set_hostname("api.met.no");
    socket.set_root_ca_cert(SSL_CERT);
    nsapi_size_or_error_t result = localNetwork->gethostbyname("api.met.no", &socketAddress);
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


    printf("Connected to: api.met.no\n");
}

void weatherData(TLSSocket &s, SocketAddress &sa, float &lat, float &lon, FATFileSystem &fs, QSPIFBlockDevice &qspi, FILE *f)
{
    if(qspi.init() != 0)
    {
        printf("\nQSPI initialisation failed...\n");
    }
    if(fs.mount(&qspi) != 0)
    {
        printf("Formatting QSPI flash...\n");
        if(fs.reformat(&qspi) != 0)
        {
            error("Mount after format failed\n");
        }
    }
    f = fopen("/qspi/HTTPSresponse.bin", "wb");
    if(!f)
    {
        error("Could not open file on QSPI");
    }

    char req[256];

    int len = snprintf(req, sizeof(req),
    "GET /weatherapi/locationforecast/2.0/compact?lat=%.5f&lon=%.5f HTTP/1.1\r\n"
    "Host: api.met.no\r\n"
    "Accept: application/json\r\n"
    "User-Agent: WeatherCheckForMicrocontrollerExam/1.0 (christops@uia.no)\r\n"
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
            printf("Response written to QSPI flash\n\n");
            break;
        }
        if(recBytes < 0)
        {
            error("\nError in receiving response...\nReceived bytes: %i\nBreaking...\n\n", recBytes);
            break;
        }
        buffer[recBytes] = '\0';
        printf("%s", buffer);
        fwrite(buffer, 1, recBytes, f);
        
    }
    fclose(f);
    s.close();
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

void setCords(JsonDocument &doc, float &lat, float &lon)
{
    if(lat == NULL || lon == NULL)
    {
        //In an if loop to avoid overwriting user's manually set values
        lat = doc["lat"];
        lon = doc["lon"];
    }
}

void setWeatherValues(float &temp, string &weatherCode)  
{  
    const char *file = "/qspi/HTTPSresponse.bin";  
    FILE *f = fopen(file, "rb");  
    if (!f) {  
        error("Failed to open %s\n", file);  
    }  

    // Skip HTTP headers by detecting \r\n\r\n  
    int state = 0;  
    int c;  
    while ((c = fgetc(f)) != EOF && state < 4) 
    {  
        if ((state == 0 || state == 2) && c == '\r') 
        {  
            state++;  
        } 
        else if ((state == 1 || state == 3) && c == '\n') 
        {  
            state++;  
        } 
        else 
        {  
            state = 0;  
        }  
    }  
    if (state < 4) 
    {  
        fclose(f);  
        error("Could not find end of headers\n");  
    }  

    // 1) Find 2nd "air_temperature" and read its value  
    const char *key1 = "air_temperature";  
    size_t k1len = strlen(key1);  
    size_t match = 0;  
    int occ = 0;  
    while ((c = fgetc(f)) != EOF) 
    {  
        if ((char)c == key1[match]) 
        {  
            match++;  
            if (match == k1len) 
            {  
                occ++;  
                if (occ == 2) 
                {  
                    break;  
                }  
                match = 0;  
            }  
        } 
        else 
        {  
            match = ((char)c == key1[0]) ? 1 : 0;  
        }  
    }  
    if (occ < 2) 
    {  
        fclose(f);  
        error("air_temperature not found twice\n");  
    }  
    // skip to ':'  
    while ((c = fgetc(f)) != EOF && c != ':');  
    // skip whitespace  
    while ((c = fgetc(f)) != EOF && isspace(c));  
    // read numeric text  
    char num[16] = {0};  
    int i = 0;  
    while (c != EOF && (isdigit(c) || c=='+'||c=='-'||c=='.') && i < (int)sizeof(num)-1) 
    {  
        num[i++] = c;  
        c = fgetc(f);  
    }  
    num[i] = '\0';  
    temp = atof(num);  

    // 2) Find 1st "symbol_code" and read its string  
    fseek(f, 0, SEEK_SET);  // rewind to start  
    state = 0;  
    // skip headers again  
    while ((c = fgetc(f)) != EOF && state < 4) 
    {  
        if ((state == 0 || state == 2) && c == '\r') 
        { 
            state++; 
        }  
        else if ((state == 1 || state == 3) && c == '\n') 
        { 
            state++; 
        }  
        else 
        { 
            state = 0; 
        }  
    }  
    const char *key2 = "symbol_code";  
    size_t k2len = strlen(key2);  
    match = 0;  
    while ((c = fgetc(f)) != EOF) 
    {  
        if ((char)c == key2[match]) 
        {  
            if (++match == k2len) 
            {  
                break;  
            }  
        } 
        else 
        {  
            match = ((char)c == key2[0]) ? 1 : 0;  
        }  
    }  
    if (match < k2len) 
    {  
        fclose(f);  
        error("symbol_code not found\n");  
    }  
    // skip to first '"' after colon  
    while ((c = fgetc(f)) != EOF && c != ':');  
    while ((c = fgetc(f)) != EOF && c != '"');  
    // read string  
    char code[64] = {0};  
    i = 0;  
    while ((c = fgetc(f)) != EOF && c != '"' && i < (int)sizeof(code)-1) 
    {  
        code[i++] = c;  
    }  
    code[i] = '\0';  
    weatherCode = code;  

    fclose(f);  
}


