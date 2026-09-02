# Neon XFire — Intelligent Digital CDI System

**Neon XFire** is an **AVR C (Bare-Metal)** firmware designed to control a **Digital CDI (Capacitor Discharge Ignition)** system in real-time. Operating on an **ATmega328P** microcontroller clocked at 16 MHz, this system accurately tracks engine speed (RPM) using the hardware Input Capture feature and dynamically computes optimal ignition timing via linear interpolation from a predefined ignition map.

---

## 🚀 Key Features

- **Real-Time RPM Tracking**: Utilizes Timer 1 Input Capture interrupts to measure the exact duration per engine revolution instantaneously.
- **30-Point Ignition Mapping**: Features a built-in look-up table (`map0`) spanning from 0 RPM up to 14,000 RPM.
- **Linear Interpolation Algorithm**: Smoothly calculates the intermediate ignition advance angles between fixed map intervals.
- **Dual-Mode Operation**:
  - **Starting / Low RPM Mode (< 1500 RPM)**: Instantly fires the ignition pulse upon signal capture to ensure quick and reliable engine starts.
  - **Dynamic Mode (≥ 1500 RPM)**: Schedules precise microsecond-delayed micro-ignition timings relative to the physical hardware anchor (Pick-up/Pulser).
- **Jitter & Late-Trigger Protection**: Employs real-time `TCNT1` register delta checks to instantly fire the ignition if scheduling latency is detected.

---

## 🛠️ Hardware Configuration & Pinout

The firmware runs at a core frequency of **16 MHz** (`F_CPU = 16000000UL`) with the following peripheral register mapping:

| Peripheral / Register | Description | ATmega328P Pin | Arduino Pin Equivalent |
| :--- | :--- | :--- | :--- |
| **Pulse Input (Pulser)** | Magneto sensor pulse trigger (Falling-edge, Input Capture Noise Canceler active) | **PB0 / ICP1** | Digital Pin 8 |
| **CDI Trigger Output** | Dual-channel synchronized pulse to trigger the CDI SCR | **PD4 & PD5** | Digital Pin 4 & 5 |
| **Auxiliary / Indicator** | Internal input configuration with pull-up enabled | **PD6** | Digital Pin 6 |
| **Timer 1 Prescaler** | Set to **64** (`CS11` & `CS10`) | Frequency: **250 kHz** | 1 Tick = 4 µs |

---

## 📊 Ignition Map Profile

Below is the structured predefined timing curve profile (`map0`) paired with its corresponding engine speed intervals (`rpm_axis`) stored inside the flash memory via `PROGMEM`:

| Engine Speed (RPM) | Advance Angle (Degrees) | | Engine Speed (RPM) | Advance Angle (Degrees) |
|---|---|---|---|---|
| **0 - 1200** | 15° | | **8000 - 8500** | 36° |
| **1500** | 18° | | **9000** | 35° |
| **2000** | 22° | | **9500** | 34° |
| **2500** | 26° | | **10000** | 32° |
| **10500** | 30° |
| **3500** | 31° | | **11000** | 28° |
| **4000** | 32° | | **11500** | 25° |
| **4500 - 6500** | 33° - 34° | | **12000** | 20° |
| **7000** | 34° | | **12500** | 15° |
| **7500** | 35° | | **13000 - 14000** | 10° → 0° (Rev-Limiter) |

*Note: The hardware base physical pulser offset is calibrated at `ANCHOR_DEG_X10 = 150` (15.0° Before Top Dead Center / BTDC).*

---

## 💻 Firmware Logic & Interrupt Architecture

1. **`ISR(TIMER1_CAPT_vect)`**: Captures the absolute hardware timer snapshot on every sensor falling edge to compute the total elapsed `ticks_per_rev`.
2. **`calculate_advance()`**: Executes a binary search routine (`find_map_segment`) and maps the target timing degree linearly to eliminate sudden steps.
3. **`ISR(TIMER1_COMPA_vect)`**: Asserts the ignition output pins (`PD4` & `PD5`) the exact moment the calculated target degree timing is reached.
4. **`ISR(TIMER1_COMPB_vect)`**: Clears the ignition trigger pins after a brief 25-tick cycle (~100 µs), forming a crisp square pulse to drive the hardware SCR gate.

---

## ⚙️ Compilation and Flashing

### Prerequisites
- **Toolchain**: `avr-gcc`, `avr-libc`, and GNU `make`.
- **In-System Programmer**: `avrdude` alongside a hardware flasher (e.g., USBasp, USBtinyISP, or ArduinoISP).

### Compilation Pipeline

1. **Compile C Source Code**:
   ```bash
   avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o neon_xfire.elf main.c
   ```
2. **Extract Intel HEX Binary**:
   ```bash
   avr-objcopy -O ihex -R .eeprom neon_xfire.elf neon_xfire.hex
   ```
3. **Flash Firmware to Microcontroller**:
   ```bash
   avrdude -c usbasp -p m328p -U flash:w:neon_xfire.hex:i
   ```

---

## ⚠️ Disclaimer
*This firmware is created strictly for educational, research, and development purposes in automotive electronics control units. Writing an unstable or overly advanced ignition map poses severe risks of engine knocking, detonation, and catastrophic mechanical engine failure.*

---
**ZOID TECHNOLOGY SYSTEM © 2021**
