#include <Arduino.h>
#include "hub12.h"

#define PANELS_X 2
#define PANELS_Y 1

hub12 display(PANELS_X, PANELS_Y);

uint32_t frameCounter = 0;
uint32_t lastUpdate = 0;

void setup() {
    Serial.begin(115200);
    delay(200);

    display.begin();
    display.setBrightness(60);
    display.fillScreen(0);
    display.updateDisplay();

    Serial.println("Ejemplo 02: rectangulo en movimiento");
}

void loop() {
    if (millis() - lastUpdate >= 100) {
        lastUpdate = millis();
        frameCounter++;

        display.fillScreen(0);

        // Rectángulo que rebota horizontalmente
        int16_t x = (frameCounter * 4) % (PANELS_X * 32 - 8);
        display.drawRect(x, 4, 8, 8, 1);

        // Línea inferior de referencia
        display.drawFastHLine(0, (PANELS_Y * 16) - 1, PANELS_X * 32, 1);

        display.updateDisplay();
    }
}