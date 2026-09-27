/*
NODE: A
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
LiquidCrystal_I2C lcd(0x27, 16, 2); 
int buzzerPin = 4;

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

// --- Keypad Setup ---
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 18, 32, 33}; // Adjust to your wiring
byte colPins[COLS] = {25, 26, 27, 14}; // Adjust to your wiring
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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

// --- Buzzer beep (one-time) ---
void beep() {
  ledcAttach(buzzerPin, 2000, 8);  
  ledcWriteTone(buzzerPin, 1000);  
  delay(150);
  ledcWriteTone(buzzerPin, 0);     
}

// --- ESP-NOW Callbacks ---
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Send Status:");
  lcd.setCursor(0,1);
  lcd.print(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fail");
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  uint8_t buffer[250];
  memcpy(buffer, incomingData, len);
  encryptDecrypt(buffer, len);
  buffer[len] = '\0';

  String msg = String((char*)buffer);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("From Node");
  lcd.setCursor(0,1);
  lcd.print(msg);
  beep();

  if (msg == "Alert") {
    SerialBT.println("[ALERT] Emergency from another node!");
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Node Started");
  delay(1000);

  // ESP-NOW init
  if (esp_now_init() != ESP_OK) {
    lcd.clear();
    lcd.print("ESP-NOW Error");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, macB, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  if (!SerialBT.begin("ESP32_AlertNode")) {
    Serial.println("BT init failed!");
    lcd.clear();
    lcd.print("BT Error");
  } else {
    Serial.println("Bluetooth ready. Pair with 'ESP32_AlertNode'");
    lcd.setCursor(0,1);
    lcd.print("BT: Ready");
  }

  Serial.println("=== Node Started ===");
  Serial.println("Available Nodes: A, B, C, D");
}

// --- Loop ---
void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    if (String("1234567890ABCD*#").indexOf(key) == -1) {
      lcd.clear();
      lcd.print("Invalid Key");
      return;
    }

    // Handle node selection
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
      lcd.clear();
      if (inactive) {
        lcd.print("Node ");
        lcd.print(key);
        lcd.setCursor(0,1);
        lcd.print("not active!");
      } else {
        lcd.print("Target: Node ");
        lcd.print(key);
      }
      return;
    }

    // Otherwise → send message
    String msg = getMessageForKey(key);
    if (msg == "") return;

    bool inactive = true;
    for (int i = 0; i < 6; i++) {
      if (targetMAC[i] != 0x00) { inactive = false; break; }
    }
    lcd.clear();
    if (inactive) {
      lcd.print("Error: Node ");
      lcd.print(targetNode);
      lcd.setCursor(0,1);
      lcd.print("not active!");
      return;
    }

    uint8_t buffer[250];
    int len = msg.length();
    memcpy(buffer, msg.c_str(), len);
    encryptDecrypt(buffer, len);

    esp_err_t result = esp_now_send(targetMAC, buffer, len);
    if (result == ESP_OK) {
      lcd.print("Sent to Node ");
      lcd.print(targetNode);
      lcd.setCursor(0,1);
      lcd.print(msg);
      beep();

      if (msg == "Alert") {
        SerialBT.println("[ALERT] Emergency triggered locally!");
      }
    } else {
      lcd.print("Send Error");
    }
  }
}
