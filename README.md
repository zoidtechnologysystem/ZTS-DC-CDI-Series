# Zoid Technology System: Advanced Embedded Ignition System (ECU/CDI)

A high-performance, bare-metal embedded firmware ecosystem custom-built for motorcycle racing (Grasstrack/Dirt Bike). Developed from 2021 to 2026, this project showcases the evolution from linear-polling architecture to advanced asynchronous, interrupt-driven multi-mapping execution on 8-bit AVR microcontrollers (ATmega328P).

<img width="720" height="540" alt="IMG_20250301_165703" src="https://github.com/user-attachments/assets/5e9f07eb-efbd-4f53-92d2-54ef053ec175" />

## 🏆 Track Proven Resilience
The **Shadow Master** firmware architecture within this ecosystem has been rigorously tested in competitive regional racing, securing multiple podium finishes (1st, 2nd, and 3rd places) due to its high deterministic loop rate and noise-immune state polling architecture.

## 🛠️ Key Technical Features
*   **Asynchronous Interrupt-Driven Timing:** Replaced CPU-blocking loop routines with high-precision `TIMER1_CAPT` input capture for RPM telemetry and `TIMER1_COMPA/B` for hardware-timed spark ignition.
*   **High-Speed Binary Search Interpolation:** Implemented 1D/2D ignition map lookup using binary search variants to execute 30-point matrix interpolations within sub-millisecond loops without Floating-Point Units (FPU).
*   **Hierarchical Launch Control & Multi-Limiters:** Multi-stage safety and acceleration strategies (up to 6 configurable dynamic limits) handled strictly via non-blocking hardware compares.
*   **Flash Memory (PROGMEM) Optimization:** Direct pointer-offset manual addressing on 2D arrays (`ignition_maps`) to drastically reduce static RAM footprints to less than 15% utilization.
*   **Atomic Data Protection:** Implemented strict block isolation during critical 16-bit register copies (`ticks_per_rev` and `capture_snap`) to completely prevent race conditions at high operational speeds (14,000+ RPM).

## 🚀 Firmware Evolution Roadmap
1.  **V1 Base (2021):** Proof of concept using raw register configurations and blocking linear polling loops.
2.  **Neon Xfire (2022):** Migration to hardware-timed interrupt capture routines. Fixed-point math introduction.
3.  **Vortex (2022):** Implementation of dual-fuel maps (Pertalite/Pertamax) and binary search lookups.
4.  **Shadow Master (2023):** Champion-grade execution. Implemented robust hardware switch toggles for glitch-free track runtime.
5.  **Andromeda (2025 - Flagship):** Expanded to 6 distinct fuel-curve matrices (up to Nitro Race grade) and digital multi-limiter sequences using time-decoupled button state machines.

---
*Developed by ZOID TECHNOLOGY SYSTEM*
