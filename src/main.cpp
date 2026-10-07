#include <Arduino.h>
#include <FastLED.h>
#include "definitions.h"

// Define the array of leds
CRGB leds[NUM_LEDS];

int currentPattern = 0;
const int NUM_PATTERNS = 8;
bool p5Started = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- Device Booted Successfully ---");
    Serial.print("LED Pin: GPIO ");
    Serial.print(LED_DATA_PIN);
    Serial.print(" | Total LEDs: ");
    Serial.println(NUM_LEDS);

    FastLED.setBrightness(60);
    CFastLED::addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void checkButton() {
    static int buttonState = HIGH;
    static int lastReading = HIGH;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;

    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastReading) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                if (currentPattern == 5 && !p5Started) {
                    p5Started = true;
                    Serial.println("Pattern 5 started by button press");
                } else {
                    currentPattern = (currentPattern + 1) % NUM_PATTERNS;
                    p5Started = false;
                    Serial.print("Pattern changed to: ");
                    Serial.println(currentPattern);
                    FastLED.clear();
                    FastLED.show();
                }
            }
        }
    }

    lastReading = reading;
}

// Pattern 0: White Group Chase
void pattern0() {
    static unsigned long lastUpdate = 0;
    static int i = 0;

    if (millis() - lastUpdate > 150) {
        lastUpdate = millis();
        FastLED.clear();

        for (int g = 0; g < NUM_GROUPS; g++) {
            int headPos = (i + g * 2) % GROUP_SIZE;

            for (int t = 0; t <= 4; t++) {
                int localPos = (headPos + t) % GROUP_SIZE;
                int globalIdx = g * GROUP_SIZE + localPos;

                CRGB color = CRGB::White;
                if (t == 1) {
                    color.nscale8(180);
                } else if (t == 2) {
                    color.nscale8(120);
                } else if (t == 3) {
                    color.nscale8(50);
                } else if (t == 4) {
                    color.nscale8(10);
                }

                if (globalIdx < NUM_LEDS) {
                    leds[globalIdx] = color;
                }
            }
        }
        FastLED.show();
        i = (i - 1 + GROUP_SIZE) % GROUP_SIZE;
    }
}

// Pattern 1: Rainbow Cycle
void pattern1() {
    static unsigned long lastUpdate = 0;
    static uint8_t hue = 0;

    if (millis() - lastUpdate > 50) {
        lastUpdate = millis();
        fill_rainbow(leds, NUM_LEDS, hue, 7);
        FastLED.show();
        hue += 2;
    }
}

// Pattern 2: Cyan Chase
void pattern2() {
    static unsigned long lastUpdate = 0;
    static int i = 0;

    if (millis() - lastUpdate > 25) {
        lastUpdate = millis();
        FastLED.clear();

        // Chase from low (0) to high (NUM_LEDS-1) across the entire row
        for (int t = 0; t <= 16; t++) {
            int idx = (i - t + NUM_LEDS) % NUM_LEDS;
            CRGB color = CRGB::Cyan;
            if (t > 0) {
                int scale = 255 - (t * 15);
                if (scale < 0) scale = 0;
                color.nscale8(scale);
            }
            leds[idx] = color;
        }
        FastLED.show();
        i = (i + 1) % NUM_LEDS;
    }
}

// Pattern 3: Magenta Pulse
void pattern3() {
    static unsigned long lastUpdate = 0;
    static uint8_t brightnessScale = 0;
    static int delta = 2;

    if (millis() - lastUpdate > 15) {
        lastUpdate = millis();

        CRGB color = CRGB::Magenta;
        color.nscale8(brightnessScale);
        fill_solid(leds, NUM_LEDS, color);
        FastLED.show();

        brightnessScale += delta;
        if (brightnessScale == 0 || brightnessScale >= 252) {
            delta = -delta;
        }
    }
}

// Pattern 4: Stepping Color Groups
void pattern4() {
    static unsigned long lastUpdate = 0;
    static int currentGroup = 0;

    if (millis() - lastUpdate > 300) {
        lastUpdate = millis();
        FastLED.clear();

        for (int t = 0; t < NUM_GROUPS; t++) {
            int g = (currentGroup - t + NUM_GROUPS) % NUM_GROUPS;
            CRGB color = CHSV(g * (255 / NUM_GROUPS), 255, 255);
            if (t == 1) {
                color.nscale8(180);
            } else if (t == 2) {
                color.nscale8(120);
            } else if (t == 3) {
                color.nscale8(50);
            }

            for (int localPos = 0; localPos < GROUP_SIZE; localPos++) {
                int globalIdx = g * GROUP_SIZE + localPos;
                if (globalIdx < NUM_LEDS) {
                    leds[globalIdx] = color;
                }
            }
        }
        FastLED.show();
        currentGroup = (currentGroup + 1) % NUM_GROUPS;
    }
}

