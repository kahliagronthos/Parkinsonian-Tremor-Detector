# Parkinsonian Tremor Detector

## Overview

Parkinson's disease affects over 10 million people globally, with more than 70% experiencing a symptom known as **resting tremor**—involuntary rhythmic shaking that typically occurs when muscles are relaxed. Detecting this tremor in a clinically accurate and non-invasive manner is essential for effective treatment and therapy optimization.

This project implements a **wearable Parkinsonian Tremor Detector** using the **STM32F429 Discovery Board** and its built-in **L3GD20 gyroscope**. The system performs real-time tremor analysis and provides visual feedback on the **presence and intensity** of resting tremors through on-board visual indicators (e.g., LEDs or LCD screen).

---

## Objective

Build a standalone embedded system that:
- Captures real-time angular velocity data using the L3GD20 gyroscope.
- Analyzes data segments to detect **resting tremors** (3–6 Hz frequency range).
- Provides **visual indication** of detected tremors and their intensity using only on-board hardware (no external sensors or modules).

---

## Hardware Requirements

- STM32F429 Discovery Board  
- USB power supply or portable USB power bank  

---

## Software Requirements

- [PlatformIO](https://platformio.org/) IDE (VS Code extension)
- STM32 HAL drivers (via PlatformIO)

---

## Features

- **Real-time gyroscope data capture** via L3GD20
- **Frequency analysis** to detect 3–6 Hz tremor patterns
- **Onboard visualization** of tremor presence/intensity using:
  - LEDs
  - LCD display (optional, if used for numeric/graphical feedback)

---

## Detection Methodology

1. **Sampling**: Continuously sample angular velocity data along relevant axes.
2. **Segmentation**: Divide signal into short time windows (e.g., 1–2 seconds).
3. **Frequency Analysis**: Perform FFT or zero-crossing analysis to extract frequency content.
4. **Thresholding**: Classify signal as a resting tremor if energy is concentrated in the 3–6 Hz range.
5. **Visualization**: Update onboard indicators (LEDs/LCD) based on detection result and tremor intensity.

---

## Project Restrictions

- No additional hardware or sensors allowed.
- Must be implemented as either:
  - An **individual project**, or  
  - A **2- to 3-student team** project.
- Must be developed **entirely using PlatformIO**.
- Only STM32-provided drivers/HAL functions may be used.

---

## Repository Structure
Parkinsonian-Tremor-Detector/
├── src/ # Main application code
│ ├── main.cpp # Initialization, data acquisition, and tremor detection
│ └── gyro_utils.cpp # Gyroscope read and processing helpers
├── include/ # Header files
├── lib/ # Optional user libraries
├── platformio.ini # PlatformIO project configuration
└── README.md # Project overview

--- 

## Authors

This project was completed as part of the **Real-Time Embedded Systems course project**  
Spring 2024 by Kahlia Gronthos, Joshua Manogaran and Kyle Six
