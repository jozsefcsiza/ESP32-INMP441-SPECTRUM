/*
 * Bluetooth controlled FFT VU meter for a matrix, ESP32 and INMP441 digital mic.
 * The matrix width MUST be either 8 or a multiple of 16 but the height can
 * be any value. E.g. 8x8, 16x16, 8x10, 32x9 etc.
 *
 * We are using the LEDMatrx library for easy setup of a matrix with various
 * wiring options. Options are:
 *  HORIZONTAL_ZIGZAG_MATRIX
 *  HORIZONTAL_MATRIX
 *  VERTICAL_ZIGZAG_MATRIX
 *  VERTICAL_MATRIX
 * If your matrix has the first pixel somewhere other than the bottom left
 * (default) then you can reverse the X or Y axis by writing -M_WIDTH and /
 * or -M_HEIGHT in the cLEDMatrix initialisation.
 *
 * REQUIRED LIBRARIES
 * FastLED            Arduino libraries manager
 * ArduinoFFT         Arduino libraries manager
 * Preferences        Built in
 * LEDMatrix          https://github.com/AaronLiddiment/LEDMatrix
 *
 * WIRING
 * LED data     D2 via 470R resistor
 * GND          GND
 * Vin          5V
 *
 * INMP441
 * VDD          3V3
 * GND          GND
 * L/R          GND
 * WS           D15
 * SCK          D14
 * SD           D32
 *
 * REFERENCES
 * Main code            Scott Marley      https://www.youtube.com/c/ScottMarley
 * Bluetooth code       Jozsef Csiza
 * Audio and mic  Andrew Tuline et al     https://github.com/atuline/WLED
 */

#include "audio_reactive.h"
#include <FastLED.h>
#include <LEDMatrix.h>
#include <Preferences.h>
#include "my_bluetooth.h"

#define LED_PIN     2
#define M_WIDTH     16
#define M_HEIGHT    16
#define NUM_LEDS    (M_WIDTH * M_HEIGHT)

#define ESP_PREFS           "esp_prefs" //Namespace name is limited to 15 characters
#define PREFS_BRIGHTNESS    "0" //key
#define PREFS_GAIN          "1" //key
#define PREFS_SQUELCH       "2" //key
#define PREFS_PATTERN       "3" //key
#define PREFS_DISPLAY_TIME  "4" //key

uint8_t numBands;
uint8_t barWidth;
uint8_t pattern;
uint8_t brightness;
uint16_t displayTime;
bool autoChangePatterns = false, isDeepSlepp = false;

cLEDMatrix<M_WIDTH, M_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX> leds;

