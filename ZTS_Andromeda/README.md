# Andromeda — Advanced Multi-Map CDI with Non-Blocking State Machine

**Andromeda (Non-Blocking Revision)** is a high-end, competition-grade **Bare-Metal AVR C** firmware engineered for the **ATmega328P** microcontroller (16 MHz). Operating as a fully **hardware interrupt-driven** and **non-blocking** architecture, it provides microsecond-level ignition timing accuracy for professional racing platforms operating up to 14,000 RPM.

This edition features a **Sequential Scrolling State Machine** that handles 6 distinctive fuel ignition curves and 6 programmable rev-limiter stages via a single-button cyclic interface, paired with an ultra-stable **Non-Blocking Crank-Synchronized Debounce Algorithm**.

---

## 🚀 Key Features

- **Sequential Map Scrolling (6-Map Matrix)**: Supports 6 independent 30-point lookup tables stored in Flash memory (`PROGMEM`) toggled dynamically on-the-fly using a single push-button cycle (Pin D2).
- **Multi-Stage Rev Limiter (6-Limiter Array)**: Cycles through 6 different rev-limiter thresholds sequentially using a physical button interface (Pin D3), mapped from a dedicated `limiter_sequence` array.
- **Crank-Synchronized Non-Blocking Debounce**: Replaces primitive software loop delays with an elegant hardware-event flag lock (`btn_ready_to_read = 0`). Button sampling is gated by the magneto pulser's hardware rotative cycle (`cdi_data.new_pulse`), ensuring **zero misfires or timing drops at high RPM**.
- **Integrated Launch Control (2-Step Limiter)**: Ties the system down to a precise staging limit of **4,500 RPM** upon holding the physical grounding switch (Pin D7). This mechanism overrides any active sequential limiter stage to deliver optimal hole-shot traction.
- **Zero Pointer Ambiguity**: Enforces explicit array dimension indices (`[0]` and `[MAP_POINTS - 1U]`) across all `PROGMEM` data retrievals, protecting the core mapping engine against compiler memory offset distortions.
- **Hardware-Level Potato Idle (< 1400 RPM)**: Emulates a rhythmic, low-frequency V-Twin style stasioner rumble by evaluating the lower register bits of the hardware timer counter (`TCNT1 & 0x07`) alongside an extreme timing retard adjustment (~4° BTDC).

---

## 🛠️ Hardware Pinout Configuration

The system runs on an external **16 MHz** clock crystal (`F_CPU = 16000000UL`) with the following peripheral register layout:

| MCU Peripheral Interface | Hardware Component / Connection | ATmega328P Pin | Arduino Pin |
| :--- | :--- | :--- | :--- |
| **Input Capture 1 (ICP1)** | Pulser / Pickup Coil Signal Input (**Falling Edge Locked**) | **Pin PB0** | Digital Pin 8 |
| **Trigger Output A** | Synchronized Dual CDI SCR Gate Drive Outputs | **Pin PD4** | Digital Pin 4 |
| **Trigger Output B** | Synchronized Dual CDI SCR Gate Drive Outputs | **Pin PD5** | Digital Pin 5 |
| **Map Selection Input** | Cyclic Map Change Push-Button (Internal Pull-Up) | **Pin PD2** | Digital Pin 2 |
| **Limiter Selection Input** | Cyclic Limiter Stage Push-Button (Internal Pull-Up) | **Pin PD3** | Digital Pin 3 |
| **Launch Control Input** | Push-Button Launch / Anti-Lag Switch (Internal Pull-Up) | **Pin PD7** | Digital Pin 7 |

*Timer Architecture Note: Timer 1 is bound to a hardware Prescaler of **64** (`CS11` and `CS10` set). The resulting base frequency is **250 kHz**, yielding an exact tracking scale where 1 internal timer tick equals **4 µs**.*

---

## 📊 Predefined Timing Curve Profiles

The matrix below maps the 6 multi-fuel ignition curves (`ignition_maps`) relative to the shared engine speed axis (`rpm_axis`):

