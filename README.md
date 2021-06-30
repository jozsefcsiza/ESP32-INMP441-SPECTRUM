![icon](https://user-images.githubusercontent.com/61933721/123552174-983dc180-d77d-11eb-9d72-8daecaa46584.png)

# ESP32-INMP441-SPECTRUM
ESP32 spectrum analyzer with INMP441 I2S microphone with bluetooth control via any Android device.

# HARDWARE
WS2812B led matrix.

INMP441 I2S microphone.

ESP32 dev board, the exact type doesn't matter.

Any android devce with bluetooth.

# WIRING
![Wiring](https://user-images.githubusercontent.com/61933721/123543411-ef7c6b80-d756-11eb-9c7a-f273d759a34c.png)


# SOFTWARE

    -> Replace
    #define LED_PIN 2 in the ESP32-INMP441-SPECTRUM.ino file to your desired led pin of the matrix.

    -> Replace
    #define I2S_WS 15   // aka LRCL
    #define I2S_SD 32   // aka DOUT
    #define I2S_SCK 14  // aka BCLK
    in the audio_reactive.h file to your desired INMP441 microphone pins

Spectrum analyzer matrix patterns source code by [Scott Marley](https://github.com/s-marley/ESP32-INMP441-Matrix-VU)

INMP441 I2S audio process source code by [WLED sound reactive fork](https://github.com/atuline/WLED)

Bluetooth control Arduino and Android source code by me.

# BUILD AND RUN
Wire your ESP32 properly.

Upload the arduino code to your ESP32 device.

Install the Spectrum.apk tou your android device.

![Spectrum2](https://user-images.githubusercontent.com/61933721/123543784-df658b80-d758-11eb-9e7d-9141360e3ed1.png)

SPECTRUM SETTINGS - Accesss the spectrum settings.

SELECT BLUETOOTH - Select the esp32 bluetooth, it is saved and you don't have to reselect all the time.

ESP32 DEEP SLEEP - Pauses the audio process and sets the led brightness to 0, so only the bluetooth will remain in function and after restarting the android app the spectrum analyzer will be activated automatically. It is like a switch off, except the bluetooth.

![Spectrum1](https://user-images.githubusercontent.com/61933721/123284133-afc64180-d514-11eb-88a6-4419e1215f16.png)

NEXT PATTERN - You can switch between the built in patterns.

AUTO CHANGE - You can set the auto change in seconds. In every N seconds the pattern is changing.

BRIGHTNESS - Led brightness setting. It consumes amps. If you use it with PC or something, at maximum brightness can freeze the esp. A phone charger has enough amps to use it on maximum brightness.

GAIN - Increases the sensitivity of the microphone to adjust for louder or quieter environoments.

SQUELCH - Increasing this value puts a limit on the quietest sounds that will be picked up. Useful for if you have some background noise to remove.

RESET (DEFAULTS) - Resets all the spectrum settings.