uint8_t peak[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
uint8_t prevFFTValue[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
uint8_t barHeights[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

// Colors and palettes
DEFINE_GRADIENT_PALETTE(purple_gp) {
    0, 0, 212, 255,   //blue
        255, 179, 0, 255
}; //purple
DEFINE_GRADIENT_PALETTE(outrun_gp) {
    0, 141, 0, 100,   //purple
        127, 255, 192, 0,   //yellow
        255, 0, 5, 255
};  //blue
DEFINE_GRADIENT_PALETTE(greenblue_gp) {
    0, 0, 255, 60,   //green
        64, 0, 236, 255,   //cyan
        128, 0, 5, 255,   //blue
        192, 0, 236, 255,   //cyan
        255, 0, 255, 60
}; //green
DEFINE_GRADIENT_PALETTE(redyellow_gp) {
    0, 200, 200, 200,   //white
        64, 255, 218, 0,   //yellow
        128, 231, 0, 0,   //red
        192, 255, 218, 0,   //yellow
        255, 200, 200, 200
}; //white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

//EEPROM library is deprecated in favor of the Preferences library, so we will use the Preferences library
Preferences preferences;

void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds[0], NUM_LEDS);
    Serial.begin(57600);

    startBlueTooth();
    setupAudio();

    if (M_WIDTH == 8) numBands = 8;
    else numBands = 16;
    barWidth = M_WIDTH / numBands;

    preferences.begin(ESP_PREFS, false); //The false argument means that we’ll use it in read/write mode
    brightness = preferences.getUInt(PREFS_BRIGHTNESS, 2);
    gain = preferences.getUInt(PREFS_GAIN, 30);
    squelch = preferences.getUInt(PREFS_SQUELCH, 0);
    pattern = preferences.getUInt(PREFS_PATTERN, 0);
    displayTime = preferences.getUInt(PREFS_DISPLAY_TIME, 1);
    preferences.end();
    if (displayTime > 1) autoChangePatterns = true;
}

void loop() {
    readBlueTooth();

    if (!isDeepSlepp) {
        if (pattern != 5) FastLED.clear();

        uint8_t divisor = 1;                                                    // If 8 bands, we need to divide things by 2
        if (numBands == 8) divisor = 2;                                         // and average each pair of bands together

        for (int i = 0; i < 16; i += divisor) {
            uint8_t fftValue;

            if (numBands == 8) fftValue = (fftResult[i] + fftResult[i + 1]) / 2;    // Average every two bands if numBands = 8
            else fftValue = fftResult[i];

            fftValue = ((prevFFTValue[i / divisor] * 3) + fftValue) / 4;            // Dirty rolling average between frames to reduce flicker
            barHeights[i / divisor] = fftValue / (255 / M_HEIGHT);                  // Scale bar height

            if (barHeights[i / divisor] > peak[i / divisor])                          // Move peak up
                peak[i / divisor] = min(M_HEIGHT, (int)barHeights[i / divisor]);

            prevFFTValue[i / divisor] = fftValue;                                   // Save prevFFTValue for averaging later

        }

        // Draw the patterns
        for (int band = 0; band < numBands; band++) {
            drawPatterns(band);
        }

        // Decay peak
        EVERY_N_MILLISECONDS(60) {
            for (uint8_t band = 0; band < numBands; band++)
                if (peak[band] > 0) peak[band] -= 1;
        }

        EVERY_N_SECONDS_I(timingObj, displayTime) {
            timingObj.setPeriod(displayTime);
            if (autoChangePatterns) pattern = (pattern + 1) % 6;
        }

        FastLED.setBrightness(brightness);
        FastLED.show();
    }
}

void saveSpectrumPrefs() {
    preferences.begin(ESP_PREFS, false); //The false argument means that we’ll use it in read/write mode
    preferences.putUInt(PREFS_BRIGHTNESS, brightness);
    preferences.putUInt(PREFS_GAIN, gain);
    preferences.putUInt(PREFS_SQUELCH, squelch);
    preferences.putUInt(PREFS_PATTERN, pattern);
    preferences.putUInt(PREFS_DISPLAY_TIME, displayTime);
    preferences.end();
}

void deepSlepp() {
    isDeepSlepp = true;
    pauseAudio();
    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();
}

void wakeUp() {
    if (isDeepSlepp) {
        isDeepSlepp = false;
        resumeAudio();
    }
}

void drawPatterns(uint8_t band) {

    uint8_t barHeight = barHeights[band];

    // Draw bars
    switch (pattern) {
    case 0:
        rainbowBars(band, barHeight);
        break;
    case 1:
        // No bars on this one
        break;
    case 2:
        purpleBars(band, barHeight);
        break;
    case 3:
        centerBars(band, barHeight);
        break;
    case 4:
        changingBars(band, barHeight);
        EVERY_N_MILLISECONDS(10) { colorTimer++; }
        break;
    case 5:
        createWaterfall(band);
        EVERY_N_MILLISECONDS(30) { moveWaterfall(); }
        break;
    }

    // Draw peaks
    switch (pattern) {
    case 0:
        whitePeak(band);
        break;
    case 1:
        outrunPeak(band);
        break;
    case 2:
        whitePeak(band);
        break;
    case 3:
        // No peaks
        break;
    case 4:
        // No peaks
        break;
    case 5:
        // No peaks
        break;
    }
}

//////////// Patterns ////////////

void rainbowBars(uint8_t band, uint8_t barHeight) {
    int xStart = barWidth * band;
    for (int x = xStart; x < xStart + barWidth; x++) {
        for (int y = 0; y <= barHeight; y++) {
            leds(x, y) = CHSV((x / barWidth) * (255 / numBands), 255, 255);
        }
    }
}

void purpleBars(int band, int barHeight) {
    int xStart = barWidth * band;
    for (int x = xStart; x < xStart + barWidth; x++) {
        for (int y = 0; y < barHeight; y++) {
            leds(x, y) = ColorFromPalette(purplePal, y * (255 / barHeight));
        }
    }
}

void changingBars(int band, int barHeight) {
    int xStart = barWidth * band;
    for (int x = xStart; x < xStart + barWidth; x++) {
        for (int y = 0; y < barHeight; y++) {
            leds(x, y) = CHSV(y * (255 / M_HEIGHT) + colorTimer, 255, 255);
        }
    }
}

void centerBars(int band, int barHeight) {
    int xStart = barWidth * band;
    for (int x = xStart; x < xStart + barWidth; x++) {
        if (barHeight % 2 == 0) barHeight--;
        int yStart = ((M_HEIGHT - barHeight) / 2);
        for (int y = yStart; y <= (yStart + barHeight); y++) {
            int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
            leds(x, y) = ColorFromPalette(heatPal, colorIndex);
        }
    }
}

void whitePeak(int band) {
    int xStart = barWidth * band;
    int peakHeight = peak[band];
    for (int x = xStart; x < xStart + barWidth; x++) {
        leds(x, peakHeight) = CRGB::White;
    }
}

void outrunPeak(int band) {
    int xStart = barWidth * band;
    int peakHeight = peak[band];
    for (int x = xStart; x < xStart + barWidth; x++) {
        leds(x, peakHeight) = ColorFromPalette(outrunPal, peakHeight * (255 / M_HEIGHT));
    }
}

void createWaterfall(int band) {
    int xStart = barWidth * band;
    // Draw bottom line
    for (int x = xStart; x < xStart + barWidth; x++) {
        leds(x, 0) = CHSV(constrain(map(fftResult[band], 0, 254, 160, 0), 0, 160), 255, 255);
    }
}

void moveWaterfall() {
    // Move screen up starting at 2nd row from top
    for (int y = M_HEIGHT - 2; y >= 0; y--) {
        for (int x = 0; x < M_WIDTH; x++) {
            leds(x, y + 1) = leds(x, y);
        }
    }
}

#pragma region Bluetooth
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
    if (event == ESP_SPP_SRV_OPEN_EVT) {
        Serial.println("Client Connected");
    }
}

