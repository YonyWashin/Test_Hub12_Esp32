# Test Hub12  contiente Librería HUB12 para ESP32

Librería optimizada para el control de matrices LED P10 (HUB12) utilizando ESP32, FreeRTOS y compatibilidad nativa con **Adafruit_GFX**.

Al principio el proyecto utilizaba el SPI, DMA y Interrupciones para tranferir datos a los paneles P10, pero vemos que no es muy agradable utilizar interrupciones para refrescar la pantalla ademas si queriamos mapear una pantalla led de 4 filas por 5 columnas de modulos nesecitabamos grandes cables de datos IDC. 
La otra opcion que teniamos es utilizar el I2S en paralelo a los pines GPIO con DMA, sin consumir ciclos de CPU, pero encontramos un inconveniente, los paneles no todos son fabricados con el mismo tipo de IC algunos traen la 74hc que no eran compatibles, actualmente en el mercado existen mas con IC tipo DP, para hacer compatible con ambos tipos de IC obtamos por usar FreeRTOS para que una de las CPU este continuamente refrescando la pantalla led P10. Talvez mas adelante desaparescan los modulos p10 basados en IC tipo 74HC y actulizamos la libreria para usar I2S y DMA, sin la intervencion de la CPU de ESP32.
 
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
