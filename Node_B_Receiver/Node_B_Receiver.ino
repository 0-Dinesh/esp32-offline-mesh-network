/*
NODE: B
TYPE: SERIAL
SENSOR IMPLEMENTATION: NO
BLUETOOTH IMPLEMENTATION: NO
ENCRYPTION: NO
*/

#include <esp_now.h>
#include <WiFi.h>

// --- Node MAC Addresses ---
uint8_t macA[] = {0x68, 0x25, 0xDD, 0x33, 0x2A, 0x08}; 
uint8_t macB[] = {0x68, 0x25, 0xDD, 0x32, 0x5E, 0x24}; 
uint8_t macC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
uint8_t macD[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Store peer info
esp_now_peer_info_t peerInfo;

// Active target node
char targetNode = 'A'; // default send to A
uint8_t* targetMAC = macA;

// Key → Message mapping
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
  Serial.print("Sent to ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->des_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" -> Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  char msg[250];
  if (len < sizeof(msg)) {
    memcpy(msg, incomingData, len);
    msg[len] = '\0';
    Serial.print("From Node ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X -> ", 
      info->src_addr[0], info->src_addr[1], info->src_addr[2],
      info->src_addr[3], info->src_addr[4], info->src_addr[5]);
    Serial.println(msg);
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Add peer A by default
  memcpy(peerInfo.peer_addr, macA, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add Node A as peer");
  }

  Serial.println("=== Node B Started ===");
  Serial.println("Available Nodes: A, B, C, D");
  Serial.println("Use keys [A-D] to select target, [1-9,0,*,#] to send messages.");
}

// --- Loop ---
void loop() {
  if (Serial.available()) {
    char key = toupper(Serial.read());

    // Only accept valid keypad chars
    if (String("1234567890ABCD*#").indexOf(key) == -1) {
      Serial.println("Invalid key. Use keypad keys only (1-9,0,A-D,*,#).");
      return;
    }

    // If node selection key pressed
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
      targetNode = key;
      if (key == 'A') targetMAC = macA;
      else if (key == 'B') targetMAC = macB;
      else if (key == 'C') targetMAC = macC;
      else if (key == 'D') targetMAC = macD;

      // Check if target is valid
      bool inactive = true;
      for (int i = 0; i < 6; i++) {
        if (targetMAC[i] != 0x00) { inactive = false; break; }
      }
      if (inactive) {
        Serial.print("Node "); Serial.print(key); Serial.println(" not available!");
      } else {
        Serial.print("Target node set to "); Serial.println(key);
      }
      return;
    }

    // Handle message keys
    String msg = getMessageForKey(key);
    if (msg == "") return;

    // Check if node is active
    bool inactive = true;
    for (int i = 0; i < 6; i++) {
      if (targetMAC[i] != 0x00) { inactive = false; break; }
    }
    if (inactive) {
      Serial.print("Error: Node "); Serial.print(targetNode); Serial.println(" is not active!");
      return;
    }

    // Send message
    esp_err_t result = esp_now_send(targetMAC, (uint8_t*)msg.c_str(), msg.length());
    if (result == ESP_OK) {
      Serial.print("Sent to Node "); Serial.print(targetNode); Serial.print(": ");
      Serial.println(msg);
    } else {
      Serial.println("Error sending message!");
    }
  }
}
