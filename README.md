# Offline Mesh Network using ESP-NOW

An offline, peer-to-peer IoT communication system utilizing ESP32 microcontrollers to facilitate secure, router-independent messaging. This project implements a custom hardware interface for inter-node communication, secured by a lightweight cryptographic cipher and extended by a Bluetooth (BLE) gateway for mobile monitoring.

## System Features

- **Decentralized Communication:** Utilizes the ESP-NOW protocol for low-latency, Wi-Fi router-independent data transmission between ESP32 nodes.
- **Hardware Interface:** Integrates a 4x4 matrix keypad for user input, a 16x02 I2C LCD for message display, and a piezoelectric buzzer for auditory alerts.
- **Data Security:** Implements a symmetric XOR encryption cipher (`0xAA`) at the payload level to obscure broadcasted messages from unauthorized interception.
- **Mobile Gateway:** Features a Bluetooth Serial integration (`ESP32_AlertNode`) allowing paired smartphones to receive critical system alerts remotely.

## Hardware Pin Mapping

The system relies on specific GPIO assignments to avoid hardware conflicts (specifically isolating GPIO 14 from the buzzer on Node B).

| Component | ESP32 GPIO (Node A & B) |
| :--- | :--- |
| **I2C LCD (SDA)** | GPIO 21 |
| **I2C LCD (SCL)** | GPIO 22 |
| **Keypad Rows (1-4)** | GPIO 19, 18, 32, 33 |
| **Keypad Cols (1-4)** | GPIO 25, 26, 27, 14 |
| **Buzzer** | GPIO 13 (Node B) |

## Message Matrix

The 4x4 matrix keypad is mapped to transmit predefined strings over the mesh network.

| Key | Message Transmitted | Key | Message Transmitted |
| :--- | :--- | :--- | :--- |
| **1** | Hello | **8** | Break[cite: 16] |
| **2** | Meet me[cite: 16] | **9** | Ok[cite: 16] |
| **3** | File Ready[cite: 16] | **0** | Node Toggle[cite: 16] |
| **4** | Wait[cite: 16] | **A-D** | Target Node Selection[cite: 16] |
| **5** | System Down[cite: 16] | **\*** | Alert[cite: 16] |
| **6** | Work Done[cite: 16] | **#** | Disconnect[cite: 16] |

## Getting Started

1. Open `Node_A_Transmitter.ino` and `Node_B_Receiver.ino` in the Arduino IDE.
2. Install required libraries: `Keypad`, `LiquidCrystal_I2C`, and `BluetoothSerial`.
3. Flash the transmitter code to the primary ESP32 and the receiver code to the secondary ESP32.
4. Supply 5V power to both nodes[cite: 16]. The LCD will display "Node Started" upon successful initialization.
5. Use keys A-D to select the target MAC address, then press a numeric key to transmit the encrypted payload.
