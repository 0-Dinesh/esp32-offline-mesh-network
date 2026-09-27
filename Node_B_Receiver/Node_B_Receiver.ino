/*
NODE: B
TYPE: PHYSICAL
SENSOR IMPLEMENTATION: YES (THROUGHT CODE)
BLUETOOTH IMPLEMENTATION: YES
ENCRYPTION: YES
*/

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"
#include <Keypad.h>

// --- LCD + Buzzer Setup ---
LiquidCrystal_I2C lcd_B(0x27, 16, 2); 
int buzzerPin_B = 4;

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

// --- Keypad Setup ---
const byte ROWS_B = 4; 
const byte COLS_B = 4; 
char keys_B[ROWS_B][COLS_B] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins_B[ROWS_B] = {19, 18, 32, 33}; 
byte colPins_B[COLS_B] = {25, 26, 27, 14}; 
Keypad keypad_B = Keypad(makeKeymap(keys_B), rowPins_B, colPins_B, ROWS_B, COLS_B);

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

// --- Buzzer beep ---
void beep_B() {
  ledcAttach(buzzerPin_B, 2000, 8);  
  ledcWriteTone(buzzerPin_B, 1000);  
  delay(150);
  ledcWriteTone(buzzerPin_B, 0);     
}

// --- ESP-NOW Callbacks ---
void OnDataSent_B(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  lcd_B.clear();
  lcd_B.setCursor(0,0);
  lcd_B.print("Send Status:");
  lcd_B.setCursor(0,1);
  lcd_B.print(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fail");
}

void OnDataRecv_B(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  uint8_t buffer[250];
  memcpy(buffer, incomingData, len);
  encryptDecrypt_B(buffer, len);
  buffer[len] = '\0';

  String msg = String((char*)buffer);

  lcd_B.clear();
  lcd_B.setCursor(0,0);
  lcd_B.print("From Node");
  lcd_B.setCursor(0,1);
  lcd_B.print(msg);
  beep_B();

  if (msg == "Alert") {
    SerialBT_B.println("[ALERT] Emergency from another node!");
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  lcd_B.init();
  lcd_B.backlight();
  lcd_B.clear();
  lcd_B.setCursor(0,0);
  lcd_B.print("Node B Started");
  delay(1000);

  // ESP-NOW init
  if (esp_now_init() != ESP_OK) {
    lcd_B.clear();
    lcd_B.print("ESP-NOW Error");
    return;
  }
  esp_now_register_send_cb(OnDataSent_B);
  esp_now_register_recv_cb(OnDataRecv_B);

  memcpy(peerInfo_B.peer_addr, macA_B, 6);  // Default target is Node A
  peerInfo_B.channel = 0;
  peerInfo_B.encrypt = false;
  esp_now_add_peer(&peerInfo_B);

  if (!SerialBT_B.begin("ESP32_AlertNodeB")) {
    Serial.println("BT init failed!");
    lcd_B.clear();
    lcd_B.print("BT Error");
  } else {
    Serial.println("Bluetooth ready. Pair with 'ESP32_AlertNodeB'");
    lcd_B.setCursor(0,1);
    lcd_B.print("BT: Ready");
  }

  Serial.println("=== Node B Started ===");
  Serial.println("Available Nodes: A, B, C, D");
}

// --- Loop ---
void loop() {
  char key = keypad_B.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    if (String("1234567890ABCD*#").indexOf(key) == -1) {
      lcd_B.clear();
      lcd_B.print("Invalid Key");
      return;
    }

    // Handle node selection
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
      lcd_B.clear();
      if (inactive) {
        lcd_B.print("Node ");
        lcd_B.print(key);
        lcd_B.setCursor(0,1);
        lcd_B.print("not active!");
      } else {
        lcd_B.print("Target: Node ");
        lcd_B.print(key);
      }
      return;
    }

    // Otherwise → send message
    String msg = getMessageForKey_B(key);
    if (msg == "") return;

    bool inactive = true;
    for (int i = 0; i < 6; i++) {
      if (targetMAC_B[i] != 0x00) { inactive = false; break; }
    }
    lcd_B.clear();
    if (inactive) {
      lcd_B.print("Error: Node ");
      lcd_B.print(targetNode_B);
      lcd_B.setCursor(0,1);
      lcd_B.print("not active!");
      return;
    }

    uint8_t buffer[250];
    int len = msg.length();
    memcpy(buffer, msg.c_str(), len);
    encryptDecrypt_B(buffer, len);

    esp_err_t result = esp_now_send(targetMAC_B, buffer, len);
    if (result == ESP_OK) {
      lcd_B.print("Sent to Node ");
      lcd_B.print(targetNode_B);
      lcd_B.setCursor(0,1);
      lcd_B.print(msg);
      beep_B();

      if (msg == "Alert") {
        SerialBT_B.println("[ALERT] Emergency triggered locally!");
      }
    } else {
      lcd_B.print("Send Error");
    }
  }
}
