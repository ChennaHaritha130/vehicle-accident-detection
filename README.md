# Vehicle Accident Detection System

## Overview
An Arduino-based vehicle safety system designed to detect potential accidents, monitor alcohol levels, and send emergency alerts with GPS location information.

## Key Features
- Alcohol detection using the MQ-3 sensor
- Accident detection using the ADXL345 accelerometer
- Relay-based engine simulation and control
- LCD status display and buzzer alerts
- GPS location retrieval and GSM emergency notifications
- Push-button cancellation of false accident alerts

## Hardware Components
- Arduino Uno
- MQ-3 alcohol sensor
- ADXL345 accelerometer
- GPS and GSM modules
- I2C LCD
- Relay module, buzzer, push button, and DC motor

## Working Principle
The Arduino monitors alcohol levels and vehicle motion. If alcohol exceeds the configured threshold, the system activates warning and relay control. When a potential accident is detected, a cancellation timer starts. If the alert is not cancelled, the system attempts to obtain GPS coordinates and send an emergency message through GSM.

## Implementation
The project integrates sensor interfacing, embedded control logic, display output, relay control, and GPS/GSM communication.

## My Contribution
Designed the hardware circuit and worked on component connections as part of the project team.

