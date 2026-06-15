# PillReminder

<p align="center">
  <h3 align="center">Smart Medication Adherence Platform</h3>
  <p align="center">
    A complete platform that combines a mobile application and an IoT device to improve medication adherence and treatment monitoring.
  </p>
</p>

---

## About

PillReminder is a medication management platform designed to help users and caregivers maintain treatment routines through a combination of mobile technology and smart hardware.

The project integrates a mobile application developed in FlutterFlow with an ESP32-based physical device capable of assisting users during medication administration.

---

## Main Features

### Mobile Application

- User authentication
- User onboarding
- Medication scheduling
- Smart reminders
- Device pairing
- Manual device linking
- Medication administration history
- User profile management

### Smart Device

- ESP32 firmware
- Wi-Fi connectivity
- Physical confirmation buttons
- Medication compartment management
- Real-time synchronization

---

# Application Screens

## Authentication

![Authentication](screenshots/AuthenticationPage.png)

---

## User Registration

![User Registration](screenshots/NameUserPage.png)

---

## Home Dashboard

![Home](screenshots/HomePage.png)

---

## Device Pairing

### Automatic Connection

![Already Linked](screenshots/AlreadyLinkedPage.png)

### Device Search

![Hardware Link](screenshots/HardwareLinkPage.png)

### Confirmation

![Link Confirm](screenshots/LinkConfirmPage.png)

### Manual Connection

![Manual Link](screenshots/ManualLinkPage.png)

---

## Medication Schedule

![Timer](screenshots/TimerPage.png)

---

## User Profile

![Profile](screenshots/ProfilePage.png)

---

## System Architecture

```text
                 Mobile Application
                         │
                         │
                    Wi-Fi Sync
                         │
                         ▼
                 Smart Device
                         │
                         ▼
                       ESP32
```

---

## Technologies

### Mobile

- FlutterFlow
- Flutter
- Dart

### Embedded Systems

- ESP32
- Arduino Framework

### Design

- Figma
- Fusion 360

### Version Control

- Git
- GitHub

---

## Current Progress

### Completed

- Mobile application
- User authentication
- Device pairing system
- Medication scheduling
- ESP32 integration
- Functional hardware prototype
- Functional enclosure prototype

### In Development

- PCB design
- Final enclosure version
- Product optimization

---

## Repository Structure

```text
pillreminder/
│
├── screenshots/
├── esp32/
├── docs/
└── README.md
```

---

## Project Goal

PillReminder aims to simplify medication management by providing an accessible ecosystem that combines mobile software and embedded hardware into a single experience.

---

## Author

Raphael Soares

GitHub: https://github.com/raphaelsoares01
