# ESP32 & FastLED Controller Project

A multi-target FastLED project supporting Arduino Uno, Nano, and ESP32 with multi-pattern animations and optimal GPIO pin configurations for multi-strip addressable LED driving.

---

## ESP32 FastLED Pin Allocation Guide

### Recommended 10 LED Output Pins
When driving multiple strips (using FastLED's default RMT driver or I2S DMA parallel mode), the following **10 GPIOs** are optimal:

* `GPIO 13`
* `GPIO 14`
* `GPIO 16` *(Safe on ESP32-WROOM; reserved for PSRAM on WROVER)*
* `GPIO 17` *(Safe on ESP32-WROOM; reserved for PSRAM on WROVER)*
* `GPIO 18` *(Default LED data pin)*
* `GPIO 19`
* `GPIO 23`
* `GPIO 25`
* `GPIO 26`
* `GPIO 27`

#### Key Advantages
* **Zero Boot Interference:** None of these are strapping pins, preventing bootloader glitches and power-on LED flash.
* **Full Matrix & DMA Compatibility:** Works seamlessly with both FastLED RMT channels and 16-channel I2S parallel DMA.
* **Preserves Hardware Buses:** Leaves default I2C (`GPIO 21`, `GPIO 22`) and UART (`GPIO 1`, `GPIO 3`) completely open.

---

### Remaining GPIOs Breakdown (16 Usable Pins Left)

| Category | Pins | Count | Best Utilization & Purpose |
| :--- | :--- | :---: | :--- |
| **Dedicated Input-Only (GPI)** | `GPIO 34`, `35`, `36` (VP), `39` (VN) | **4** | Analog sensors, potentiometers, light sensors, or push buttons (requires external pull-up/down resistors). |
| **I2C Bus** | `GPIO 21` (SDA), `GPIO 22` (SCL) | **2** | OLED/LCD status displays, RTC clocks, IMUs, or environmental sensors. |
| **Primary Serial / Flashing** | `GPIO 1` (TX0), `GPIO 3` (RX0) | **2** | Hardware UART, Serial monitor, and USB flashing. |
| **General Purpose I/O** | `GPIO 32` *(Default Button)*, `GPIO 4`, `GPIO 33` | **3** | Buttons, rotary encoders, capacitive touch inputs, or ADC1 analog inputs. |
| **Strapping Pins (Caution)** | `GPIO 0`, `GPIO 2`, `GPIO 5`, `GPIO 12`, `GPIO 15` | **5** | Secondary buttons or digital triggers (avoid holding non-default logic levels during boot/reset). |

---

### Pins That CANNOT Be Used for LED Output

| Pin Range / Pins | Reason |
| :--- | :--- |
| **`GPIO 34`, `35`, `36` (VP), `39` (VN)** | **Input-only:** Silicon hardware has no output drivers. |
| **`GPIO 6`, `7`, `8`, `9`, `10`, `11`** | **Integrated SPI Flash:** Connected directly to onboard flash memory; using these will crash the MCU. |
| **`GPIO 20`, `24`, `28`, `29`, `30`, `31`** | **Invalid / Not Exposed:** Not available on the ESP32 silicon package. |

---

### FastLED ESP32 Driver Modes & Capacity

1. **Default RMT Driver:**
   - **Classic ESP32:** 8 hardware TX channels (up to 8 individual outputs).
   - **ESP32-S2 / ESP32-S3:** 4 hardware TX channels.
   - **ESP32-C3 / ESP32-C6 / ESP32-H2:** 2 hardware TX channels.

2. **I2S DMA Parallel Mode:**
   - Drives up to **16 individual outputs** in parallel with minimal CPU overhead.
   - Enable by defining `#define FASTLED_ESP32_I2S true` before `#include <FastLED.h>`.

```cpp
#define FASTLED_ESP32_I2S true
#include <FastLED.h>

#define LEDS_PER_STRIP 64

CRGB strip1[LEDS_PER_STRIP];
CRGB strip2[LEDS_PER_STRIP];

void setup() {
    FastLED.addLeds<WS2812B, 18, GRB>(strip1, LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, 19, GRB>(strip2, LEDS_PER_STRIP);
}
```

---

## Animation Patterns Included

The firmware cycles through 8 preset patterns via the pushbutton (`BUTTON_PIN`):

- **Pattern 0 — White Group Chase:** Stepped white chasing pulses across segmented groups.
- **Pattern 1 — Rainbow Cycle:** Smooth rainbow spectrum flowing along the entire strip.
- **Pattern 2 — Cyan Chase:** Fast, high-density cyan pulse with a smooth trailing fade.
- **Pattern 3 — Magenta Pulse:** Breathing/pulsing magenta ambient glow.
- **Pattern 4 — Stepping Color Groups:** Discrete color blocks cycling across grouped LEDs.
- **Pattern 5 — Staged Group Build-Up:** Interactive multi-phase staged brightness build-up.
- **Pattern 6 — Fast Stars:** Fast dynamic twinkling starfield over soft blue starlight.
- **Pattern 7 — Slow Stars:** Relaxed, slow twinkling stars over a soft background glow.

---

## Hardware Targets & Building (PlatformIO)

Build and upload for any supported environment using PlatformIO:

```bash
# Arduino Uno
pio run -e uno

# Arduino Nano
pio run -e nano

# ESP32 Dev Module
pio run -e esp32dev
```