void startBlueTooth() {
    SerialBT.register_callback(callback);

    if (!SerialBT.begin(BLUETOOTH_NAME)) {
        Serial.println("An error occurred initializing Bluetooth");
    }
    else {
        Serial.println("Bluetooth initialized");
    }
}

void getBtValue(char c1, char c2, char c3) {
    try
    {
        char str[3] = { c1, c2, c3 };
        bluetoothData = atoi(str);
    }
    catch (const std::exception&)
    {
        bluetoothData = O;
    }
}

void readBlueTooth() {
    BT_BUTTON = "";
    if (SerialBT.available())
    {
        int i = 0;
        char byteArray[6] = { 0, 0, 0, 0, 0, 0 };
        while (SerialBT.available())
        {
            if (i < 6) {
                byteArray[i] = SerialBT.read();
                i += 1;
            }
            else {
                SerialBT.read();
            }
        }
        BT_BUTTON = "";
        BT_BUTTON = BT_BUTTON + byteArray[0] + byteArray[1] + byteArray[2];
        BT_BUTTON.trim();
        if (BT_BUTTON.equals(BT_SPECTRUM_NEXT_PATTERN)) {
            pattern = (pattern + 1) % 6;
            autoChangePatterns = false;
            displayTime = 1;
            saveSpectrumPrefs();
        }
        else if (BT_BUTTON.equals(BT_SPECTRUM_AUTO_PATTERN)) {
            getBtValue(byteArray[3], byteArray[4], byteArray[5]);
            if (bluetoothData != O) {
                displayTime = bluetoothData;
                if (displayTime == 1)
                    autoChangePatterns = false;
                else
                    autoChangePatterns = true;
                saveSpectrumPrefs();
            }
        }
        else if (BT_BUTTON.equals(BT_SPECTRUM_BRIGHTNESS)) {
            getBtValue(byteArray[3], byteArray[4], byteArray[5]);
            if (bluetoothData != O) {
                brightness = bluetoothData;
                saveSpectrumPrefs();
            }
        }
        else if (BT_BUTTON.equals(BT_SPECTRUM_GAIN)) {
            getBtValue(byteArray[3], byteArray[4], byteArray[5]);
            if (bluetoothData != O) {
                gain = bluetoothData;
                saveSpectrumPrefs();
            }
        }
        else if (BT_BUTTON.equals(BT_SPECTRUM_SQUELCH)) {
            getBtValue(byteArray[3], byteArray[4], byteArray[5]);
            if (bluetoothData != O) {
                squelch = bluetoothData;
                saveSpectrumPrefs();
            }
        }
        else if (BT_BUTTON.equals(BT_SPECTRUM_RESET)) {
            brightness = 2;
            gain = 30;
            squelch = 0;
            pattern = 0;
            displayTime = 1;
            autoChangePatterns = false;
            saveSpectrumPrefs();
        }
        else if (BT_BUTTON.equals(BT_GET_ESP_DATA)) {
            wakeUp();
            SerialBT.println(BT_SPECTRUM_BRIGHTNESS + brightness);
            delay(10);
            SerialBT.println(BT_SPECTRUM_GAIN + gain);
            delay(10);
            SerialBT.println(BT_SPECTRUM_SQUELCH + squelch);
            delay(10);
            SerialBT.println(BT_SPECTRUM_AUTO_PATTERN + displayTime);
        }
        else if (BT_BUTTON.equals(BT_ESP_DEEP_SLEEP)) {
            deepSlepp();
        }
    }
}
#pragma endregion
