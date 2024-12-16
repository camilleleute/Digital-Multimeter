## P2 - Digital Multimeter

## Reference Materials
- STM32L4xxxx Reference Manual (RM) – General Purpose Timers, ADC, USART
- Technical Note - VT100 Terminal

## Introduction
- The purpose of this project is to design and implement a digital multimeter (DMM) that can measure voltage and frequency to meet the specified requirements below. This project is a more complicated coding challenge than the previous projects and requires a strategy to be formulated before starting the coding. Failure to properly plan out this project ahead of time and implement a solution in smaller testable stages will result in wasting a significant amount of time.

## Objectives
- Understand the challenges of measuring unknown signals
- Gain experience building systems that process data continuously in real time
- Gain experience making design decisions with hardware vs software tradeoffs

## System Requirements
- The DMM shall measure voltage.
  - Voltage measurements shall be limited to 0 to 3.3 volts.
  - Voltage measurements shall be accurate to +/- 25 mv
  - Voltage measurements shall be updated at least every 3.5 seconds.
  - The DMM shall have DC and AC measurements.
    - DC and AC measurements can be displayed in separate modes or simultaneously
    - If measurements are displayed in separate modes, the mode selection can be selected manually by the user.
- The DMM DC measurements
  - DC measurement shall be an average of multiple measurements over a constant time period greater than 1 ms and less than 100 ms.
- The DMM AC measurements
  - AC measurements shall display RMS values
    - The RMS measurement shall be true-RMS (include the DC offset)
  - AC measurements shall display peak-to-peak values
  - AC measurements shall work for various waveforms
    - Sine waveforms
    - Saw-tooth waveforms
    - Square waveforms
    - Other periodic waveforms
  - AC measurements shall work for waveforms of various amplitudes and offsets
    - The maximum voltage that shall be measured is 3 V
    - The minimum voltage that shall be measured is 0 V
    - The minimum peak-to-peak voltage that shall be measured is 0.5 V
    - The maximum DC offset value that shall be measured is 2.75 V
- The DMM shall measure frequency.
  - Frequency measurements shall be limited from 1 to 1000 HZ.
  - Frequency measurements shall be accurate to within 10% or 5 Hz whichever is smaller.
  - Frequency measurements shall work for various waveforms.
    - Sine waveforms
    - Saw-tooth waveforms 
    - Square waveforms
    - Other periodic waveforms
- The DMM shall have a terminal-based interface.
  - The terminal shall operate at 115200 baud.
  - The terminal shall utilize VT100 escape codes
    - The terminal shall display all fields in non-changing locations 
  - The terminal shall update at least every 3.5 seconds
  - The terminal shall display AC voltages as described above.
  - The terminal shall display DC voltages as described above.
  - The terminal shall display frequency as described above.
  - The terminal shall organize the presentation of information.
    - AC, DC, and frequency shall be simple to read.
    - The display may use horizontal and vertical lines (borders) to organize the presentation of information.  
  - The terminal shall use character-based bar-graphs for voltages being measured.
    - The bar graphs shall be a single line or column of characters
    - The bar graphs shall have delineators, e.g. a scale, indicating the measured value.
    - The bar graphs shall have a length or height proportional to the value being measured.
    - The bar graphs shall respond in real-time to changes in AC or DC measurements
    - The AC measurements shall have a bar-graph for the RMS voltage.
    - The DC measurement shall have a bar-graph for the DC voltage.
    - Example horizontal bar-graph
      ################
      |-----|-----|-----|-----|-----|-----|
      0    0.5   1.0   1.5  2.0   2.5    3.0
    - Example vertical bar-graph
    3.0 -|
         |
         |
         |
    1.5 -|
         | #
         | #
         | #
    0.0 -| #

## Hints
- Speed up the MCU, running at a minimum of 24 MHz. 
- Run the ADC clock at the fastest clock rate to make the conversion process as quick as possible. Adjust the desired sampling time of the ADC accordingly
- Use a timer to set the start conversion bit at regular intervals for uniform sampling
- Perform the measurement in stages, with the first stage determining the frequency
- A state machine is an ideal mechanism to organize and control the measurement process in stages

## Demonstration
Demonstrate the completed multimeter device with each input specified below for an instructor or lab assistant to complete the signoff sheet.

- DC Voltage
  - Two different DC values between 0 and 3.3 V
- AC Voltage
  - 2.78 Vpp sine wave with 1.5 V DC offset at 286 Hz
  - 1 Vpp sine wave with 0.5 V DC offset at 834 Hz
  - 1 Vpp sine wave with 2.5 V DC offset at  627 Hz
  - 2.62 Vpp saw-tooth wave with 1.5 V DC offset at 54 Hz
  - 0.5 Vpp square wave with 1.5 V DC offset at 10 Hz
- Frequency Specific
  - 3 Vpp sine wave with 1.5 V DC offset at the lowest and highest measurable frequencies (1 Hz and 1 kHz to meet spec)
  - 3 Vpp square wave with 1.5 V DC offset at the lowest and highest measurable frequencies (1 Hz and 1 kHz to meet spec)
