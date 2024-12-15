#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFiNINA.h>
#include "config.h"

class NetworkManager {
private:
    const char* ssid;
    const char* password;
    const char* serverUrl;
    const char* thingSpeakApiKey;
    WiFiClient thingSpeakClient;  // Regular client for ThingSpeak
    WiFiSSLClient tweeticamClient; // SSL client for Tweeticam
    bool isConnected;

    String urlEncode(const String& msg) {
        String encodedMsg = "";
        char c;
        char code0;
        char code1;
        for (unsigned int i = 0; i < msg.length(); i++) {
            c = msg.charAt(i);
            if (isalnum(c)) {
                encodedMsg += c;
            } else if (c == ' ') {
                encodedMsg += '+';
            } else {
                code1 = (c & 0xf) + '0';
                if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
                c = (c >> 4) & 0xf;
                code0 = c + '0';
                if (c > 9) code0 = c - 10 + 'A';
                encodedMsg += '%';
                encodedMsg += code0;
                encodedMsg += code1;
            }
        }
        return encodedMsg;
    }

public:
    NetworkManager(const char* wifi_ssid, const char* wifi_pass, const char* url, const char* ts_api_key)
        : ssid(wifi_ssid), password(wifi_pass), serverUrl(url), thingSpeakApiKey(ts_api_key), isConnected(false) {}

    bool connect() {
        if (WiFi.status() == WL_CONNECTED) {
            isConnected = true;
            return true;
        }

        int attempts = 0;
        while (attempts < 5) {
            int status = WiFi.begin(ssid, password);
            if (status == WL_CONNECTED) {
                isConnected = true;
                return true;
            }
            delay(5000);
            attempts++;
        }
        return false;
    }

    // Send data to ThingSpeak
    bool sendToThingSpeak(float field1, float field2, float field3, float field4, 
                         float field5, float field6, float field7, float field8) {
        if (!isConnected) return false;

        const char* thingSpeakServer = "api.thingspeak.com";
        String data = String("field1=" + String(field1) + 
                           "&field2=" + String(field2) + 
                           "&field3=" + String(field3) + 
                           "&field4=" + String(field4) +
                           "&field5=" + String(field5) +
                           "&field6=" + String(field6) +
                           "&field7=" + String(field7) +
                           "&field8=" + String(field8));

        if (thingSpeakClient.connect(thingSpeakServer, 80)) {
            thingSpeakClient.println("POST /update HTTP/1.1");
            thingSpeakClient.println("Host: api.thingspeak.com");
            thingSpeakClient.println("Connection: close");
            thingSpeakClient.println("X-THINGSPEAKAPIKEY: " + String(thingSpeakApiKey));
            thingSpeakClient.println("Content-Type: application/x-www-form-urlencoded");
            thingSpeakClient.print("Content-Length: ");
            thingSpeakClient.println(data.length());
            thingSpeakClient.println();
            thingSpeakClient.println(data);

            unsigned long timeout = millis();
            while (thingSpeakClient.connected() && (millis() - timeout < 5000)) {
                if (thingSpeakClient.available()) {
                    thingSpeakClient.read();
                }
            }
            thingSpeakClient.stop();
            return true;
        }
        return false;
    }

    // Original Tweeticam send function
    bool sendToTweeticam(const char* author, const char* secretKey, String message) {
        if (!isConnected) return false;

        if (tweeticamClient.connect(serverUrl, 443)) {
            String request = createTweeticamRequest(author, secretKey, message);
            tweeticamClient.println(request);
            tweeticamClient.println("Host: " + String(serverUrl));
            tweeticamClient.println("Connection: close");
            tweeticamClient.println();
            
            unsigned long timeout = millis();
            while (tweeticamClient.connected() && (millis() - timeout < 10000)) {
                while (tweeticamClient.available()) {
                    tweeticamClient.read();
                    timeout = millis();
                }
            }
            
            tweeticamClient.stop();
            return true;
        }
        return false;
    }

private:
    String createTweeticamRequest(const char* author, const char* secretKey, String message) {
        String encodedAuthor = urlEncode(author);
        String encodedMessage = urlEncode(message);
        String encodedKey = urlEncode(secretKey);
        
        return String("GET /~fasanicebox/createTweeticam.php?") +
               "author=" + encodedAuthor +
               "&secretkey=" + encodedKey +
               "&message=" + encodedMessage +
               " HTTP/1.1";
    }
};

#endif