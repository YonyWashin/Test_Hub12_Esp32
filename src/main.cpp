#include <Arduino.h>
#include "hub12.h"
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>


hub12 panel(3,2);//(largo,Alto)
TaskHandle_t scrollTaskHandle = NULL;

// Variables globales para el mensaje de scroll no bloqueante
char mensajeScroll[128] = "Matriz led P10 con ESP32";
int velocidadScrollMs = 35;
int posYScroll = 12;
// Variable global para almacenar el texto formateado
char textoNumero[8] = "0";

void scrollTask(void *parameter) {
    while(1) {
        int anchoMatriz = panel.width();

        int16_t x1, y1;
        uint16_t anchoTexto, altoTexto;
        panel.setFont(&FreeMono9pt7b);
        panel.getTextBounds(mensajeScroll, 0, posYScroll, &x1, &y1, &anchoTexto, &altoTexto);

        for (int x = anchoMatriz; x >= -anchoTexto; x--) {
            
            panel.setFont(&FreeMono9pt7b);
            panel.setTextSize(1);
            panel.setTextWrap(false);           
            panel.fillScreen(0);

            // --- 2. DIBUJAR TEXTO SCROLL SUPERIOR ---
            // Limpiar franja inferior (16 a 32)           
            panel.setCursor(x, posYScroll);
            panel.print(mensajeScroll);
            
            // --- 1. DIBUJAR NÚMERO INFERIOR ---
            // Limpiar franja inferior (16 a 32)
            panel.setFont(&FreeMonoBold12pt7b);
            //panel.fillRect(0, 16, anchoMatriz, 16, 0);
            panel.setCursor(32, 30);
            panel.print(textoNumero); // Dibujamos el valor calculado por TestNumeros

            // --- 3. REFRESCAR BÚFER (Una sola vez por cuadro) ---
            panel.updateDisplay();

            vTaskDelay(pdMS_TO_TICKS(velocidadScrollMs));
        }
    }
}
void TestNumeros() {
    static int contadorNumeros = 0;
    static unsigned long ultimoTiempoTest = 0;

    if (millis() - ultimoTiempoTest >= 100) {
        ultimoTiempoTest = millis();

        // Solo formateamos la cadena en memoria RAM
        snprintf(textoNumero, sizeof(textoNumero), "%d", contadorNumeros);

        contadorNumeros++;
        if (contadorNumeros > 199) contadorNumeros = 0;
    }
}
void setup() {
    Serial.begin(115200);           
    delay(100); 

    panel.begin();      
    xTaskCreatePinnedToCore(scrollTask, "ScrollTask", 4096, NULL, 1, &scrollTaskHandle, 0);
    panel.fillScreen(0);  // Limpiar pantalla (negro)    
    panel.setFont(&FreeMono9pt7b);
    panel.setTextSize(1);
    panel.setTextWrap(false);
    panel.setCursor(32, 30);
    panel.print("2026");
    panel.updateDisplay();
    panel.setBrightness(40);
    
    Serial.println("--- Sistema HUB12 Listo ---");
    Serial.println("Escribe un nuevo mensaje y presiona Enter:");

}
void loop() {
    if (Serial.available() > 0) {
        
            String textoRecibido = Serial.readStringUntil('\n');    
            textoRecibido.trim();

            // 3. FILTRO: Solo actualizar si el texto tiene al menos 2 caracteres útiles
            if (textoRecibido.length() >= 2) {
                snprintf(mensajeScroll, sizeof(mensajeScroll), "%s", textoRecibido.c_str());
                
                Serial.print("Mensaje actualizado a: ");
                Serial.println(mensajeScroll);
            }
     } 
     TestNumeros();
     //panel.updateDisplay();
     vTaskDelay(pdMS_TO_TICKS(10));

}

