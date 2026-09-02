# Neon XFire — Intelligent Digital CDI System

**Neon XFire** adalah firmware berbasis **AVR C (Bare-Metal)** yang dirancang untuk mengontrol sistem pengapian **Digital CDI (Capacitor Discharge Ignition)** secara real-time. Menggunakan mikrokontroler **ATmega328P** dengan clock 16 MHz, sistem ini mendeteksi kecepatan putaran mesin (RPM) secara presisi melalui fitur *Input Capture* dan menghitung derajat waktu pengapian secara dinamis menggunakan metode interpolasi linear berdasarkan tabel pemetaan (*ignition map*).

---

## 🚀 Fitur Utama

- **Real-Time RPM Tracking**: Menggunakan interupsi *Input Capture* Timer 1 untuk menghitung durasi per rotasi secara instan.
- **30-Point Ignition Mapping**: Memiliki tabel pemetaan kurva pengapian bawaan yang mencakup rentang dari 0 RPM hingga 14.000 RPM.
- **Linear Interpolation Algorithm**: Menghitung sudut derajat pengapian (*advance angle*) secara halus di antara titik pemetaan standar.
- **Dual Mode Operation**:
  - **Mode Starting/Low RPM (< 1500 RPM)**: Pengapian langsung ditembakkan secara instan begitu sinyal pulsa masuk demi kemudahan menyalakan mesin.
  - **Mode Dinamis (≥ 1500 RPM)**: Waktu pengapian diatur secara mikro detasemen berdasarkan kalkulasi posisi derajat jangkar (*Anchor/Pick-up*).
- **Jitter Protection**: Deteksi waktu aman menggunakan perbandingan register `TCNT1` untuk mencegah keterlambatan pembakaran yang tidak diinginkan.

---

## 🛠️ Detail Konfigurasi Perangkat Keras

Sistem berjalan pada frekuensi internal **16 MHz** (`F_CPU = 16000000UL`) dengan konfigurasi register periferal sebagai berikut:

| Komponen / Register | Deskripsi | Pin MCU (ATmega328P) |
| :--- | :--- | :--- |
| **Input Pulsa (Pulser/Pick-up)** | Input capture sensor magnet (Falling-edge, Noise Canceler aktif) | **Pin PB0 / ICP1** (Digital Pin 8) |
| **Output CDI Trigger** | Sinyal pulsa ganda untuk memicu SCR CDI | **Pin PD4 & PD5** (Digital Pin 4 & 5) |
| **Pin Indikator / Aux** | Konfigurasi internal input dengan pull-up | **Pin PD6** (Digital Pin 6) |
| **Timer 1 Prescaler** | Diatur ke pembagi **64** (`CS11` & `CS10`) | Frekuensi Timer: **250 kHz** (1 tick = 4 µs) |

---

## 📊 Kurva Pengapian (Ignition Map)

Tabel berikut menunjukkan pemetaan sudut derajat pengapian bawaan (`map0`) berdasarkan putaran mesin (`rpm_axis`) yang disimpan di dalam memori `PROGMEM`:

| RPM | Advance Angle (Derajat) | | RPM | Advance Angle (Derajat) |
|---|---|---|---|---|
| **0 - 1200** | 15° | | **8000 - 8500** | 36° |
| **1500** | 18° | | **9000** | 35° |
| **2000** | 22° | | **9500** | 34° |
| **2500** | 26° | | **10000** | 32° |
| **3000** | 29° | | **10500** | 30° |
| **3500** | 31° | | **11000** | 28° |
| **4000** | 32° | | **11500** | 25° |
| **4500 - 6500** | 33° - 34° | | **12000** | 20° |
| **7000** | 34° | | **12500** | 15° |
| **7500** | 35° | | **13000 - 14000** | 10° → 0° (Limiter) |

*Catatan: Posisi tonjolan pulser fisik diatur pada nilai jangkar baku `ANCHOR_DEG_X10 = 150` (15.0 derajat sebelum TMA / BTDC).*

---

## 💻 Struktur Logika Firmware

1. **`ISR(TIMER1_CAPT_vect)`**: Menangkap nilai timer setiap kali pulser mendeteksi sinyal jatuh (*falling edge*) untuk menghitung nilai `ticks_per_rev`.
2. **`calculate_advance()`**: Melakukan pencarian segmen biner (`find_map_segment`) lalu menginterpolasi nilai derajat pengapian secara linier agar transisi kurva halus.
3. **`ISR(TIMER1_COMPA_vect)`**: Mengaktifkan pin pemicu (`PD4` & `PD5`) secara akurat saat target sudut pengapian tercapai.
4. **`ISR(TIMER1_COMPB_vect)`**: Mematikan pin pemicu setelah jeda waktu tertentu (25 ticks atau ~100 µs) untuk menghasilkan pulsa *trigger* berbentuk persegi.

---

## ⚙️ Kompilasi dan Flashing

### Persyaratan
- **Toolchain**: `avr-gcc`, `avr-libc`, dan `make`.
- **Uploader**: `avrdude` dengan programmer pilihan (seperti USBasp atau Arduino as ISP).

### Langkah-langkah

1. **Kompilasi Kode C**:
   ```bash
   avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o neon_xfire.elf main.c
   ```
2. **Konversi ke Format Intel HEX**:
   ```bash
   avr-objcopy -O ihex -R .eeprom neon_xfire.elf neon_xfire.hex
   ```
3. **Flash ke Mikrokontroler**:
   ```bash
   avrdude -c usbasp -p m328p -U flash:w:neon_xfire.hex:i
   ```

---

## ⚠️ Disklaimer
*Kode ini dirancang untuk keperluan edukasi dan riset sistem otomotif. Kesalahan penyusunan peta kurva pengapian (terlahu maju/advance) pada mesin nyata berisiko menyebabkan knocking tinggi dan kerusakan fatal pada komponen internal mesin (piston/stang seher).*

---
**ZOID TECHNOLOGY SYSTEM © 2026**
