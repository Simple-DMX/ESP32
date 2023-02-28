#include "Arduino.h"
#include "setup.h"

void setup() {
    Serial.begin(115200);
    wifi_setup();
}

void loop() {
}
