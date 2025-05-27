# Parkinsonian Tremor Detector

## Overview

Parkinson’s disease affects over 10 million people worldwide, with more than 70% experiencing resting tremors—characterized by involuntary shaking while the limb is at rest, typically at 3–6 Hz.

This project implements a wearable tremor detector using the STM32F429 Discovery Board and its built-in L3GD20 gyroscope. It performs real-time tremor detection and displays tremor presence and intensity on the onboard LCD.

## Objective

Design a standalone embedded system that:
- Captures angular velocity via the L3GD20 gyroscope
- Detects resting tremors (3–6 Hz) through frequency analysis
- Displays tremor presence and intensity using only onboard resources

## Hardware

- STM32F429 Discovery Board  
- USB power supply or portable USB power bank  

## Software

- [PlatformIO](https://platformio.org/) (VS Code extension)  
- STM32 HAL drivers (via PlatformIO)

## Features

- Real-time gyroscope sampling at 33.3 Hz  
- FFT-based detection of tremors in 3–6 Hz range  
- Live LCD visualization of tremor frequency and intensity  

## Detection Pipeline

1. **Acquisition**  
   - Samples X-axis angular velocity at 30 ms intervals (256 samples ≈ 7.68 s)

2. **Preprocessing**  
   - Converts raw sensor data into complex buffer for FFT

3. **FFT Analysis**  
   - Uses `arm_cfft_f32` (CMSIS-DSP) for frequency domain conversion  
   - Computes magnitude spectrum via `arm_cmplx_mag_f32`  
   - Identifies dominant frequency with `arm_max_f32`

4. **Classification**  
   - Smooths frequency with moving average  
   - Maps frequency to tremor intensity:
     - 3.0–4.0 Hz → LOW  
     - 4.0–5.0 Hz → MID  
     - 5.0–6.0 Hz → HIGH  

5. **Feedback**  
   - Displays frequency and intensity on LCD with color-coded indicators

## Constraints

- No external sensors or components allowed  
- Must use PlatformIO and STM32 HAL functions only

## Authors

Developed for the Real-Time Embedded Systems course (ECE-GY 6483)  
NYU Tandon School of Engineering, Spring 2024  
Kahlia Gronthos, Joshua Manogaran, Kyle Six

