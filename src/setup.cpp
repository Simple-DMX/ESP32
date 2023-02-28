#include <WiFi.h>
#include <SPIFFS.h>
#include <vector>
#include "Arduino.h"
#include "BluetoothSerial.h"
#include "setup.h"

#define WIFI_TIMEOUT 10
#define WIFI_FILE "/wifi.txt"

String bt = "";
bool connected = false;
char ssid[256];
char pass[512];

BluetoothSerial SerialBT;

void wifi_setup() {
    spiffs_setup();
    SerialBT.begin("SimpleDMX");
    WiFiClass::mode(WIFI_STA);
    wifi_read_file();
    bt = "";
}

void wifi_try_connect() {
    WiFi.begin(ssid, pass);
    SerialBT.print("Attempting connection with SSID:");
    SerialBT.print(ssid);
    SerialBT.print(" and PASS:");
    SerialBT.println(pass);
    SerialBT.flush();
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        wait++;
        if (++wait > WIFI_TIMEOUT) {
            SerialBT.println("Connection wasn't successful, please try again!");
            SerialBT.flush();
            wifi_read();
            return;
        }
    }
    SerialBT.print("WiFi was connected: ");
    SerialBT.println(WiFi.localIP());
    connected = true;
    File file = SPIFFS.open(WIFI_FILE, FILE_WRITE);
    file.println(ssid);
    file.println(pass);
    file.flush();
    file.close();
}

void wifi_read() {
    while (!connected) {
        if (SerialBT.available()) {
            char read = (char) SerialBT.read();
            if (read == '\n') {
                if (bt.indexOf("SSID ") >= 0) {
                    String ssid_str = bt.substring(5);
                    ssid_str.toCharArray(ssid, ssid_str.length());
                } else if (bt.indexOf("PASS ") >= 0) {
                    String pass_str = bt.substring(5);
                    pass_str.toCharArray(pass, pass_str.length());
                } else if (bt.indexOf("CONN_STAT") >= 0) {
                    SerialBT.print("WiFi Status (3 is connected): ");
                    SerialBT.println(WiFi.status());
                } else if (bt.indexOf("CONN") >= 0) {
                    wifi_try_connect();
                }
                bt = "";
            } else {
                bt += read;
            }
        }
        delay(20);
    }
}

void wifi_read_file() {
    File file = SPIFFS.open(WIFI_FILE);
    if (!file || file.size() == 0) {
        wifi_read();
        return;
    }
    std::vector<String> content;
    while (file.available()) {
        content.push_back(file.readStringUntil('\n'));
    }
    content[0].toCharArray(ssid, content[0].length());
    content[1].toCharArray(pass, content[1].length());
    file.close();
    wifi_try_connect();
}

void spiffs_setup() {
    if (!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
}
