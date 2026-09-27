# Offline Mesh Network using ESP-NOW (Legacy: Serial-Only Phase)

**⚠️ Branch Notice:** This branch contains the absolute baseline *Serial-Only* developmental phase of the project. In this iteration, there is no physical hardware (keypads, LCDs), no Bluetooth gateway, and no encryption. It serves as the foundational skeleton for the ESP-NOW communication protocol.

---

## Phase Objectives

The primary goal of this phase was to strip away all complex variables to achieve a single objective: verifying that two ESP32 microcontrollers could successfully send and receive data payloads over a localized Wi-Fi mesh without a router.
1. Establish foundational ESP-NOW configurations (`WIFI_STA` mode).
2. Validate the hardcoded MAC address targeting system.
3. Confirm that asynchronous send/receive callbacks trigger correctly without blocking system execution.

---

## Methodology & Protocol Architecture

### 1. Pure ESP-NOW Communication
This phase isolates the ESP32's 2.4 GHz radio to establish a raw peer-to-peer connection.
- The ESP32 is initialized in Station Mode, immediately disconnecting from any saved Wi-Fi networks to dedicate the antenna to the ESP-NOW protocol.
- Target MAC addresses are hardcoded as hexadecimal arrays (e.g., `uint8_t macA[] = {0x68, 0x25, ...}`).
- The `esp_now_peer_info_t` struct is populated with the target MAC, defaulting to channel `0` with encryption explicitly set to `false`.

### 2. Serial Monitor Interface
All inputs and outputs are simulated via the Arduino IDE Serial Monitor to bypass physical wiring constraints.
- The system reads incoming keystrokes using `Serial.read()`.
- Valid alphanumeric inputs are mapped to predefined string payloads using a simple `switch` statement (e.g., pressing `1` queues the message "Hello").
- Received payloads are parsed from their byte arrays back into `char` arrays and printed directly to the Serial console.

---

## Testing & Execution Workflow

### Setup Instructions
1. Flash `Node_A_Transmitter.ino` and `Node_B_Receiver.ino` to two separate ESP32 modules.
2. Open two separate Serial Monitor instances set to **115200 baud**.
3. Ensure "No Line Ending" is selected in the Serial Monitor to prevent transmitting invisible carriage return characters.

### Operational Commands
Enter the following characters into the Serial Monitor to transmit payloads:
* **Target Selectors:** `A`, `B`, `C`, `D`
* **Message Triggers:** `1` through `9`, `0`, `*` (Alert), `#` (Disconnect)

---

## Known Limitations of this Phase
- **Zero Security:** Payloads are transmitted as plain text over standard 2.4 GHz frequencies, making them easily interceptable by any packet sniffer configured to read 802.11 action frames.
- **No Remote Connectivity:** Without the Bluetooth Serial module, the system has no way to alert users who are not physically staring at the Serial Monitor.
