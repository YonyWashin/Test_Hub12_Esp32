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

<img width="653" height="616" alt="matrix led esp32" src="https://github.com/user-attachments/assets/b8dea364-7827-4021-a7b4-df8b99ba90d6" />

## Instalación en PlatformIO
## La libreria utiliza estos pines por Defecto 
```text
HUB 12 PANEL         ESP 32 PIN
+----------+
|  OE  A   |    OE  -> IO25    A -> IO22
|  N   B   |                   B -> IO23
|  N  GND  |                   GND 
|  N  CLK  |                   CLK -> IO27 
|  N  LAT  |                   LAT -> IO26
|  N  DAT  |                   DAT -> IO12
|  N  GND  |                   GND 
|  N  GND  |                   GND -> ESP32 GND
+----------+
```
si nesecita utilizar otros pines debe modificar dentro de la libreria 
Agrega el repositorio a tu archivo `platformio.ini`:
