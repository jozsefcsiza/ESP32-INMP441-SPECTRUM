# ESP32-INMP441-SPECTRUM
ESP32 INMP441 SPECTRUM WITH BLUETOOTH CONTROL
WS2812B 16x16 led matrix spectrum analyzer with bluetooth control via Android.

# HARDWARE
WS2812B led matrix.

    -> Replace the #define LED_PIN 5 in the ESP32-INMP441-SPECTRUM.ino to your desired led pin.
INMP441 I2S microphone.
    -> Replace the
    #define I2S_WS 15   // aka LRCL
    #define I2S_SD 32   // aka DOUT
    #define I2S_SCK 14  // aka BCLK
    in the audio_reactive.h file.
ESP32 dev board, the exact type doesn't matter.
Any android devce with bluetooth.
