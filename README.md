# ESP32-INMP441-SPECTRUM
ESP32 INMP441 SPECTRUM WITH BLUETOOTH CONTROL
WS2812B 16x16 led matrix spectrum analyzer with bluetooth control via Android.

Youtube video by [Scott Marley](https://www.youtube.com/watch?v=9PEjvFkdpIE)

# HARDWARE
WS2812B led matrix.

    -> Replace the #define LED_PIN 5 in the ESP32-INMP441-SPECTRUM.ino to your desired led pin.
INMP441 I2S microphone.

    -> Replace the
    #define I2S_WS 15   // aka LRCL
    #define I2S_SD 32   // aka DOUT
    #define I2S_SCK 14  // aka BCLK
    in the audio_reactive.h file to your desired pins.
ESP32 dev board, the exact type doesn't matter.
Any android devce with bluetooth.

# SOFTWARE
It is a visual studio project and you can easily copy the necessary files to your arduino ide project.

Spectrum analyzer matrix patterns source code and wiring by [Scott Marley](https://github.com/s-marley/ESP32-INMP441-Matrix-VU)

INMP441 I2S source code by [WLED sound reactive fork](https://github.com/atuline/WLED)

Bluetooth control arduino and android source code by me.

# BUILD AND RUN
Wire your ESP32 properly.

Upload the arduino code.

Install the Spectrum.apk tou your android device.

![Spectrum1](https://user-images.githubusercontent.com/61933721/123283635-40e8e880-d514-11eb-9f5c-1d3c8222d49a.jpg)
![Spectrum2](https://user-images.githubusercontent.com/61933721/123283652-434b4280-d514-11eb-9d7d-b424968e906e.jpg)

