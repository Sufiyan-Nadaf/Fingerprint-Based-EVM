# 🔐 Biometric EVM — Fingerprint-Based Electronic Voting Machine

An Arduino-based electronic voting machine prototype that integrates fingerprint authentication with electronic voting. The system demonstrates biometric verification, candidate selection, vote counting, and hardware-software integration using an Arduino UNO R3.

<p align="center">
  <strong>Embedded Systems • Biometric Authentication • Arduino • Electronics</strong>
</p>

---

## 📌 Table of Contents

- [Overview](#-overview)
- [Project Objectives](#-project-objectives)
- [Features](#-features)
- [Project Demonstration](#-project-demonstration)
- [Hardware Components](#-hardware-components)
- [Hardware Connections](#-hardware-connections)
- [Project Structure](#-project-structure)
- [How It Works](#-how-it-works)
- [Getting Started](#-getting-started)
- [Testing Checklist](#-testing-checklist)
- [Limitations and Security](#-limitations-and-security)
- [Future Improvements](#-future-improvements)
- [Applications](#-applications)
- [Author](#-author)
- [License](#-license)

---

## 📖 Overview

The **Biometric EVM** is a hardware-based project designed to demonstrate how fingerprint authentication can be integrated into an electronic voting system.

The prototype uses an Arduino UNO R3 as its main controller. A fingerprint sensor verifies enrolled fingerprints, while physical push buttons allow candidate selection. An LCD displays system messages, and LEDs and a buzzer provide feedback.

The project combines embedded C/C++ programming, biometric sensor interfacing, digital electronics, and hardware integration in a practical engineering application.

> **Project type:** Academic and educational hardware prototype  
> **Controller:** Arduino UNO R3  
> **Biometric sensor:** R307 fingerprint sensor  
> **Display:** 16×2 LCD

**Note:** This is an educational prototype, not a certified or election-ready voting system.

---

## 🎯 Project Objectives

- Develop a fingerprint-based voter authentication prototype.
- Interface a biometric fingerprint sensor with an Arduino microcontroller.
- Implement candidate selection using physical push buttons.
- Display system instructions and status messages on an LCD.
- Provide visual and audio feedback using LEDs and a buzzer.
- Demonstrate electronic vote counting and the associated control logic.
- Gain practical experience in embedded systems and hardware-software integration.

---

## ✨ Features

- **Fingerprint Authentication:** Uses a fingerprint sensor to verify enrolled fingerprints.
- **Fingerprint Enrollment:** Supports registration of fingerprints through the enrollment controls implemented in the program.
- **Candidate Selection:** Uses physical push buttons to select candidates.
- **LCD Interface:** Displays instructions and system status.
- **LED Indicators:** Provides visual feedback for relevant system states.
- **Buzzer Alerts:** Provides audio feedback.
- **Mode Control:** Supports the operating modes implemented in the Arduino code.
- **Vote Counting:** Records and counts votes according to the implemented logic.
- **Reset Control:** Provides reset functionality according to the program's implementation.
- **Physical Hardware Prototype:** Integrates the controller, biometric sensor, display, and input/output components.

*Feature behavior depends on the actual implementation in `src/EVM_Code.ino`. Confirm each feature against your working prototype.*

---

## 📸 Project Demonstration

### Front View

![Biometric EVM - Front View](images/Front%20View.jpeg)

The front view shows the assembled electronic voting machine prototype.

### Top View

![Biometric EVM - Top View](images/Top%20View.jpeg)

The top view provides another perspective of the hardware assembly and component arrangement.

---

## 🧰 Hardware Components

The following components are used in the prototype.

| No. | Component | Purpose |
|---|---|---|
| 1 | Arduino UNO R3 | Main microcontroller |
| 2 | R307 Fingerprint Sensor | Fingerprint enrollment and verification |
| 3 | 16×2 LCD | Displays instructions and system status |
| 4 | Push Buttons | Candidate selection and system controls |
| 5 | Green LED | Visual status indication |
| 6 | Red LED | Visual status indication |
| 7 | Buzzer | Audio feedback |
| 8 | Potentiometer | LCD contrast adjustment |
| 9 | Resistors | Current limiting and circuit protection where required |
| 10 | Breadboard | Circuit assembly |
| 11 | Jumper Wires | Electrical connections |
| 12 | USB Cable / Suitable Power Supply | Programming and powering the prototype |

See the complete component list in [`hardware/Component_List.png`](hardware/Component_List.png).

---

## 🔌 Hardware Connections

The hardware connection diagram is available here:

**[View Hardware Connections](hardware/Connections.png)**

The following is the documented pin mapping. Verify it against your physical circuit and the Arduino source code before making changes.

### LCD Connections

The LCD uses 4-bit communication.

| LCD Pin | Arduino Pin |
|---|---|
| RS | D4 |
| EN | D5 |
| D4 | D6 |
| D5 | D7 |
| D6 | D8 |
| D7 | D9 |
| VSS | GND |
| VDD | 5V |
| VO | Potentiometer middle pin |

Connect the LCD backlight according to the display module's specifications.

### Fingerprint Sensor Connections

| R307 Pin | Arduino Connection |
|---|---|
| VCC | Supply voltage specified for your sensor |
| GND | GND |
| TX | D0 / RX |
| RX | D1 / TX |

**Important:** Arduino UNO pins D0 and D1 are shared with USB serial communication. The fingerprint sensor can interfere with uploading sketches or serial debugging when connected. Disconnecting the sensor's serial wires during uploading may be necessary.

Verify the sensor's supply voltage and wiring requirements before powering it.

### Buttons and Indicators

| Component | Arduino Pin |
|---|---|
| Candidate Button 1 | D10 |
| Candidate Button 2 | D11 |
| Candidate Button 3 | D12 |
| Candidate Button 4 | D13 |
| Mode Control Button | A0 |
| Enrollment Button | A1 |
| Reset Button | A2 |
| Green LED | A3 |
| Red LED | A4 |
| Buzzer | A5 |

For buttons configured with `INPUT_PULLUP`, connect one side of the button to its assigned input and the other side to GND.

Use appropriate current-limiting resistors for LEDs and ensure the buzzer is compatible with the Arduino output or has a suitable driver circuit.

> **Note:** The pin mapping above should match your actual Arduino code and wiring. Treat the source code and verified circuit as the final reference.

---

## ⚙️ How It Works

The general workflow of the prototype is:

1. **Initialization:** The Arduino initializes the LCD, fingerprint sensor, buttons, and output indicators.
2. **Fingerprint Enrollment:** An operator registers a fingerprint using the enrollment controls and the instructions implemented in the program.
3. **Fingerprint Verification:** The voter places a finger on the sensor, which attempts to match it against enrolled fingerprint templates.
4. **Authentication:** The program responds to the fingerprint match result and applies its implemented authorization logic.
5. **Candidate Selection:** The voter selects a candidate using the corresponding physical push button.
6. **Vote Recording:** The program updates its vote records according to the implemented storage and counting logic.
7. **Feedback:** The LCD, LEDs, and buzzer provide feedback according to the current system state.
8. **Results and Reset:** Result display and reset behavior depend on the functions implemented in the Arduino program.

---

## 📂 Project Structure

The repository is organized as follows:

```text
Biometric_EVM/
│
├── docs/
│   └── Report.docx
│
├── hardware/
│   ├── Component_List.png
│   └── Connections.png
│
├── images/
│   ├── Front View.jpeg
│   └── Top View.jpeg
│
├── src/
│   └── EVM_Code.ino
│
├── .gitignore
├── LICENSE
└── README.md
```

### Folder Description

| File / Folder | Description |
|---|---|
| `docs/` | Contains the project report. |
| `hardware/` | Contains the component list and hardware connection diagram. |
| `images/` | Contains photographs of the physical prototype. |
| `src/` | Contains the Arduino source code. |
| `.gitignore` | Specifies files Git should ignore. |
| `LICENSE` | Defines the permissions and conditions for using the project. |
| `README.md` | Contains the project's documentation and instructions. |

---

## 🚀 Getting Started

### Prerequisites

Before running the project, you need:

- An Arduino UNO R3.
- An R307 fingerprint sensor.
- A 16×2 LCD and potentiometer.
- Push buttons, LEDs, a buzzer, resistors, and jumper wires.
- The Arduino IDE.
- The fingerprint sensor library required by the source code.
- The project source file: `src/EVM_Code.ino`.

### Installation and Setup

**Step 1: Clone the repository**

```bash
git clone https://github.com/Sufiyan-Nadaf/Biometric_EVM.git
```

**Step 2: Open the Arduino sketch**

Open the Arduino IDE and load:

```text
src/EVM_Code.ino
```

**Step 3: Install the required library**

Check the `#include` statements in the sketch and install the compatible fingerprint sensor library through the Arduino IDE Library Manager.

**Step 4: Assemble the hardware**

Connect the Arduino, fingerprint sensor, LCD, buttons, LEDs, and buzzer using the verified connection diagram.

**Step 5: Select the board and port**

In the Arduino IDE, select:

- **Board:** Arduino UNO
- **Port:** The port corresponding to your connected Arduino

**Step 6: Upload the sketch**

Upload the program to the Arduino. If the fingerprint sensor is connected to D0 and D1, disconnect those serial connections during uploading if necessary, then reconnect them afterward.

**Step 7: Test the prototype**

Power on the system and follow the prompts displayed on the LCD.

---

## 🧪 Testing Checklist

Use this checklist to document your hardware testing.

- [ ] Arduino initializes correctly.
- [ ] LCD displays the expected startup message.
- [ ] Fingerprint sensor initializes successfully.
- [ ] Fingerprint enrollment works.
- [ ] Enrolled fingerprints are recognized.
- [ ] Unregistered fingerprints are handled correctly.
- [ ] Candidate selection buttons work.
- [ ] Vote counts update correctly.
- [ ] LEDs provide the expected feedback.
- [ ] Buzzer provides the expected alerts.
- [ ] Mode control operates correctly.
- [ ] Reset behavior matches the intended design.
- [ ] The system behaves as expected after power cycling.
- [ ] Repeated voting by the same fingerprint is handled according to the intended rules.

Only mark an item complete after verifying it on the actual prototype.

---

## 🔒 Limitations and Security

This project is intended for educational and demonstration purposes.

Important limitations include:

- Fingerprint authentication alone does not establish a person's legal eligibility to vote.
- The prototype has not been certified for public elections.
- The system has not undergone an independent security audit.
- Physical access to the device may create opportunities for tampering.
- Vote integrity and recovery depend on the storage and reset mechanisms implemented in the code.
- A production voting system requires robust security, privacy, auditability, accessibility, and procedural safeguards.

**This prototype must not be used for real public elections or other high-stakes voting applications.**

---

## 🔮 Future Improvements

Potential extensions include:

- Connecting multiple voting machines to a centralized dashboard.
- Developing a web interface for monitoring test sessions.
- Improving voter and poll management.
- Strengthening vote storage and recovery mechanisms.
- Adding tamper detection and secure access controls.
- Designing a custom PCB.
- Improving communication between the fingerprint sensor and microcontroller.
- Adding automated tests and detailed test results.
- Conducting security testing and documenting the findings.

These are possible future enhancements and should not be interpreted as existing features.

---

## 💡 Applications

The project can be used for:

- Embedded systems demonstrations.
- Biometric authentication experiments.
- Arduino and peripheral interfacing.
- Electronic voting workflow demonstrations.
- Hardware-software integration projects.
- Engineering project demonstrations and learning.

---

## 👨‍💻 Author

**Sufiyan Nadaf**

B.Tech — Electronics and Computer Science

GitHub: [Sufiyan-Nadaf](https://github.com/Sufiyan-Nadaf)

---

## 📜 License

See the [`LICENSE`](LICENSE) file in this repository for the applicable license terms.

---

⭐ If you find this project interesting, consider starring the repository!