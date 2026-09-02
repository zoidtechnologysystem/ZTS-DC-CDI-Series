# Vortex — High-Performance Digital CDI System

**Vortex** is a **Bare-Metal AVR C** firmware engineered for the **ATmega328P** microcontroller (16 MHz) to manage a Programmable Digital CDI (Capacitor Discharge Ignition) system. This architecture is entirely **hardware interrupt-driven** and **non-blocking**, ensuring microsecond-level ignition timing accuracy for high-speed engines operating up to 14,000 RPM.

This final optimized production version prioritizes aggressive memory efficiency, clean code practices, and stable Flash memory (`PROGMEM`) addressing to eliminate potential compiler pointer anomalies.

---

## 🚀 Key Features

- **Optimized Memory Addressing**: Implements explicit array index syntax (`[0]`) on Flash memory (`PROGMEM`) lookups, guaranteeing deterministic data mapping without the risk of corrupted data pointers.
- **Dual-Map Ignition Selector**: Features two independent 30-point lookup tables that can be toggled on-the-fly via a physical toggle switch (Pin D2):
  - **Pertalite Curve (Map 0)**: A conservative ignition timing profile designed for mid-octane fuel (Maximum 36° BTDC).
  - **Pertamax Curve (Map 1)**: An advanced mid-to-high RPM ignition timing profile optimized for high-octane fuel (Maximum 38° BTDC).
- **Dual-Mode Hard-Cut Limiter**: An aggressive engine speed limiter that completely kills the coil trigger pulse once the active threshold is breached (Pin D3):
  - **Low Limiter (9000 RPM)**: Ideal for a dedicated launch control function.
  - **High Limiter (12500 RPM)**: Unlocks full power capacity up to the safe mechanical limit.
- **Hardware-Level Potato Idle (< 1400 RPM)**: Generates a unique, erratic big-twin styling idle tone by sampling the pseudo-random lower bits of the main timer register (`TCNT1 & 0x07`) combined with an extreme ignition retard adjustment (~4° BTDC).

---

## 🛠️ Hardware Pinout Configuration

The firmware operates with a fixed core clock of **16 MHz** (`F_CPU = 16000000UL`) using the following peripheral pin assignments:

| MCU Peripheral | Physical Hardware Interface | ATmega328P Pin | Arduino Pin |
| :--- | :--- | :--- | :--- |
| **Input Capture (ICP1)** | Pulser / Pickup Coil Sensor (Configured for **Falling Edge**) | **Pin PB0** | Digital Pin 8 |
| **Trigger Output A** | CDI SCR Gate Trigger (Dual-Channel Parallel Configuration) | **Pin PD4** | Digital Pin 4 |
| **Trigger Output B** | CDI SCR Gate Trigger (Dual-Channel Parallel Configuration) | **Pin PD5** | Digital Pin 5 |
| **Map Selector Switch** | Fuel Profile Selector Switch (`GND` = Pertamax, `Open/HIGH` = Pertalite) | **Pin PD2** | Digital Pin 2 |
| **Limiter Selector Switch** | Limiter Mode Selector Switch (`GND` = 9000 RPM, `Open/HIGH` = 12500 RPM) | **Pin PD3** | Digital Pin 3 |

*System Timer Specifications: Timer 1 runs on a **64** Prescaler (`CS11` and `CS10`). This establishes a clock frequency of **250 kHz**, where exactly 1 tick equals **4 µs**.*

---

## 📊 Predefined Ignition Map Profile

Below is the structured predefined timing curve profile (`ignition_maps`) paired with its corresponding engine speed intervals (`rpm_axis`):

| Engine Speed (RPM) | Pertalite Curve (Map 0) | Pertamax Curve (Map 1) |
| :--- | :--- | :--- |
| **0 - 1200 RPM** | 15° BTDC | 15° BTDC |
| **1500 RPM** | 18° BTDC | 19° BTDC |
| **2000 - 3000 RPM** | 22° → 29° BTDC | 24° → 31° BTDC |
| **3500 - 4000 RPM** | 31° → 32° BTDC | 33° → 34° BTDC |
| **4500 - 7000 RPM** | 33° → 34° BTDC | 35° → 37° BTDC |
| **7500 - 8500 RPM** | 35° → 36° BTDC | 38° BTDC (Peak Performance) |
| **9000 - 11500 RPM** | 35° → 25° BTDC | 37° → 25° BTDC |
| **12000 - 14000 RPM** | 20° → 0° BTDC (Rev-Limiter Slope) | 20° → 0° BTDC (Rev-Limiter Slope) |

*Note: The hardware base physical pulser offset is calibrated at `ANCHOR_DEG_X10 = 150` (15.0° Before Top Dead Center / BTDC) relative to the sensor falling edge trigger point.*

---

## 💻 Interrupt & Control Flow Architecture

1. **`ISR(TIMER1_CAPT_vect)`**: Captures the absolute hardware timer snapshot on every sensor falling edge to compute the total elapsed `ticks_per_rev` without software processing delay.
2. **`calculate_advance`**: Executes a fast binary search routine (`find_map_segment`) and maps the target timing degree linearly to eliminate abrupt ignition angle jumps.
3. **`ISR(TIMER1_COMPA_vect)`**: Asserts the CDI trigger pins (`PD4` & `PD5`) the exact microsecond the calculated target ignition advance degree is reached.
4. **`ISR(TIMER1_COMPB_vect)`**: Clears the ignition trigger pins after a brief 25-tick cycle (~100 µs), forming a crisp square pulse to drive the hardware SCR gate safely before the capacitor recharge cycle begins.

---

## ⚙️ Compilation and Flashing

### Prerequisites
- Software Toolchain: `avr-gcc`, `avr-libc`, and GNU `make`.
- In-System Programmer: `avrdude` (supports USBasp, USBtinyISP, or ArduinoISP).

### Terminal Command Line Sequence

1. **Compile C Source Code**:
   ```bash
   avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o vortex.elf main.c
   ```
2. **Extract Intel HEX Binary**:
   ```bash
   avr-objcopy -O ihex -R .eeprom vortex.elf vortex.hex
   ```
3. **Flash Firmware to Target MCU (ATmega328P)**:
   ```bash
   avrdude -c usbasp -p m328p -U flash:w:vortex.hex:i
   ```

---

## ⚠️ Disclaimer
*This firmware manipulates critical ignition parameters of high-speed internal combustion engines in real-time. Unstable ignition map values or sustained usage of hard-cut limiters present severe risks of engine knocking, detonation, and catastrophic mechanical engine failure. Always validate your firmware behavior with an oscilloscope or engine simulator before deployment to a live high-voltage ignition coil.*

---
**ZOID TECHNOLOGY SYSTEM © 2022**
