# 24GHz Radar-Based Smart Light
# 🚀 Pico Presence Relay Controller

### Smart Human Presence Detection & Automatic Relay Control Using Raspberry Pi Pico and 24 GHz mmWave Radar

A smart, automatic presence-based relay controller built using the **Raspberry Pi Pico (RP2040)** and the **HLK-LD2420 24 GHz mmWave Human Presence Radar Sensor**.

This project detects human presence, including people sitting or standing still, and automatically controls an electrical relay based on the detected distance.

Unlike traditional PIR motion sensors, the mmWave radar can detect stationary human presence, making this project suitable for smart lighting, desk automation, room appliances, and other presence-based applications.

The system features **three selectable detection modes (Desk, Room, and Outdoor)**, a configurable relay hold-off timer, and a real-time OLED status display.

A single push button allows you to switch between operating modes, while the OLED displays the current mode, human detection status, measured distance, relay state, and automatic OFF countdown.

---

## 📸 Project Overview

**Project Name:** 24GHz Radar-Based Smart Light 
**Microcontroller:** Raspberry Pi Pico (RP2040)  
**Radar Sensor:** HLK-LD2420 (24 GHz mmWave)  
**Display:** 0.91-inch OLED (128 × 32, SSD1306)  
**Relay:** 5 V Single-Channel Relay Module  
**Programming IDE:** Arduino IDE  
**Programming Language:** C++  
**License:** MIT License

---

