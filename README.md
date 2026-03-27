# Real-Time Target Tracking Laser Turret

Embedded system for real-time target detection and tracking using TM4C123GXL microcontroller.

---

## 📌 Overview

This project implements an autonomous laser turret that detects and tracks objects using an ultrasonic sensor. The system performs continuous scanning and locks onto targets within a defined threshold distance.

The design integrates sensing, actuation, and display modules to simulate a real-time embedded tracking system for surveillance and robotics applications.

---

## 🚀 Features

- Real-time object detection using HC-SR04 ultrasonic sensor  
- Pan-tilt tracking using dual servo motors (0°–180°)  
- Automatic target locking within **20 cm range**  
- Laser activation on target detection  
- OLED display for real-time feedback (distance / lock status)  
- Continuous scanning with smooth servo control  

---

## 🧠 System Architecture

The system consists of:

- **Microcontroller**: TM4C123GXL (control unit)  
- **Sensor**: Ultrasonic sensor (distance measurement)  
- **Actuators**: Servo motors (pan & tilt motion)  
- **Output**: Laser module (target indication)  
- **Display**: OLED (I²C communication)  

---

## 🔄 Working

1. Ultrasonic sensor measures object distance  
2. Distance is processed by TM4C123GXL  
3. If distance ≤ 30 cm:
   - Servo movement stops  
   - Laser is activated  
   - OLED displays **"TARGET LOCKED"**  
4. Otherwise:
   - System continues scanning  
   - Distance is displayed on OLED  

---

## 📊 Results

- Distance measurement accuracy: **±1 cm**  
- Servo motion range: **0°–180°**  
- Stable real-time tracking achieved  
- Reliable target locking within threshold range  

---

## 🎥 Demo

[Watch Demo Video](https://drive.google.com/file/d/1wN-HqL7u_J5BtfhbBMFW_SnpxHUnpaQ0/view?usp=drive_link)

---

## 📄 Documentation

[Detailed Project Report](docs/project_report.pdf)

---

## 🛠 Hardware Components

- TM4C123GXL LaunchPad  
- HC-SR04 Ultrasonic Sensor  
- MG995 Servo Motors (x2)  
- Laser Module  
- SSD1306 OLED Display (I²C)  
- Breadboard, resistors, wiring  

---

## 💻 Software

- Energia IDE  
- TivaWare Peripheral Library  
- Embedded C  

---

## 📌 Applications

- Surveillance and security systems  
- Robotics and automation  
- Target tracking systems  

---

## 🔮 Future Work

- Camera-based tracking using OpenCV  
- AI-based object recognition  
- Wireless control (Wi-Fi/Bluetooth)  

---

## 👤 Author

Ansh Verma  