| RPM Range | Map 0: Pertalite | Map 1: Turbo | Map 2: Bensol | Map 3: VP 110 | Map 4: VP Q16 | Map 5: VP M5 / Nitro |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **0 - 1200** | 15° BTDC | 15° BTDC | 15° BTDC | 15° BTDC | 15° BTDC | 15° BTDC |
| **1500** | 18° BTDC | 19° BTDC | 20° BTDC | 21° BTDC | 22° BTDC | 23° BTDC |
| **2000 - 3000** | 22° → 29° | 24° → 31° | 25° → 32° | 26° → 33° | 27° → 34° | 28° → 35° |
| **3500 - 4000** | 31° → 32° | 33° → 34° | 34° → 35° | 35° → 37° | 36° → 38° | 37° → 39° |
| **4500 - 7000** | 33° → 34° | 35° → 37° | 36° → 40° | 38° → 42° | 39° → 44° | 40° → 45° |
| **7500 - 8500** | 35° → 36° | 38° → 39° | 41° → 40° | 43° → 41° | 44° → 41° | 45° → 42° |
| **9000 - 11500** | 35° → 25° | 37° → 25° | 38° → 25° | 39° → 25° | 39° → 25° | 40° → 25° |
| **12000 - 14000**| 20° → 0° | 20° → 0° | 20° → 0° | 20° → 0° | 20° → 0° | 20° → 0° |

### 🛑 Cyclic Rev-Limiter Stages Array
The `limiter_sequence` cycles through the following values on each push-button trigger:
1. **Stage 0**: 12,500 RPM (`LIMIT_RPM_HIGH`)
2. **Stage 1**: 9,000 RPM (`LIMIT_RPM_LOW`)
3. **Stage 2**: 10,000 RPM
4. **Stage 3**: 10,500 RPM
5. **Stage 4**: 11,000 RPM
6. **Stage 5**: 11,800 RPM

---

## 💻 Control Logic & Interrupt Flow Hierarchy

1. **`ISR(TIMER1_CAPT_vect)`**: Triggers instantly on the pickup sensor's falling edge to latch the raw timer snapshot into atomic duration registers (`ticks_per_rev`).
2. **`State Debounce Engine`**: The program samples the cyclic push-buttons once per revolution. Upon detecting a falling transition, the state (`map_index` or `limiter_index`) increments, and `btn_ready_to_read` locks down to `0`. It remains locked until the next `new_pulse` refreshes it, bypassing the need for a delay.
3. **`calculate_advance`**: Executes an optimized binary search routine (`find_map_segment`) and solves intermediate advance angles via integer linear interpolation.
4. **`ISR(TIMER1_COMPA_vect)`**: Drives the CDI gate output network high (`PD4` & `PD5`) the exact microsecond the pre-calculated target timing register matches.
5. **`ISR(TIMER1_COMPB_vect)`**: Clears the output gate pins exactly 25 ticks (~100 µs) later, forming a consistent square firing pulse before the high-voltage capacitor recharge sequence begins.

---

## ⚙️ Compilation and Deployment

### Development Environment
- Compiler Toolchain: `avr-gcc`, `avr-libc`, GNU `make`.
- Programming Interface: `avrdude` paired with a hardware flash interface (e.g., USBasp, USBtinyISP).

### Compilation Flow Sequence

1. **Compile C Source Firmware**:
   ```bash
   avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o andromeda.elf main.c
   ```
2. **Extract Intel Production HEX File**:
   ```bash
   avr-objcopy -O ihex -R .eeprom andromeda.elf andromeda.hex
   ```
3. **Flash Image to Target Microcontroller**:
   ```bash
   avrdude -c usbasp -p m328p -U flash:w:andromeda.hex:i
   ```

---

## ⚠️ Disclaimer
*This firmware directly controls high-performance internal combustion engine parameters. Sustained deployment of aggressive ignition advances (e.g., Maps 4 & 5) on low-octane fuels, or prolonged activation of hard-cut rev limiters, will cause extreme engine knocking, thermal degradation, physical valve-to-piston collisions, and total engine block failure. Always analyze CDI behavioral outputs using an oscilloscope before live testing.*

---
**ZOID TECHNOLOGY SYSTEM © 2026**
