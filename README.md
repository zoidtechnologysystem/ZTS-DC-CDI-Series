# ZTS DC CDI Series

Custom programmable CDI system developed by ZOID TECHNOLOGY SYSTEM for motorcycle applications.

## Overview

The ZTS DC CDI Series is a programmable ignition-control system developed through hands-on experimentation, firmware development, testing, and real-world motorcycle applications.

The system supports both **2-stroke and 4-stroke motorcycles**.

This repository documents the development and engineering work behind the system.

## Features

- 6 selectable ignition maps
- 4 selectable RPM limiter settings
- 60-point timing map per ignition map
- Map selector input
- Limiter selector input
- RPM / pulse-period sensing
- Programmable ignition timing
- Microcontroller-based ignition control
- Designed for 2-stroke and 4-stroke applications

## ZTS Andromeda

One of the documented versions in this series is:

**ZTS ANDROMEDA — 5 UNIT LIMITED EDITION**

The Andromeda firmware contains:

- 6 ignition maps
- 4 limiter configurations
- 60 timing positions per map
- Map and limiter selection through digital inputs
- Timer-based pulse measurement
- Programmable ignition delay calculation
- Controlled ignition output
- Pulse-width control

## Firmware Architecture

The firmware uses a table-based ignition map stored in program memory.

Each ignition map contains **60 timing values**.

The active map is selected using a digital input, while a second digital input selects the active limiter configuration.

The ignition event is calculated from the measured pulse period and the selected timing-map value.

## Hardware Interface

| Function | Pin |
|---|---|
| Pulse input | D2 |
| Ignition output | D3 |
| Secondary output | D4 |
| Map selector | D5 |
| Limiter selector | D6 |

The firmware configures Timer1 with a **64x prescaler** for pulse-period measurement.

## Development Philosophy

This project was developed through practical experimentation rather than a purely theoretical design process.

The development cycle follows:

**Build → Test → Observe → Debug → Modify → Test Again**

## Status

**Active product / ongoing documentation**

The CDI has been produced and sold in small quantities.

Additional hardware documentation, firmware versions, testing data, wiring information, and project photographs will be added progressively.

## Repository Structure

```text
ZTS-DC-CDI-Series/
│
├── firmware/
│   └── .gitkeep
│
└── README.md

Disclaimer

This repository is provided primarily as engineering documentation.

Ignition-system modification involves high-voltage electrical systems and engine operation. Hardware implementation and testing should be performed with appropriate technical knowledge and safety precautions.

ZTS — Build Not Bought.
