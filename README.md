<img width="1200" height="1600" alt="matrix led esp32" src="https://github.com/user-attachments/assets/23d3bd5d-b00f-4229-b030-9f162e8a5b02" />
# Test Hub12  contiente Librería HUB12 para ESP32

Librería optimizada para el control de matrices LED P10 (HUB12) utilizando ESP32, FreeRTOS y compatibilidad nativa con **Adafruit_GFX**.

## Características
- Control de paneles P10 mediante FreeRTOS en Core 1.
- Compatible con las funciones de dibujo de Adafruit GFX (líneas, texto, fuentes, rectángulos).
- Control de brillo por PWM.
- Maneja hasta 4 puertos hub12.
- Soporta hasta 40 Paneles
- 10 Paneles por cada puerto.
- Los Modulos P10 soportados y probados con IC(74HC595, DP5125C, ICN2037) y otro.
- MCU ESP-WROOM-32 de espressif.

## Instalación en PlatformIO

Agrega el repositorio a tu archivo `platformio.ini`:
