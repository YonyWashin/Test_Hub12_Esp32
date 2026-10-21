#pragma once
#include <Arduino.h>
#include "driver/ledc.h"
#include <Adafruit_GFX.h>
// ================= CONFIG HARDWARE =================
#define CLK_PIN     27
#define LAT_PIN     26//21
#define OE_PIN      25//23
#define A_PIN       22//26
#define B_PIN       23//25

#define DATA_SHIFT   12            // GPIO12..19
#define CLK_BIT    (1 << CLK_PIN)
#define LAT_BIT    (1 << LAT_PIN)
#define OE_BIT     (1 << OE_PIN)
#define A_BIT      (1 << A_PIN)
#define B_BIT      (1 << B_PIN)

#define HUB12_PWM_GPIO     OE_PIN
#define HUB12_PWM_TIMER    LEDC_TIMER_0
#define HUB12_PWM_CHANNEL  LEDC_CHANNEL_0
#define HUB12_PWM_MODE     LEDC_HIGH_SPEED_MODE
#define HUB12_PWM_FREQ     1000
#define HUB12_PWM_RES      LEDC_TIMER_8_BIT
#define HUB12_PULSE_US     50

#define FILAS_P10				  16 	// Filas modulo P10
#define COLUMNAS_P10			32	// Columnas  modulo P10

// ================= HUB12 CLASS =====================
class hub12 : public Adafruit_GFX 
{
  public:
    // Constructor
    hub12(byte panelsX, byte panelsY);

    // Inicializa GPIO + PWM
    void begin();

    // Actualiza brillo (0–100)
    void setBrightness(uint8_t bright);

    // Llamar cuando termina el PWM (fade end)
    void IRAM_ATTR refresh();
    void clearDisplay     (bool fill);
    void clearDisplayOnly (uint16_t x1, uint16_t y1, uint16_t sizeV,uint16_t sizeH,bool fill);
    void placePixel		    (uint16_t xpos, uint16_t ypos, uint16_t pixeldata);  
   // Métodos obligatorios de Adafruit_GFX
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    
    // Métodos optimizados (opcionales, mejoran rendimiento)
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    // Conversión de buffer GFX a tu formato P10
    void updateDisplay();
 
  private:
    byte ALTO_SCREEN,LARGO_SCREEN,HUB12_Ports;
    byte BYTES_FILA_SCREEN;
    uint8_t *_fb;               // framebuffer monocromo
    uint16_t _w, _h;

    uint32_t _duty=240;
    uint16_t _buf_len;

    static volatile uint8_t _row;
    static volatile bool scan_done;
    static uint8_t* buferScreen;
    TaskHandle_t scanTaskHandle = NULL;

    static void scanTaskWrapper(void *parameter);
    static void IRAM_ATTR send_parallel(uint8_t *buf, int len);
    static bool IRAM_ATTR ledc_fade_cb(const ledc_cb_param_t *param, void *user_arg);
    void setupPWM();
  
};
