#include "hub12.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "soc/ledc_reg.h"
#include "esp_system.h"

// ================= STATIC =================
volatile uint8_t hub12::_row = 3;
volatile bool hub12::scan_done = false;
uint8_t* hub12::buferScreen = nullptr;
// Wrapper estático: recibe la instancia de la clase a través de 'parameter'
void hub12::scanTaskWrapper(void *parameter) {  
    hub12 *panelInstance = (hub12*)parameter;
    // IMPORTANTE: Esperar 100ms antes de empezar a refrescar 
    // para dejar que el setup() del main.cpp termine completamente.
    vTaskDelay(pdMS_TO_TICKS(100));

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // Refresco cada 1ms

    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        panelInstance->refresh(); // Llama al método de la instancia
    }
}
// ================= ISR FADE END =================
bool IRAM_ATTR hub12::ledc_fade_cb(const ledc_cb_param_t *param, void *user_arg)
{
    scan_done = true;
    return true;
}

// ================= CONSTRUCTOR =================
hub12::hub12(byte panelsX, byte panelsY): 
    Adafruit_GFX(panelsX * 32, panelsY * 16)  // Llamar al constructor padre
{   
  
    _w = panelsX * 32;
    _h = panelsY * 16;
    _buf_len = panelsX * (FILAS_P10*COLUMNAS_P10);   
    buferScreen = (uint8_t*)heap_caps_malloc(_buf_len, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);  
    _fb = (uint8_t*)heap_caps_malloc((_w * _h) / 8, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);  
    // Inicializar buffer GFX
    if (_fb) { memset(_fb, 0, (_w * _h) / 8); }
}

// ================= INIT =================
void hub12::begin()
{
    setCpuFrequencyMhz(240);
    for (int i = 12; i <= 19; i++) pinMode(i, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(LAT_PIN, OUTPUT);
    pinMode(B_PIN, OUTPUT);
    pinMode(A_PIN, OUTPUT);
    pinMode(OE_PIN, OUTPUT);

    memset(_fb, 0, (_w * _h) / 8);
    clearDisplay(0);
    //buferScreen[0]=0xFE;// test     
    setupPWM();
    // 2. Crear automáticamente la tarea de escaneo en el Core 1
    if (scanTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            hub12::scanTaskWrapper, // Función wrapper
            "HUB12_ScanTask",       // Nombre de la tarea
            4096,                   // Tamaño del Stack
            this,                   // Pasamos la instancia actual (this)
            2,                      // Prioridad alta
            &scanTaskHandle,        // Handle
            1                       // Fijado al Core 1
        );
    }
}

// ================= PWM =================
void hub12::setupPWM()
{
    ledc_timer_config_t timer = {};
    timer.speed_mode       = HUB12_PWM_MODE;
    timer.timer_num        = HUB12_PWM_TIMER;
    timer.duty_resolution  = HUB12_PWM_RES;
    timer.freq_hz          = HUB12_PWM_FREQ;
    timer.clk_cfg          = LEDC_AUTO_CLK;
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {};
    ch.gpio_num   = HUB12_PWM_GPIO;
    ch.speed_mode = HUB12_PWM_MODE;
    ch.channel    = HUB12_PWM_CHANNEL;
    ch.timer_sel  = HUB12_PWM_TIMER;
    ch.duty       = _duty;
    ch.hpoint     = 0;
    ch.intr_type  = LEDC_INTR_FADE_END;
    ch.flags.output_invert = 1;
    ledc_channel_config(&ch);

    ledc_fade_func_install(0);

    ledc_cbs_t cbs = {};
    cbs.fade_cb = ledc_fade_cb;
    ledc_cb_register(HUB12_PWM_MODE, HUB12_PWM_CHANNEL, &cbs, nullptr);

    ledc_set_fade_time_and_start(
        HUB12_PWM_MODE,
        HUB12_PWM_CHANNEL,
        _duty,
        HUB12_PULSE_US,
        LEDC_FADE_NO_WAIT
    );
}

