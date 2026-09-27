/*
NODE: B
TYPE: SERIAL
SENSOR IMPLEMENTATION: YES (THROUGHT CODE)
BLUETOOTH IMPLEMENTATION: YES
ENCRYPTION: YES
*/


#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

// --- Bluetooth Setup ---
BluetoothSerial SerialBT_B;

// --- Node MAC Addresses ---
uint8_t macA_B[] = {0x68, 0x25, 0xDD, 0x33, 0x2A, 0x08}; 
uint8_t macB_B[] = {0x68, 0x25, 0xDD, 0x32, 0x5E, 0x24}; 
uint8_t macC_B[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
uint8_t macD_B[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

esp_now_peer_info_t peerInfo_B;
char targetNode_B = 'A';      
uint8_t* targetMAC_B = macA_B;

// --- XOR Encryption Key ---
const uint8_t XOR_KEY_B = 0xAA;

// --- Encrypt/Decrypt function ---
void encryptDecrypt_B(uint8_t *data, int len) {
  for (int i = 0; i < len; i++) {
    data[i] ^= XOR_KEY_B;
  }
}

// --- Key → Message mapping ---
String getMessageForKey_B(char key) {
  switch (key) {
    case '1': return "Hello";
    case '2': return "Meet me";
    case '3': return "File Ready";
    case '4': return "Wait";
    case '5': return "System Down";
    case '6': return "Work Done";
    case '7': return "Busy";
    case '8': return "Break";
    case '9': return "Ok";
    case '0': return "Node Toggle";
    case '*': return "Alert";
    case '#': return "Disconnect";
    default: return "";
  }
}

// --- ESP-NOW Callbacks ---
void OnDataSent_B(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fail");
}

void OnDataRecv_B(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  uint8_t buffer[250];
  memcpy(buffer, incomingData, len);
  encryptDecrypt_B(buffer, len); // decrypt
  buffer[len] = '\0';

  String msg = String((char*)buffer);
  Serial.print("From Node: ");
  for (int i=0; i<6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i<5) Serial.print(":");
  }
  Serial.print(" → ");
  Serial.println(msg);

  // Forward alerts via Bluetooth
  if (msg == "Alert") {
    SerialBT_B.println("[ALERT] Emergency from another node!");
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_send_cb(OnDataSent_B);
  esp_now_register_recv_cb(OnDataRecv_B);

  memcpy(peerInfo_B.peer_addr, macA_B, 6);   // Default target: Node A
  peerInfo_B.channel = 0;
  peerInfo_B.encrypt = false;
  esp_now_add_peer(&peerInfo_B);

  if (!SerialBT_B.begin("ESP32_SerialNodeB")) {
    Serial.println("BT init failed!");
  } else {
    Serial.println("Bluetooth ready. Pair with 'ESP32_SerialNodeB'");
  }

  Serial.println("=== Node B Started ===");
  Serial.println("Available Nodes: A, B, C, D");
  Serial.println("Use keys [A-D] to select target, [1-9,0,*,#] to send messages.");
}

// --- Loop ---
void loop() {
  if (Serial.available()) {
    char key = Serial.read();

    if (String("1234567890ABCD*#").indexOf(key) == -1) {
      Serial.println("Invalid Key");
      return;
    }

    // Select target node
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
      targetNode_B = key;
      if (key == 'A') targetMAC_B = macA_B;
      else if (key == 'B') targetMAC_B = macB_B;
      else if (key == 'C') targetMAC_B = macC_B;
      else if (key == 'D') targetMAC_B = macD_B;

      bool inactive = true;
      for (int i = 0; i < 6; i++) {
        if (targetMAC_B[i] != 0x00) { inactive = false; break; }
      }
      if (inactive) {
        Serial.print("Node ");
        Serial.print(key);
        Serial.println(" not active!");
      } else {
        Serial.print("Target: Node ");
        Serial.println(key);
      }
      return;
    }

    // Build message
    String msg = getMessageForKey_B(key);
    if (msg == "") return;

    bool inactive = true;
    for (int i = 0; i < 6; i++) {
      if (targetMAC_B[i] != 0x00) { inactive = false; break; }
    }
    if (inactive) {
      Serial.print("Error: Node ");
      Serial.print(targetNode_B);
      Serial.println(" not active!");
      return;
    }

    // Encrypt before send
    uint8_t buffer[250];
    int len = msg.length();
    memcpy(buffer, msg.c_str(), len);
    encryptDecrypt_B(buffer, len);

    esp_err_t result = esp_now_send(targetMAC_B, buffer, len);
    if (result == ESP_OK) {
      Serial.print("Sent to Node ");
      Serial.print(targetNode_B);
      Serial.print(": ");
      Serial.println(msg);

      // Emergency via Bluetooth
      if (msg == "Alert") {
        SerialBT_B.println("[ALERT] Emergency triggered locally!");
      }
    } else {
      Serial.println("Send Error");
    }
  }
}
