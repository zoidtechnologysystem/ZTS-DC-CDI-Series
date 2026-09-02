# Shadow Master — Advanced Digital CDI with Integrated Launch Control

**Shadow Master** is a production-grade, production-ready **Bare-Metal AVR C** firmware designed for the **ATmega328P** microcontroller (16 MHz). Operating as a fully **hardware interrupt-driven** and **non-blocking** architecture, it provides microsecond-level ignition timing accuracy for high-performance racing platforms operating up to 14,000 RPM.

The **SHADOW MASTER** edition introduces a hardware-level **Launch Control (2-Step Limiter)** and achieves 100% deterministic memory addressing across all flash memory (`PROGMEM`) lookups.

---

## 🚀 Key Features

- **Integrated Launch Control (2-Step Limiter)**: Asserts a dedicated staging limiter profile at **4,500 RPM** via a physical staging button (Pin D7). It holds the engine at the perfect torque band for hole-shots while eliminating wheel-spin, then instantly restores full RPM capability upon release.
- **Dual-Mode Hard-Cut Limiter**: Toggles between a secondary staging limit and raw maximum performance thresholds using a hardware toggle switch (Pin D3):
  - **Low Limiter (9000 RPM)**: Valet / Pit-lane / Break-in safety limit.
  - **High Limiter (12500 RPM)**: Full mechanical performance ceiling.
- **Deterministic Memory Addressing**: Completely resolves all residual compiler pointer ambiguities by enforcing explicit array index lookups (`` and `[MAP_POINTS - 1U]`) on all `PROGMEM` data retrievals.
- **Dual-Map Ignition Profile Selection**: Dynamically switches ignition advance curves on-the-fly via a fuel selector interface (Pin D2):
  - **Pertalite Curve (Map 0)**: Conservative mapping optimized for mid-octane fuels (Max 36° BTDC).
  - **Pertamax Curve (Map 1)**: Aggressive advance profile optimized for high-octane fuels (Max 38° BTDC).
- **Hardware-Level Potato Idle (< 1400 RPM)**: Emulates a rhythmic, low-frequency big-twin styling idle tone by sampling the raw noise from the low-order register bits (`TCNT1 & 0x07`) combined with an extreme ignition timing retard (~4° BTDC).

---

## 🛠️ Hardware Pinout & Architecture Configuration

The system runs on an external **16 MHz** oscillator source (`F_CPU = 16000000UL`) utilizing the following peripheral register mapping:

| MCU Register / Interface | Component / Wire Interface | ATmega328P Pin | Arduino Pin |
| :--- | :--- | :--- | :--- |
| **Input Capture 1 (ICP1)** | Pulser / Pickup Coil Signal Input (**Falling Edge Locked**) | **Pin PB0** | Digital Pin 8 |
| **Trigger Output A** | Synchronized Dual CDI SCR Gate Drive Outputs | **Pin PD4** | Digital Pin 4 |
| **Trigger Output B** | Synchronized Dual CDI SCR Gate Drive Outputs | **Pin PD5** | Digital Pin 5 |
| **Map Selector Switch** | Fuel Octane Curve Switch (`GND` = Pertamax, `High` = Pertalite) | **Pin PD2** | Digital Pin 2 |
| **Limiter Selector Switch** | Secondary Rev Limiter Switch (`GND` = 9000 RPM, `High` = 12500 RPM) | **Pin PD3** | Digital Pin 3 |
| **Launch Control Switch** | Push-Button Launch Switch (`GND` = Active 4500 RPM Limiter) | **Pin PD7** | Digital Pin 7 |

*Timer Configuration Note: Timer 1 is set to run with a hardware Prescaler of **64** (`CS11` and `CS10` enabled). The resulting timer frequency is **250 kHz**, meaning exactly 1 internal timer tick equals **4 µs**.*

---

## 📊 Predefined Timing Curve Profiles

The matrix below illustrates the mapped engine timing degrees (`ignition_maps`) synchronized across the engine acceleration index (`rpm_axis`):

| Engine Speed (RPM) | Pertalite Profile (Map 0) | Pertamax Profile (Map 1) |
| :--- | :--- | :--- |
| **0 - 1200 RPM** | 15° BTDC | 15° BTDC |
| **1500 RPM** | 18° BTDC | 19° BTDC |
| **2000 - 3000 RPM** | 22° → 29° BTDC | 24° → 31° BTDC |
| **3500 - 4000 RPM** | 31° → 32° BTDC | 33° → 34° BTDC |
| **4500 - 7000 RPM** | 33° → 34° BTDC | 35° → 37° BTDC |
| **7500 - 8500 RPM** | 35° → 36° BTDC | 38° BTDC (Peak Advanced Torque) |
| **9000 - 11500 RPM** | 35° → 25° BTDC | 37° → 25° BTDC |
| **12000 - 14000 RPM** | 20° → 0° BTDC (Rev-Limiter Slope) | 20° → 0° BTDC (Rev-Limiter Slope) |

*The physical stator pickup base mechanical alignment offset (`ANCHOR_DEG_X10`) is calibrated precisely at **150** (15.0° Before Top Dead Center / BTDC).*

---

## 💻 Control Logic & Interrupt Flow Hierarchy

1. **`ISR(TIMER1_CAPT_vect)`**: Executes on the hardware signal falling edge to sample rotation cycles instantly into atomic duration variables (`ticks_per_rev`).
2. **`calculate_advance`**: Utilizes an accelerated binary lookup search algorithm (`find_map_segment`) and solves real-time timing angles via floating-point equivalent integer linear interpolation.
3. **`Limiter Execution Logic`**: Evaluates active safety limiters sequentially in the root loops. **Launch Control (Pin D7) takes ultimate hardware priority**, overriding standard high/low limiters when tied low.
4. **`ISR(TIMER1_COMPA_vect)`**: Fires the CDI gate drive pins (`PD4` & `PD5`) immediately upon reaching the pre-scheduled ignition timing register profile.
5. **`ISR(TIMER1_COMPB_vect)`**: Disasserts the trigger network precisely after a brief 25-tick delay (~100 µs), providing a clean capacitive discharge cycle window before the high-voltage charging loop restarts.

---

## ⚙️ Compilation and Deployment

### Toolchain Requirements
- Development Packages: `avr-gcc`, `avr-libc`, GNU `make`.
- Programming Interface: `avrdude` combined with a hardware programmer (e.g., USBasp, USBtinyISP, or an Arduino configured as an ISP).

### Deployment Automation Flow

1. **Compile C Source Firmware**:
   ```bash
   avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o shadow_master.elf main.c
   ```
2. **Extract Intel Production HEX File**:
   ```bash
   avr-objcopy -O ihex -R .eeprom shadow_master.elf shadow_master.hex
   ```
3. **Write Image to Target Microcontroller**:
   ```bash
   avrdude -c usbasp -p m328p -U flash:w:shadow_master.hex:i
   ```

---

## ⚠️ Disclaimer
*This firmware directly alters internal combustion behavior. Sustained usage of aggressive hard-cut rev limiters, extreme anti-lag adjustments, or poorly tuned timing advance tables can cause intense engine knocking, thermal degradation, physical valve train collisions, and catastrophic block failures. Always analyze CDI behavioral outputs using an oscilloscope or test bench simulation before driving physical ignition components.*

---
**ZOID TECHNOLOGY SYSTEM © 2022**