// Pattern 5: Staged Group Build-Up
void pattern5() {
    enum P5State { P5_BUILD, P5_HOLD, P5_FADEDOWN };
    static P5State state = P5_BUILD;
    static unsigned long lastUpdate = 0;
    static int currentGroup = 0;
    static int fadeBrightness = 0;
    static int p5MaxBrightness = 40; // Max brightness setting (0-100)
    static unsigned long holdStartTime = 0;
    static int fadeDownProgress = 0;

    if (!p5Started) {
        state = P5_BUILD;
        currentGroup = 0;
        fadeBrightness = 0;
        fadeDownProgress = 0;
        return;
    }

    if (state == P5_HOLD) {
        if (millis() - holdStartTime > 1500) {
            state = P5_FADEDOWN;
            fadeDownProgress = 0;
            lastUpdate = millis();
        }
        return;
    }

    if (state == P5_FADEDOWN) {
        if (millis() - lastUpdate > 25) {
            lastUpdate = millis();

            uint8_t startVal = (255 * p5MaxBrightness) / 100;
            CRGB startColor = CRGB(startVal, startVal, startVal);
            CRGB targetColor = CRGB(10, 10, 15); // Pattern 6 base background color
            CRGB currentColor = blend(startColor, targetColor, (uint8_t)fadeDownProgress);

            fill_solid(leds, NUM_LEDS, currentColor);
            FastLED.show();

            fadeDownProgress += 3;
            if (fadeDownProgress > 255) {
                // Ensure leds are exactly at Pattern 6 background color
                fill_solid(leds, NUM_LEDS, targetColor);
                FastLED.show();

                // Seamlessly transition into Pattern 6
                currentPattern = 6;
                p5Started = false;
                state = P5_BUILD;
                currentGroup = 0;
                fadeBrightness = 0;
                fadeDownProgress = 0;
                Serial.println("Pattern 5 completed; transitioning smoothly to Pattern 6");
            }
        }
        return;
    }

    if (state == P5_BUILD) {
        if (millis() - lastUpdate > 25) {
            lastUpdate = millis();
            FastLED.clear();

            // Previously fully lit groups
            for (int g = 0; g < currentGroup; g++) {
                CRGB color = CRGB::White;
                color.nscale8((255 * p5MaxBrightness) / 100);
                for (int localPos = 0; localPos < GROUP_SIZE; localPos++) {
                    int globalIdx = g * GROUP_SIZE + localPos;
                    if (globalIdx < NUM_LEDS) {
                        leds[globalIdx] = color;
                    }
                }
            }

            // Currently fading group
            if (currentGroup < NUM_GROUPS) {
                CRGB color = CRGB::White;
                color.nscale8((fadeBrightness * p5MaxBrightness) / 100);
                for (int localPos = 0; localPos < GROUP_SIZE; localPos++) {
                    int globalIdx = currentGroup * GROUP_SIZE + localPos;
                    if (globalIdx < NUM_LEDS) {
                        leds[globalIdx] = color;
                    }
                }

                fadeBrightness += 10;
                if (fadeBrightness >= 255) {
                    fadeBrightness = 0;
                    currentGroup++;
                    if (currentGroup >= NUM_GROUPS) {
                        state = P5_HOLD;
                        holdStartTime = millis();
                    }
                }
            }
            FastLED.show();
        }
    }
}

// Pattern 6: Fast Stars
void pattern6() {
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate > 70) {
        lastUpdate = millis();

        // Fade existing LEDs towards the base dim background
        for (auto & led : leds) {
            led.nscale8(180);
            if (led.r < 10) led.r = 10;
            if (led.g < 10) led.g = 10;
            if (led.b < 15) led.b = 15;
        }

        // Randomly light up a few bright blinking stars
        int numStars = random8(2, 6);
        for (int s = 0; s < numStars; s++) {
            int idx = random16(NUM_LEDS);
            if (random8() < 160) {
                leds[idx] = CRGB::White;
            } else {
                leds[idx] = CHSV(random8(32, 96), 120, 255);
            }
        }

        FastLED.show();
    }
}

// Pattern 7: Slow Stars
void pattern7() {
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate > 250) {
        lastUpdate = millis();

        // Fade existing LEDs at a slower rate towards the base dim background
        for (auto & led : leds) {
            led.nscale8(220);
            if (led.r < 10) led.r = 10;
            if (led.g < 10) led.g = 10;
            if (led.b < 15) led.b = 15;
        }

        // Randomly light up a few calm blinking stars
        int numStars = random8(1, 4);
        for (int s = 0; s < numStars; s++) {
            int idx = random16(NUM_LEDS);
            if (random8() < 160) {
                leds[idx] = CRGB::White;
            } else {
                leds[idx] = CHSV(random8(32, 96), 120, 255);
            }
        }

        FastLED.show();
    }
}

void loop() {
    checkButton();
    if (currentPattern == 0) {
        pattern0();
    } else if (currentPattern == 1) {
        pattern1();
    } else if (currentPattern == 2) {
        pattern2();
    } else if (currentPattern == 3) {
        pattern3();
    } else if (currentPattern == 4) {
        pattern4();
    } else if (currentPattern == 5) {
        pattern5();
    } else if (currentPattern == 6) {
        pattern6();
    } else {
        pattern7();
    }
}