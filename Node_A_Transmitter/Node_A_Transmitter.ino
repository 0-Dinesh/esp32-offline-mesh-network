#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

// --- Bluetooth Setup ---
BluetoothSerial SerialBT;

// --- Node MAC Addresses ---
uint8_t macA[] = {0x68, 0x25, 0xDD, 0x33, 0x2A, 0x08}; 
uint8_t macB[] = {0x68, 0x25, 0xDD, 0x32, 0x5E, 0x24}; 
uint8_t macC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
uint8_t macD[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

esp_now_peer_info_t peerInfo;
char targetNode = 'B';      
uint8_t* targetMAC = macB;

// --- XOR Encryption Key ---
const uint8_t XOR_KEY = 0xAA;

// --- Encrypt/Decrypt function ---
void encryptDecrypt(uint8_t *data, int len) {
  for (int i = 0; i < len; i++) {
    data[i] ^= XOR_KEY;
  }
}

// --- Key → Message mapping ---
String getMessageForKey(char key) {
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
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fail");
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  uint8_t buffer[250];
  memcpy(buffer, incomingData, len);
  encryptDecrypt(buffer, len); // decrypt
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
    SerialBT.println("[ALERT] Emergency from another node!");
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

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, macB, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  if (!SerialBT.begin("ESP32_SerialNode")) {
    Serial.println("BT init failed!");
  } else {
    Serial.println("Bluetooth ready. Pair with 'ESP32_SerialNode'");
  }

  Serial.println("=== Node Started ===");
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
      targetNode = key;
      if (key == 'A') targetMAC = macA;
      else if (key == 'B') targetMAC = macB;
      else if (key == 'C') targetMAC = macC;
      else if (key == 'D') targetMAC = macD;

      bool inactive = true;
      for (int i = 0; i < 6; i++) {
        if (targetMAC[i] != 0x00) { inactive = false; break; }
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
    String msg = getMessageForKey(key);
    if (msg == "") return;

    bool inactive = true;
    for (int i = 0; i < 6; i++) {
      if (targetMAC[i] != 0x00) { inactive = false; break; }
    }
    if (inactive) {
      Serial.print("Error: Node ");
      Serial.print(targetNode);
      Serial.println(" not active!");
      return;
    }

    // Encrypt before send
    uint8_t buffer[250];
    int len = msg.length();
    memcpy(buffer, msg.c_str(), len);
    encryptDecrypt(buffer, len);

    esp_err_t result = esp_now_send(targetMAC, buffer, len);
    if (result == ESP_OK) {
      Serial.print("Sent to Node ");
      Serial.print(targetNode);
      Serial.print(": ");
      Serial.println(msg);

      // Emergency via Bluetooth
      if (msg == "Alert") {
        SerialBT.println("[ALERT] Emergency triggered locally!");
      }
    } else {
      Serial.println("Send Error");
    }
  }
}