## 📑 Table of Contents

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [System Architecture](#-system-architecture)
- [Circuit Connections](#-circuit-connections)
- [How It Works](#-how-it-works)
- [Operating Modes](#-operating-modes)
- [OLED Display Interface](#-oled-display-interface)
- [Software Requirements](#-software-requirements)
- [Installation and Setup](#-installation-and-setup)
- [Configuration](#-configuration)
- [Relay Compatibility](#-relay-compatibility)
- [Troubleshooting](#-troubleshooting)
- [Project Structure](#-project-structure)
- [Future Improvements](#-future-improvements)
- [Safety Notice](#-safety-notice)
- [License](#-license)

---

## ✨ Features

- **24 GHz mmWave Human Detection:** Detects both moving and stationary human presence.
- **Automatic Relay Control:** Relay switches ON immediately when valid human presence is detected.
- **15-Second OFF Delay:** Relay automatically switches OFF after 15 seconds without valid detection.
- **Three Detection Modes:** Select between Desk, Room, and Outdoor operating modes.
- **Adjustable Detection Range:** Each mode has its own maximum detection distance.
- **Push Button Control:** A single button switches between the three operating modes.
- **Real-Time OLED Display:** Shows system information on a 0.91-inch 128 × 32 OLED.
- **Distance Monitoring:** Displays the distance reported by the radar sensor.
- **OFF Countdown:** Shows the remaining time before the relay switches OFF.
- **Non-Blocking Program:** Uses `millis()`-based timing instead of blocking delays in the control loop.
- **Configurable Relay Logic:** Supports active-LOW and active-HIGH relay modules.
- **Compact Hardware Design:** Uses a Raspberry Pi Pico with simple UART and I2C connections.

---

## 🧰 Hardware Requirements

| Quantity | Component | Description |
|---|---|---|
| 1 | Raspberry Pi Pico | RP2040 microcontroller (Pico W also supported) |
| 1 | HLK-LD2420 Radar Sensor | 24 GHz human presence detection |
| 1 | 5 V Relay Module | Single-channel relay |
| 1 | 0.91-inch OLED Display | 128 × 32, I2C, SSD1306 |
| 1 | Push Button | Normally open momentary switch |
| Several | Jumper Wires | Circuit connections |
| 1 | USB Cable | Programming and power |

---

## 🏗️ System Architecture

The Raspberry Pi Pico acts as the central controller and communicates with all connected components.

- The HLK-LD2420 sends presence and distance information through UART.
- The Raspberry Pi Pico processes the sensor readings and checks the selected detection range.
- The relay module switches the connected load ON or OFF.
- The push button changes the operating mode.
- The OLED displays real-time system information through I2C.

### Block Diagram

```text
                  +----------------------+
                  |   HLK-LD2420 Radar   |
                  |    24 GHz mmWave     |
                  +----------+-----------+
                             |
                          UART0
                         GP0/GP1
                             |
              +--------------v-------------+
              |                            |
              |     Raspberry Pi Pico      |
              |          RP2040            |
              |                            |
              |   Presence Processing      |
              |   Distance Filtering       |
              |   Relay Timer Control      |
              |   Operating Mode Control   |
              |                            |
              +-----+---------------+------+
                    |               |
                  GP15            GP4/GP5
                    |               |
             +------v------+  +-----v------+
             | 5V Relay    |  | OLED       |
             | Module      |  | 128 x 32   |
             +------+------+\ +------------+
                    |
                Electrical
                   Load

              GP14
                |
           +----v----+
           | Button  |
           +---------+
```

---

## 🔌 Circuit Connections

### Raspberry Pi Pico Pin Mapping

| Device | Device Pin | Pico Connection | Pico Physical Pin |
|---|---|---|---|
| LD2420 | 3V3 | 3V3 OUT | Pin 36 |
| LD2420 | GND | GND | Pin 38 |
| LD2420 | OT1 (TX) | GP1 (UART0 RX) | Pin 2 |
| LD2420 | RX | GP0 (UART0 TX) | Pin 1 |
| Relay | VCC | VBUS (5 V) | Pin 40 |
| Relay | GND | GND | Pin 38 |
| Relay | IN | GP15 | Pin 20 |
| OLED | VCC | 3V3 OUT | Pin 36 |
| OLED | GND | GND | Pin 38 |
| OLED | SDA | GP4 (I2C0 SDA) | Pin 6 |
| OLED | SCL | GP5 (I2C0 SCL) | Pin 7 |
| Button | Leg 1 | GP14 | Pin 19 |
| Button | Leg 2 | GND | Pin 38 |

### Important Wiring Notes

- The HLK-LD2420 is a 3.3 V device. **Never connect its power input to 5 V.**
- Connect sensor TX to Pico RX and sensor RX to Pico TX.
- Connect all low-voltage circuit grounds together.
- The relay module should receive its required 5 V supply.
- Use a suitable 3.3 V-compatible UART interface.
- The button uses the Pico's internal pull-up resistor.

---

## ⚙️ How It Works

The controller operates using a continuous sensor-monitoring and relay-control process.

### 1. Human Presence Detection

The HLK-LD2420 radar sensor continuously monitors its detection area and provides sensor information to the Pico through UART0 at 115200 baud.

The program processes the sensor's configured output format, including presence state and distance information.

### 2. Distance Filtering

The measured distance is compared against the maximum detection range of the currently selected operating mode.

For example:

- Desk Mode: Maximum 100 cm
- Room Mode: Maximum 400 cm
- Outdoor Mode: Maximum 700 cm

Only valid detections inside the selected range activate the relay.

### 3. Automatic Relay Activation

When valid human presence is detected:

- The relay immediately switches ON.
- The last detection timestamp is updated.
- The OLED displays HUMAN DETECTED.
- The measured distance is displayed.

### 4. Automatic Relay Deactivation

When no valid detection is received, the controller starts checking the 15-second hold-off period.

If no valid detection occurs during this period, the relay switches OFF automatically.

If a person is detected again before the countdown finishes, the timer resets and the relay remains ON.

### 5. Operating Mode Switching

Press the push button to switch between:

```text
DESK → ROOM → OUTDOOR → DESK
```

The new mode immediately changes the maximum accepted detection range.

### 6. OLED Status Monitoring

The OLED refreshes approximately every 200 milliseconds and displays the latest system information.

---

## 🎚️ Operating Modes

The controller provides three operating modes for different environments.

| Mode | Maximum Detection Range | Recommended Application |
|---|---:|---|
| DESK | 100 cm | Desk lamp, monitor, personal fan |
| ROOM | 400 cm | Room lighting, AC, fan automation |
| OUTDOOR | 700 cm | Garage, porch, passage lighting |

### Mode Selection

A single push button controls all three modes.

```text
       +----------------+
       |   DESK MODE    |
       |    100 cm      |
       +-------+--------+
               |
           Button Press
               |
       +-------v--------+
       |   ROOM MODE    |
       |    400 cm      |
       +-------+--------+
               |
           Button Press
               |
       +-------v--------+
       |  OUTDOOR MODE  |
       |    700 cm      |
       +-------+--------+
               |
           Button Press
               |
               +----------> DESK MODE
```

**Note:** These values represent software distance limits, not guaranteed physical detection ranges. Actual performance depends on sensor configuration, mounting position, surroundings, and radar sensitivity.

---

## 🖥️ OLED Display Interface

The project uses a 0.91-inch OLED display with a 128 × 32 resolution.

### Example Display

```text
MODE:ROOM       RLY:ON
HUMAN DETECTED
Dist:230cm OFF in 12s
```

### Display Information

| Display Line | Information |
|---|---|
| Line 1 | Selected mode and relay status |
| Line 2 | Human presence status |
| Line 3 | Measured distance and OFF countdown |

The display updates automatically, allowing you to monitor the controller without connecting it to a computer.

---

## 💻 Software Requirements

### Required Software

1. Arduino IDE 2.x
2. Raspberry Pi Pico / RP2040 board package
3. Adafruit SSD1306 library
4. Adafruit GFX Library

### Raspberry Pi Pico Board Package

Add the following URL to:

`File → Preferences → Additional Boards Manager URLs`

```text
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

Open Boards Manager and install:

**Raspberry Pi Pico/RP2040 by Earle F. Philhower III**

### Required Arduino Libraries

Install these libraries using:

`Sketch → Include Library → Manage Libraries`

- Adafruit SSD1306
- Adafruit GFX Library

---

## 🚀 Installation and Setup

Follow these steps to get the project running.

**Step 1 – Download the Repository**

Clone or download this GitHub repository to your computer.

```bash
git clone https://github.com/YOUR-USERNAME/pico-presence-relay-controller.git
```

**Step 2 – Install Required Libraries**

Install the Adafruit SSD1306 and Adafruit GFX libraries using Arduino Library Manager.

**Step 3 – Open the Arduino Sketch**

Open:

```text
pico_presence_relay.ino
```

**Step 4 – Select the Board**

In Arduino IDE, select:

```text
Tools → Board → Raspberry Pi Pico
```

Select the appropriate COM port when available.

**Step 5 – Upload the Code**

Connect the Pico to your computer. If required for the first upload, hold BOOTSEL while connecting the USB cable.

Upload the sketch.

**Step 6 – Test the Controller**

After uploading:

1. Power the radar sensor and OLED.
2. Wait for the sensor to initialize.
3. Check the OLED status.
4. Move within the selected detection range.
5. Confirm that the relay switches ON.
6. Leave the detection area and wait 15 seconds.
7. Confirm that the relay switches OFF.
8. Press the button to test the other modes.

---

## 🔧 Configuration

All important settings can be modified near the beginning of the Arduino sketch.

### Pin Configuration

```cpp
#define RELAY_PIN        15
#define BUTTON_PIN       14

#define RELAY_ACTIVE_LOW true
```

### Relay OFF Delay

```cpp
const unsigned long OFF_DELAY_MS = 15000;
```

Change the value to customize the relay hold-off time.

| Value | Delay |
|---|---|
| 5000 | 5 seconds |
| 10000 | 10 seconds |
| 15000 | 15 seconds |
| 30000 | 30 seconds |
| 60000 | 60 seconds |

### Detection Range Configuration

```cpp
const ModeCfg modes[MODE_COUNT] = {
  { "DESK",    100 },
  { "ROOM",    400 },
  { "OUTDOOR", 700 }
};
```

You can change these values according to your application.

**Remember:** The sensor must provide valid distance readings in the configured output format for the software filtering to work correctly.

---

## ⚡ Relay Compatibility

Some inexpensive single-channel relay modules do not work reliably with the Pico's 3.3 V GPIO output.

A common symptom is that the relay remains ON even when the program attempts to switch it OFF.

For compatible modules that require an open-drain-style control signal, you can use the following alternative relay function:

```cpp
void setRelay(bool on) {
  relayOn = on;

  if (on) {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
  } else {
    pinMode(RELAY_PIN, INPUT);
  }
}
```

When using this alternative:

- Remove `pinMode(RELAY_PIN, OUTPUT);` from the normal setup initialization.
- Ensure the module's input circuit is compatible with this arrangement.
- Do not use this method blindly with every relay board.

For conventional relay modules, use the standard relay-control function and configure the correct active-LOW or active-HIGH polarity.

---

## 🛠️ Troubleshooting

| Problem | Possible Cause | Solution |
|---|---|---|
| Relay always ON | Incorrect relay logic or incompatible input circuit | Check relay polarity or use a compatible open-drain interface |
| Relay logic inverted | Active-HIGH/LOW mismatch | Change `RELAY_ACTIVE_LOW` |
| OLED display blank | Incorrect wiring or I2C address | Check SDA/SCL and try address `0x3D` |
| No human detection | Incorrect UART wiring or sensor configuration | Check TX/RX and sensor output |
| Incorrect distance readings | Sensor output format mismatch | Verify the LD2420 configuration |
| Relay switches randomly | Unstable power supply | Use a suitable power supply and add appropriate decoupling |
| Detection range incorrect | Mode range settings or sensor configuration | Adjust `modes[]` and sensor sensitivity |
| Button switches modes repeatedly | Button bounce or wiring problem | Check the debounce implementation and button connections |

### Important Sensor Note

The HLK-LD2420 output format depends on its firmware and configuration.

This project expects the sensor data to be parsed into presence state and distance information. If your sensor outputs binary data or a different serial format, the UART parser must be adapted accordingly.

---

## 📁 Project Structure

```text
Pico-Presence-Relay-Controller/
│
├── pico_presence_relay.ino
│
├── README.md
│
└── LICENSE
```

| File | Description |
|---|---|
| `pico_presence_relay.ino` | Main Arduino program |
| `README.md` | Project documentation |
| `LICENSE` | MIT License |

---

## 🔮 Future Improvements

Some interesting features that could be added in future versions:

- [ ] Save the selected operating mode in flash memory.
- [ ] Adjustable relay OFF delay using the push button.
- [ ] Configurable radar sensitivity.
- [ ] Wi-Fi-based remote monitoring using Raspberry Pi Pico W.
- [ ] MQTT integration for Home Assistant.
- [ ] Web-based control and monitoring dashboard.
- [ ] Real-time detection history and event logging.
- [ ] Custom 3D-printed enclosure.
- [ ] Multiple relay channel support.

---

## ⚠️ Safety Notice

**Electrical safety is extremely important.**

If you connect this project to mains-powered appliances (110 V / 230 V AC):

- Disconnect the power supply before making connections.
- Use a properly rated relay module.
- Use an appropriately rated fuse.
- Keep high-voltage wiring physically separated from low-voltage electronics.
- Use a proper insulated enclosure.
- Never touch exposed mains terminals while powered.
- Get assistance from a qualified electrician if you are unfamiliar with mains wiring.

This project is intended for educational and DIY automation purposes. Use appropriate electrical protection when controlling real appliances.

---

## 📜 License

This project is released under the **MIT License**.

You are free to use, modify, distribute, and improve this project according to the terms of the license.

See the [LICENSE](LICENSE) file for details.

---

## ❤️ Contributing

Have an idea to improve this project?

Contributions, suggestions, bug reports, and feature requests are welcome!

If you build this project, feel free to share your implementation and improvements.

---

## ⭐ Support the Project

If you find this project useful:

- Give this repository a ⭐ Star.
- Fork the repository and experiment with new features.
- Share your improvements.
- Report bugs and suggest new ideas.

**Built by Dipankar Bhunia**

*Making everyday automation smarter, one DIY project at a time!*
