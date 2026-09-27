# Offline Mesh Network using ESP-NOW (Experimental: Hybrid-Keyboard Phase)

**⚠️ Branch Notice:** This branch contains an *Experimental Hybrid* developmental phase. In this iteration, the physical LCD and Buzzer are active, alongside XOR Encryption and the Bluetooth Gateway. However, the physical **Keypad is omitted**, reverting strictly to PC Keyboard (Serial) input. 

---

## Phase Objectives

This branch was utilized as an isolation environment for debugging. When integrating multiple hardware peripherals alongside complex software (Cryptography + BLE), troubleshooting points of failure becomes difficult. The objectives of this hybrid phase were:
1. Temporarily bypass the physical keypad matrix to isolate potential wiring or pin-conflict issues.
2. Validate that the I2C LCD and Buzzer could successfully process encrypted incoming ESP-NOW packets while the Bluetooth radio was active.
3. Test the XOR cryptographic payload delivery using highly controlled Serial inputs.

---

## Methodology & Architecture

### 1. Hybrid I/O Implementation
To facilitate debugging, input and output methods were split between software and hardware paradigms.
- **Input (Software):** The `Keypad.h` library is entirely removed. The transmitter node listens to the Arduino IDE Serial Monitor (`Serial.read()`) to capture user commands.
- **Output (Hardware):** The receiver node utilizes the physical 16x02 I2C LCD (`0x27`) and the piezoelectric buzzer (GPIO `4`) to render decrypted payloads and sound alerts.

### 2. Cryptography & BLE Re-Integration
With the keypad removed, the software overhead was increased to its maximum intended capacity to test stability.
- **Encryption:** The `encryptDecrypt()` function applies the `0xAA` XOR cipher to the buffer before passing it to the Wi-Fi antenna.
- **Bluetooth:** The `BluetoothSerial` instance (`ESP32_AlertNode`) actively listens for connections. If a decrypted payload matches the string "Alert", a secondary emergency string is blasted over the BLE gateway to connected smartphones.

---

## Testing & Execution Workflow

### Setup Instructions
1. Wire the receiver ESP32 to the LCD and Buzzer. **Do not connect the 4x4 keypad to the transmitter.**
2. Flash the transmitter and receiver `.ino` files.
3. Keep the transmitter ESP32 connected to the PC via USB and open the Serial Monitor (115200 baud).
4. Pair a mobile device to the transmitter's Bluetooth broadcast (`ESP32_AlertNode`).

### Operational Commands
- Type characters (`1`-`9`, `A`-`D`, `*`, `#`) into the PC Serial Monitor.
- Watch the physical LCD on the receiver update dynamically and listen for the 1kHz buzzer tone.
- Type `*` into the Serial Monitor to trigger the XOR-encrypted alert, and verify the mobile phone receives the Bluetooth push notification.

---

## Known Limitations of this Phase
- **Not a Standalone IoT Device:** Because the system requires a PC Serial connection to generate inputs, the transmitter node is tethered and not portable. This was strictly a diagnostic testing branch before merging the keypad back into the final `main` branch.
