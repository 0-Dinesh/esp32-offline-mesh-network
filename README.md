# Offline Mesh Network using ESP-NOW (Legacy: Serial-Complete Phase)

**⚠️ Branch Notice:** This branch contains the *Serial-Complete* developmental phase of the project. In this iteration, physical hardware components (LCDs, keypads, buzzers) had not yet been integrated. All inputs, outputs, and system monitoring are handled entirely through the Arduino IDE Serial Monitor.

---

## Phase Objectives

Before introducing hardware constraints and potential physical wiring points of failure, this phase was strictly dedicated to validating the core software logic. The primary objectives of this iteration were:
1. Establishing a stable, two-way ESP-NOW communication link between two hardcoded ESP32 MAC addresses.
2. Implementing and validating a symmetrical XOR cryptographic cipher on the data payloads.
3. Integrating the `BluetoothSerial` library to establish a secondary gateway for mobile alerts without crashing the primary Wi-Fi radio.

---

## Methodology & Protocol Architecture

### 1. Simulated Hardware Interface
Because physical matrix keypads and I2C LCDs were omitted in this phase, the system uses the ESP32's built-in UART hardware serial port to simulate user interaction. 
- **Input:** The user types specific characters (`1`-`9`, `A`-`D`, `*`, `#`) directly into the Arduino IDE Serial Monitor.
- **Output:** Received messages, decrypted payloads, and system status logs are printed directly back to the Serial Monitor.

### 2. ESP-NOW Communication Protocol
The system utilizes the ESP-NOW protocol to achieve connectionless, router-independent mesh networking.
- Both ESP32 modules are explicitly set to Wi-Fi Station Mode (`WIFI_STA`).
- The native Wi-Fi connection is disconnected (`WiFi.disconnect()`) to ensure the 2.4 GHz radio is entirely dedicated to the ESP-NOW broadcast protocol.
- Peer nodes are registered using the `esp_now_peer_info_t` structure. MAC addresses for target nodes are hardcoded into the firmware (e.g., `uint8_t macA[] = {0x68, 0x25, 0xDD, 0x33, 0x2A, 0x08}`) to simulate a closed, secure network topology.

### 3. Lightweight XOR Cryptography
To prevent unauthorized packet sniffing on the 2.4 GHz spectrum, a custom encryption layer was introduced before passing data to the ESP-NOW transmission buffer.
- **Encryption:** The sender iterates through the plain-text string, applying a bitwise XOR operation using a static hexadecimal key (`0xAA`) against every character byte.
- **Decryption:** The receiving node intercepts the scrambled byte array and applies the exact same XOR operation (`msg[i] ^= XOR_KEY`) to seamlessly restore the original string for the Serial Monitor.

### 4. Bluetooth (BLE) Gateway Integration
To bridge the offline network with modern smart devices, the `BluetoothSerial` library was implemented.
- The ESP32 broadcasts a classic Bluetooth signal (e.g., `ESP32_SerialNodeA`).
- A paired smartphone can monitor the network.
- If a specific emergency trigger is caught by the ESP-NOW receiver (such as the `*` key triggering an "Alert" payload), the receiver routes a secondary warning string over the Bluetooth Serial connection to the paired mobile device.

---

## Testing & Execution Workflow

To run and test this specific legacy branch, you will need two ESP32 microcontrollers and two active USB connections.

### Setup Instructions
1. Flash `Node_A_Transmitter.ino` to the first ESP32.
2. Flash `Node_B_Receiver.ino` to the second ESP32.
3. Open two separate instances of the Arduino IDE (or use a secondary serial terminal like PuTTY) so you can view both Serial Monitors simultaneously.
4. Set both Serial Monitors to **115200 baud**.

### Operational Commands
Enter the following characters into the Node A Serial Monitor to transmit encrypted payloads to Node B:

* **Target Selection Keys:** `A`, `B`, `C`, `D` (Sets the destination MAC address).
* **Standard Message Keys:** 
  * `1` -> Hello
  * `2` -> Meet me
  * `3` -> File Ready
  * `4` -> Wait
  * `5` -> System Down
  * `6` -> Work Done
* **System Commands:**
  * `*` -> Transmits an "Alert" payload (This will also trigger the Bluetooth gateway to send an emergency ping to a paired smartphone).
  * `#` -> Disconnect

---

## Known Limitations of this Phase

- **Radio Coexistence Interference:** Actively maintaining a paired Bluetooth connection on the receiving node occasionally caused the ESP-NOW protocol to drop incoming Wi-Fi frames due to the ESP32 sharing a single 2.4 GHz radio antenna for both protocols. This was resolved in later physical branches by restricting BLE gateway operations strictly to the transmitting node.
- **Lack of Physical Portability:** The system relies entirely on a PC serial connection for input and output, defeating the purpose of an independent IoT node. This directly led to the integration of matrix keypads and LCDs in the `main` branch.
