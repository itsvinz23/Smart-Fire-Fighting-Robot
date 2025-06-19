<!-- HEADER -->
<p align="center">
  <img src="https://d1lmq142maiv1z.cloudfront.net/NIBM_City_University_Logo_2_213b4dd2f6.svg" alt="NIBM City University Logo" width="200"/>
</p>

<h1 align="center">🚒 Smart Fire-Fighting Robot 🔥</h1>
<h3 align="center">The project is submitted in partial fulfilment of the requirement of the Higher National Diploma in Software Engineering National Institute of Business Management</h3>
<p align="center">
  An IoT-based autonomous robot that detects and extinguishes fire using a NodeMCU, Firebase, and a mobile app.
</p>

<p align="center">
  <img src="https://media.giphy.com/media/NMufrvxO8a75m/giphy.gif" width="300"/>
</p>

<p align="center">
  <a href="#"><img alt="Platform" src="https://img.shields.io/badge/platform-NodeMCU%20ESP8266-blue?style=for-the-badge"></a>
  <a href="#"><img alt="License" src="https://img.shields.io/github/license/itsvinz23/Smart-Fire-Fighting-Robot?style=for-the-badge"></a>
  <a href="#"><img alt="Contributions" src="https://img.shields.io/badge/contributions-welcome-brightgreen?style=for-the-badge"></a>
</p>

---

## 🚀 Overview

The **Smart Fire-Fighting Robot** is a fully integrated solution to detect and extinguish fire. This smart bot can:

- 🔥 Detect flames using flame sensors  
- 💨 Identify smoke using an MQ2 sensor  
- 📱 Be controlled remotely via a Firebase-powered mobile app  
- 🤖 Move autonomously or via manual input  
- 💧 Spray water via a pump connected to a servo-mounted nozzle  

---

## 🧩 Features

| 🛠 Feature              | 💬 Description                          |
|------------------------|------------------------------------------|
| 🔥 Fire & Smoke Detection | IR Flame sensor + MQ2 gas sensor        |
| 📡 Wireless Control     | Firebase + Android App                  |
| 🤖 Movement             | 4-wheel base with motor driver          |
| 🔄 Obstacle Avoidance   | IR sensors for object detection         |
| 💧 Pump Activation      | Servo-aligned water spray system        |
| 🌐 Real-time Sync       | Firebase Realtime Database              |

---

## 📸 Demo Preview

<p align="center">
  <img src="https://media.giphy.com/media/WoWm8YzFQJg5i/giphy.gif" width="400"/>
</p>

*Replace this with your actual robot demo GIF.*

---

## 🔌 Hardware Components

| Component            | Quantity | Description                     |
|---------------------|----------|---------------------------------|
| NodeMCU ESP8266     | 1        | Main microcontroller            |
| Flame Sensor        | 1        | Detects fire                    |
| MQ2 Gas Sensor      | 1        | Detects smoke/gas               |
| IR Obstacle Sensors | 2–3      | For object detection            |
| L298N Motor Driver  | 1        | Controls motors                 |
| Servo Motor         | 1        | Adjusts water nozzle            |
| Mini Water Pump     | 1        | Sprays water                    |
| DC Motors (BO)      | 4        | Robot movement                  |
| Battery (12V)       | 1        | Power source                    |

---

## 📍 Pin Mapping

| Pin | Function                     |
|-----|------------------------------|
| D1  | Motor Left Forward           |
| D2  | Motor Left Backward          |
| D3  | Motor Right Forward          |
| D4  | Servo Motor (Water Nozzle)   |
| D5  | Flame Sensor                 |
| D6  | MQ2 Gas Sensor               |
| D7  | Water Pump                   |
| D8  | IR Sensors                   |
| A0  | Optional IR Sensor           |

---

## 📲 Mobile App Interface

Built using **MIT App Inventor**, the app provides:

- 🔘 Manual control buttons (FWD, BWD, LFT, RGT)  
- 🔥 Fire/Smoke status  
- 💧 Pump control toggle  
- 🔋 Battery/Status indicators  

<p align="center">
  <img src="https://media.giphy.com/media/jpbnoe3UIa8TU8LM13/giphy.gif" width="250"/>
</p>

---

## 🔥 Firebase Database Structure

```json
{
  "fire": true,
  "smoke": false,
  "status": "Alert",
  "control": {
    "move": "forward",
    "spray": "on"
  }
}
## 🔥 Firebase Database Structure

```json
{
  "fire": true,
  "smoke": false,
  "status": "Alert",
  "control": {
    "move": "forward",
    "spray": "on"
  }
}
```

✅ Realtime database updates  
📡 Bidirectional sync between app and NodeMCU

---

## 🧠 How It Works
- Flame & gas sensors constantly monitor the environment  
- NodeMCU processes signals and pushes alerts to Firebase  
- App receives alerts and allows user to respond manually  
- Robot can navigate toward the fire  
- Servo positions the water nozzle  
- Pump activates and sprays water  

---

## 🛠️ Getting Started

### 1️⃣ Arduino Setup
- Install Arduino IDE  
- Add ESP8266 Board via URL:  
  `http://arduino.esp8266.com/stable/package_esp8266com_index.json`  
- Upload code from `/firmware`  

### 2️⃣ Firebase Setup
- Go to Firebase Console  
- Enable Realtime Database  
- Add credentials in the `.ino` file  

---

## 📁 Folder Structure

```
Smart-Fire-Fighting-Robot/
│
├── firmware/              # Arduino .ino file
├── mobile-app/            # MIT App Inventor project
├── assets/                # GIFs, images, media
└── README.md              # Project documentation
```

---

## 🔮 Future Enhancements
- 📷 Add camera for visual monitoring  
- 🧭 GPS integration for location tracking  
- 🧠 Use AI for smart flame recognition  
- 🚀 Auto home-return after mission  

---

## 🙋‍♂️ Contributors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/itsvinz23">
        <img src="https://img.icons8.com/?size=100&id=12599&format=png&color=000000" width="100px;" alt=""/>
        <br />
        <sub><b>Janindu Vinura (Vinz)</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/cha0005">
        <img src="https://img.icons8.com/?size=100&id=12599&format=png&color=000000" width="100px;" alt=""/>
        <br />
        <sub><b>P.D.C.DE SILVA (cha0005)</b></sub>
      </a>
    </td><td align="center">
      <a href="https://github.com/JayDeRukz">
        <img src="https://img.icons8.com/?size=100&id=12599&format=png&color=000000" width="100px;" alt=""/>
        <br />
        <sub><b>Jithmitha Rukshan (Rookie_xR)</b></sub>
      </a>
    </td><td align="center">
      <a href="https://github.com/Ravindurrl">
        <img src="https://img.icons8.com/?size=100&id=12599&format=png&color=000000" width="100px;" alt=""/>
        <br />
        <sub><b>Ravindu Hettiarachchi (Ravindurrl)</b></sub>
      </a>
    </td><td align="center">
      <a href="https://github.com/TharushiNK">
        <img src="https://img.icons8.com/?size=100&id=12599&format=png&color=000000" width="100px;" alt=""/>
        <br />
        <sub><b>Tharushi Nikethana (TharushiNK)</b></sub>
      </a>
    </td>
  </tr>
</table>

---

## 🏫 Project by

<p align="center">
  <img src="https://d1lmq142maiv1z.cloudfront.net/NIBM_City_University_Logo_2_213b4dd2f6.svg" width="150" alt="NIBM Logo"/>
</p>

**National Institute of Business Management**  
Sri Lanka · 2025

---

## 📜 License

This project is licensed under the MIT License.

