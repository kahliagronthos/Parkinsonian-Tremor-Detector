# Parkinsonian Tremor Detector

## Overview

Over 10 million people worldwide live with Parkinson’s disease, a neurodegenerative disorder that severely impacts motor control. A common symptom in over 70% of patients is a resting tremor, defined as involuntary shaking that occurs when muscles are relaxed and the limb is supported. These tremors typically occur at a frequency of 3–6 Hz.

This project implements a wearable Parkinsonian Tremor Detector using the STM32F429 Discovery Board and its built-in L3GD20 gyroscope. The system performs real-time tremor analysis and provides visual feedback on the presence and intensity of resting tremors through the on-board LCD screen.


## Objective

Build a standalone embedded system that:
- Captures real-time angular velocity data using the L3GD20 gyroscope.
- Analyzes data segments to detect resting tremors (3–6 Hz frequency range).
- Provides visual indication of detected tremors and their intensity using only on-board hardware (no external sensors or modules).


## Hardware Requirements

- STM32F429 Discovery Board  
- USB power supply or portable USB power bank  


## Software Requirements

- [PlatformIO](https://platformio.org/) IDE (VS Code extension)
- STM32 HAL drivers (via PlatformIO)


## Features

- Real-time gyroscope data capture via L3GD20
- Frequency analysis to detect 3–6 Hz tremor patterns
- Onboard visualization of tremor presence/intensity using LCD display


## Detection Methodology

Our system identifies Parkinsonian resting tremors through gyroscope-based spectral analysis:

1. **Signal Acquisition**  
   - Samples X-axis angular velocity using the onboard L3GD20 gyroscope at 33.3 Hz (every 30 ms).
   - Collects 256 samples, representing ~7.68 seconds of wrist movement.

2. **Preprocessing**  
   - Stores raw X-axis values in a complex buffer for FFT input.
   - Real parts hold sensor data; imaginary parts are initialized to zero.

3. **FFT Analysis**  
   - Uses CMSIS-DSP function `arm_cfft_f32()` to compute the frequency spectrum.
   - Converts complex FFT output to magnitude spectrum with `arm_cmplx_mag_f32()`.
   - Identifies dominant frequency using `arm_max_f32()`.

4. **Classification**  
   - Applies a moving average to smooth out frequency readings.
   - Maps frequency to tremor intensity:
     - **3.0–4.0 Hz** → `LOW`
     - **4.0–5.0 Hz** → `MID`
     - **5.0–6.0 Hz** → `HIGH`

5. **User Feedback**  
   - Displays live frequency readouts and intensity classification on the LCD screen.
   - Visual feedback includes colored boxes indicating tremor severity.


## Project Restrictions

- No additional hardware or sensors allowed.
- Must be entirely developed using PlatformIO.
- Only STM32-provided drivers/HAL functions may be used.


## Authors

This project was completed as part of NYU Tandon School of Engineering's Real-Time Embedded Systems course (ECE-GY 6483) for
Spring 2024 by Kahlia Gronthos, Joshua Manogaran and Kyle Six
