#include <Arduino.h>
#include "hub12.h"

// 2 paneles en X, 1 en Y -> 64x16 píxeles
hub12 display(2, 1);

void setup() {
    Serial.begin(115200);
    delay(200);

    display.begin();
    display.setBrightness(50);
    display.fillScreen(0);

    display.setTextColor(1);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("HUB12 OK");
    display.setCursor(0, 8);
    display.print("ESP32 P10");
    display.updateDisplay();

    Serial.println("Ejemplo 01: texto basico iniciado");
}

void loop() {
    // Nada: el texto queda fijo en pantalla
    delay(1000);
}