// ================= BRILLO =================
void hub12::setBrightness(uint8_t bright)
{
    if (bright > 100) bright = 100;    
     bright = 100 - bright;
     _duty = map(bright, 0, 100, 0, 255);
     // Actualizar PWM inmediatamente
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, _duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

// ================= SEND PARALLEL =================
void IRAM_ATTR hub12::send_parallel(uint8_t *buf, int len)
{
    for (int i = 0; i < len; i++) {
        uint32_t v = ((uint32_t)buf[i]) << DATA_SHIFT;
        GPIO.out_w1tc = (0xFF << DATA_SHIFT);
        GPIO.out_w1ts = v;
        GPIO.out_w1ts = CLK_BIT;
        GPIO.out_w1tc = CLK_BIT;
    }
}

// ================= REFRESH =================
void IRAM_ATTR hub12::refresh()
{
    if (!scan_done) return;
    scan_done = false;   
    GPIO.out_w1ts = LAT_BIT;
    GPIO.out_w1tc = LAT_BIT;

    switch (_row) {
        case 0: GPIO.out_w1tc = A_BIT | B_BIT; break;
        case 1: GPIO.out_w1ts = A_BIT; GPIO.out_w1tc = B_BIT; break;
        case 2: GPIO.out_w1tc = A_BIT; GPIO.out_w1ts = B_BIT; break;
        case 3: GPIO.out_w1ts = A_BIT | B_BIT; break;
    }

    _row = (_row + 1) & 0x03;
    uint32_t offset = _row * (_buf_len/4);
    send_parallel(buferScreen + offset, _buf_len/4);

    ledc_set_fade_time_and_start(
        HUB12_PWM_MODE,
        HUB12_PWM_CHANNEL,
        _duty,
        HUB12_PULSE_US,
        LEDC_FADE_NO_WAIT
    );
}

/*******************************************************************************
*   Escribe un bit en bufer displayRam segun cordenadas                        *
*   Parameter:    Xpos, Ypos, fill                                             *
*   Return:                                                                    *
*******************************************************************************/
/*void hub12::placePixel(uint16_t xpos, uint16_t ypos, uint16_t pixeldata){ // max posicion(0..95,0...63)
	
    if (xpos >= _w || ypos >= _h) return;
    uint32_t xx,y1=ypos%4,y2=ypos%16;y2=y2/4;

	xx = y1*(_buf_len/4) + (24-(8*y2));
	xx = xx + 32*(xpos/8) + xpos%8;

    if(pixeldata) buferScreen[xx]&=~(1<<(ypos/16));
    else buferScreen[xx]|= 1<<(ypos/16);
}*/
void hub12::placePixel(uint16_t x, uint16_t y, uint16_t c)
{
    if (x >= _w || y >= _h) return;

    uint32_t y1 = y & 0x03;
    uint32_t y2 = (y >> 2) & 0x03;

    uint32_t idx =
        y1 * (_buf_len / 4) +
        (24 - (8 * y2)) +
        32 * (x >> 3) +
        (x & 7);

    if (c)
        buferScreen[idx] &= ~(1 << (y >> 4));
    else
        buferScreen[idx] |=  (1 << (y >> 4));
}
/*******************************************************************************
*   Clear_display: Apaga los leds de la matriz segun parametro                 *
*   Parameter:    fill = 1: al screen ON, fill= 0: all screen OFF              *
*   Return:                                                                    *
*******************************************************************************/
void hub12::clearDisplay(bool fill){
    memset(buferScreen, fill ? 0x00 : 0xFF, _buf_len);
}
/*******************************************************************************
*   rellena de cero o unos en un sector de bufer                               *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/
void hub12::clearDisplayOnly(uint16_t x1, uint16_t y1, uint16_t sizeV,uint16_t sizeH, bool fill){
	   uint16_t  i;
	for(i=0;i<sizeH;i++){
	    drawFastVLine(x1+i, y1, sizeV, fill);
	}
}

// ================= MÉTODO OBLIGATORIO: drawPixel =================
void hub12::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Verificar límites
    if (x < 0 || x >= _w || y < 0 || y >= _h) return;
    
    // Convertir coordenadas GFX a tu formato P10
    placePixel(x, y, color > 0);
    
    // También actualizar buffer GFX para otras operaciones
    if (_fb) {
        uint16_t byteIndex = x + (y / 8) * _w;
        uint8_t bitMask = 1 << (y & 7);
        
        if (color > 0) {
            _fb[byteIndex] |= bitMask;  // Pixel ON
        } else {
            _fb[byteIndex] &= ~bitMask; // Pixel OFF
        }
    }
}

// ================= MÉTODO OBLIGATORIO: fillScreen =================
void hub12::fillScreen(uint16_t color) {
    // Llamar a tu función existente
    clearDisplay(color > 0);
    
    // Actualizar buffer GFX
    if (_fb) {
        memset(_fb, color > 0 ? 0xFF : 0x00, (_w * _h) / 8);
    }
}

// ================= MÉTODOS OPTIMIZADOS (mejoran rendimiento) =================

void hub12::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    // Límites
    if (y < 0 || y >= _h) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > _w) { w = _w - x; }
    if (w <= 0) return;
    
    // Dibujar línea horizontal optimizada
    for (int16_t i = 0; i < w; i++) {
        drawPixel(x + i, y, color);
    }
}

void hub12::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    // Límites
    if (x < 0 || x >= _w) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > _h) { h = _h - y; }
    if (h <= 0) return;
    
    // Dibujar línea vertical optimizada
    for (int16_t i = 0; i < h; i++) {
        drawPixel(x, y + i, color);
    }
}

void hub12::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Límites
    if (x >= _w || y >= _h || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > _w) { w = _w - x; }
    if (y + h > _h) { h = _h - y; }
    
    // Rellenar rectángulo optimizado
    for (int16_t j = 0; j < h; j++) {
        drawFastHLine(x, y + j, w, color);
    }
}

// ================= CONVERSIÓN DE BUFFER GFX A FORMATO P10 =================
void hub12::updateDisplay() {
    if (!_fb) return;
    
    // Convertir buffer GFX (_fb) a tu formato P10 (buferScreen)
    for (uint16_t y = 0; y < _h; y++) {
        for (uint16_t x = 0; x < _w; x++) {
            uint16_t byteIndex = x + (y / 8) * _w;
            uint8_t bitMask = 1 << (y & 7);
            bool pixel = _fb[byteIndex] & bitMask;
            placePixel(x, y, pixel);
        }
    }
}
