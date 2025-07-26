//KHỐI GỬI DỮ LIỆU
//ESP32 Board MAC Address: cc:db:a7:92:aa:2c
#include <Arduino.h>
#include <WiFi.h>
#include <TM1628.h>
#include "HX711.h"
#include <Preferences.h>
#include <FirebaseESP32.h>
#include <WiFiClientSecure.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <time.h>
#define SERVICE_UUID        "****"   // UUID for the service
#define CHARACTERISTIC_UUID "****"  // UUID for the characteristic   
#define DT 26     //HX711 Pin
#define SCK 27   //HX711 Pin
#define BUTTON_PIN 15   // Button pin
#define BUTTON_PIN2 17  // Button pin 2
#define BUTTON_PIN3 16  // Button pin 3
#define PIN_DIO 21  // TM1628 DIO pin
#define PIN_CLK 22  // TM1628 CLK pin
#define PIN_STB 23  // TM1628 STB pin
#define LED_DATA_LENGTH 12 // 6 digits * 2 bytes per digit

TM1628 dvdLED(21, 22, 23); 
Preferences prefs;
HX711 scale;
hw_timer_t *timer; 
hw_timer_t *timer2; 
BLEService* pService = nullptr;
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
unsigned long lastBLEUpdate = 0;
unsigned long lastESPNOWSend = 0;
bool deviceConnected = false;  
bool oldDeviceConnected = false;
bool firebaseInitialized = false;
bool lastWifiConnected = false;
bool bleStarted = false;
char buffer_weight[6];
volatile bool timer_flag = false;  
volatile bool timer_flag2 = false;  
bool btn1 = true;
bool btn2 = true;
bool btn3 = true;
float weight;
float calibration_factor; // giá trị này bạn lấy từ code hiệu chỉnh
float raw;
float raw1;
volatile bool button1State = false;  
volatile bool button2State = false;
volatile bool button3State = false;
uint16_t physicalPos;
uint8_t dotPos;
uint16_t segment;
int digit;
float max_kg = 30.0;
float standard_kg; // 1kg = 1000g
float noiseThreshold = 0.001;  // Ngưỡng nhiễu để xác định khi nào giá trị ổn định: 10g
float noiseThreshold2 = 0.01; // Ngưỡng nhiễu để xác định khi nào giá trị ổn định quanh điểm 0: 100g
float prev_weight = 0;
float roundedWeight;
const int buttonDebounce = 200;
unsigned long pretime2Press = 0;
unsigned long wifiStartTime ;
const unsigned long wifiTimeout  = 15000;   // Thời gian chờ kết nối WiFi (10 giây)
String receivedCommand = ""; // Lưu lệnh nhận từ BLE
//////////////////////////////////////////////////////////////////////////////////////////////////////
const uint16_t digitMap[11] = {
  //TM1629A
  // 0x35A, // 0
  // 0x042, // 1
  // 0x958, // 2
  // 0x94A, // 3
  // 0xA42, // 4
  // 0xB0A, // 5
  // 0xB1A, // 6
  // 0x142, // 7
  // 0xB5A, // 8
  // 0xB4A  // 9
  //TM1624
  0xEB , // 0
  0x88 , // 1
  0xB3 , // 2
  0xBA, // 3
  0xD8 , // 4
  0x7A , // 5
  0x7B , // 6
  0xA8 , // 7
  0xFB , // 8
  0xFA ,  // 9
  0x04 //dp
};
const uint16_t digitMap_char[7] = {
  //TM1629A
  // 0x35A, // 0
  // 0x042, // 1
  // 0x958, // 2
  // 0x94A, // 3
  // 0xA42, // 4
  // 0xB0A, // 5
  // 0xB1A, // 6
  // 0x142, // 7
  // 0xB5A, // 8
  // 0xB4A  // 9
  //TM1624
  0xF900, //a
  0x6310,//c
  0x4300,//L
  0xEB00,//O
  0x9B00,//D
  0x7310,//E
  0x1900,//N
};
// const uint16_t digitMap1624[11] = {
//   0xEB , // 0
//   0x88 , // 1
//   0xB3 , // 2
//   0xBA, // 3
//   0xD8 , // 4
//   0x7A , // 5
//   0x7B , // 6
//   0xA8 , // 7
//   0xFB , // 8
//   0xFA ,  // 9
// };
const uint16_t digitOrder[6] = {
  //TM1629A
  // 3 ,  // digit 1 → ledGRID[0]
  // 5,  // digit 2 → ledGRID[1]
  // 4,  // digit 3 → ledGRID[2]
  // 2,  // digit 6 → ledGRID[5]
  // 1,  // digit 5 → ledGRID[4]
  // 0   // digit 4 → ledGRID[3]
  //TM1624
  5,4,3,2,1,0
};
//////////////////////////////////////////////////////////////////////////////////////////////////////
static uint16_t ledGRID[LED_DATA_LENGTH / 2]=  {// текущее состояние экрана
	0x0000 ,0x0000,0x0000,0x0000,0x0000,0x0000
};
static uint8_t currentBrightness = 7;
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = {0xc8, 0x2e, 0x18, 0x25, 0xe4, 0x7c};
//////////////////////////////////////////////////////////////////////////////////////////////////////
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData firebaseData;
FirebaseData fbdo;
FirebaseData fbdo2;
FirebaseData fbdo3;
FirebaseData fbdo4;
FirebaseData fbdo5;
FirebaseData fbdo6;
String paths  = "/NUTNHAN5";
String paths2  = "/NUTNHAN1";
String paths3  = "/NUTNHAN3";
String paths4  = "/KHOILUONG";
String paths5  = "/CALIBCELL";
String paths6  = "/MAXCELL";
const char ssid[] = ""****"";
const char pass[] = ""****"";
// const char ssid[] = "Xiaomi 14T";
// const char pass[] = "qz4uhgasvw32fjh";
// const char ssid[] = "Ca Phe Vong";
// const char pass[] = "";
// const char ssid[] = "Iphone 6s";
// const char pass[] = "12345678";
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    if (pServer != nullptr) {
      deviceConnected = true;
    } 
  };
  void onDisconnect(BLEServer* pServer) {
    if (pServer != nullptr) {
      deviceConnected = false;
      pServer->startAdvertising();
    }
  }
};
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (pCharacteristic != nullptr) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0 ) {
        receivedCommand = String(value.c_str());
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++) {
          Serial.print(value[i]);
        }
        Serial.println(receivedCommand);
        Serial.println("*********");
      }
    }
  }
};
void startBLE() {
  BLEDevice::init("ESP32-BLE-Server");
  // BLEServer *pServer = BLEDevice::createServer();
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // BLEService *pService = pServer->createService(SERVICE_UUID);
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE|
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new MyCallbacks());
  BLE2902* pBLE2902 = new BLE2902();
  pCharacteristic->addDescriptor(pBLE2902);
  pCharacteristic->setValue("BLE started");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("BLE đang hoạt động...");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void writeSingleCommand(const uint8_t command) {
  digitalWrite(PIN_STB, LOW);
  shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, command);
  digitalWrite(PIN_STB, HIGH);
  delayMicroseconds(1); 
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void updateDisplay(void) {
  writeSingleCommand(0x40);  
  digitalWrite(PIN_STB, LOW);
  shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, 0xC0);
  uint8_t * p = (uint8_t *) ledGRID;
  for (int8_t i = 0; i < LED_DATA_LENGTH; i++, p++) shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, *p); 
  digitalWrite(PIN_STB, HIGH);
  delayMicroseconds(1); 

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
static inline uint8_t setBrightness(const uint8_t newBrighness) {

    const uint8_t res = currentBrightness;

    currentBrightness = (newBrighness > 8) ? 8 : newBrighness;

    if (currentBrightness == 0) writeSingleCommand(0x80); 

    else writeSingleCommand(0x88 + (currentBrightness - 1)); 

    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void setDigit(const int8_t digit, const uint16_t value) {
  if (digit < 0 || digit > 6) return; 
  ledGRID[digit] = value; 
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void setDigitMapped(uint16_t logicalPos, uint16_t value) {
  if (logicalPos > 6) return;
  physicalPos = digitOrder[logicalPos];
  ledGRID[physicalPos] = value;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void displayNumber(int number) {
  for (int i = 0; i < 6; i++) {
    if (number > 0) {
      digit = number % 10;
      setDigitMapped(i, digitMap[digit]);
      number /= 10;
    } else {
      setDigitMapped(i, 0x0000);  // tắt các digit không dùng
    }
  }
  updateDisplay();  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void clearDisplay() {
  for (int i = 0; i < 6; i++) {
    setDigitMapped(i, 0x0000);  // Tắt tất cả segment
  }
  updateDisplay();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void displayFloat(float value, uint8_t decimalPlaces, uint8_t dp) {
  if (value >= 999999 || value <= -99999) return;
  int highestDigitUsed = -1;
  bool isNegative = false;
  if (value < 0) {
  isNegative = true;
  value = -value;
  }
  // Nhân lên để thành số nguyên
  int intValue = (int)(value * pow(10, decimalPlaces));
  // Cờ cho biết vị trí dấu chấm, từ phải sang trái (digit 0 là bên phải)
  dotPos = decimalPlaces;
  for (int i = 0; i < 6; i++) {
    if (intValue != 0 || i <= dotPos) {
      int digit = intValue % 10;
      segment = digitMap[digit];
      // Nếu vị trí hiện tại là vị trí của dấu chấm → thêm bit dp
      if (i == dotPos) segment |= dp;  //DP: 0x01(TM1629A); 0x04(TM1624);
      setDigitMapped(i, segment);
      intValue /= 10;
      highestDigitUsed = i;
    } else {
      setDigitMapped(i, 0x0000); // clear
    }
  }
  if (isNegative) {
    if (highestDigitUsed < 5) {
        setDigitMapped(highestDigitUsed + 1, 0x10);  // SEG_G
    } else {
        setDigitMapped(5, 0x10);  // fallback nếu đầy màn hình
    }
  }
  updateDisplay();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
///////SENDFIREBASE
void update1Firebase(float X){
  Firebase.setFloat(firebaseData, paths4, roundedWeight);//weight
}
void update2Firebase(){
  Firebase.setBool(firebaseData, paths, btn1);
}
void update3Firebase(){
  // Firebase.setBool(firebaseData, paths2, btn2);
if (Firebase.setBool(firebaseData, paths2, btn2)) {
    Serial.println("Đã gửi  lên Firebase");
  } else {
    Serial.println("Lỗi khi gửi lên Firebase: " + firebaseData.errorReason());
  }
}
void update4Firebase(){
  // Firebase.setBool(firebaseData, paths3, btn3);
if (Firebase.setBool(firebaseData, paths3, btn3)) {
    Serial.println("Đã gửi  lên Firebase");
  } else {
    Serial.println("Lỗi khi gửi lên Firebase: " + firebaseData.errorReason());
  }
}
///////RECEIVEFIREBASE
void printStreamData(StreamData data) {
  int value =0;;
  // String stringValue;
  boolean boolValue = false;
  if (data.dataType() == "int") {
    value  = data.intData();
  // } else if (data.dataType() == "string") {
  //   stringValue  = data.stringData();
  } else if (data.dataType() == "boolean") {
    boolValue = data.boolData();
  }
  else {
    // Serial.print("Other data type");
  }
  //////////////
  if (data.streamPath() == paths) {
    btn1 = boolValue; 
    if (button1State) {
      Serial.println(button1State);   
    } else {
      Serial.println(".");
    }
  } else if (data.streamPath() == paths2) { 
    btn2 = boolValue; 
      if (button2State) {
      Serial.println(button2State);   
    } else {
      Serial.println(".");
    }
  } else if (data.streamPath() == paths3) { 
    btn3 = boolValue; 
      if (button3State) {
      Serial.println(button3State);   
    } else {
      Serial.println(".");
    }
  } else if (data.streamPath() == paths5) { //CALIBRATE
      standard_kg = value;  
      prefs.begin("hx711", false); 
      prefs.putFloat("standard_kg", standard_kg);  // lưu vào flash
      prefs.end();
      Serial.print("Đã LƯU GIÁ TRỊ CALIBRATE VÀO FLASH: ");
      Serial.println(standard_kg);
  } else if (data.streamPath() == paths6) { //MAXCELL
    max_kg = value;
  }
}
void streamCallback1(StreamData data) {
  printStreamData(data);
}
void streamCallback2(StreamData data) {
  printStreamData(data);
}
void streamCallback3(StreamData data) {
  printStreamData(data);
}
void streamCallback5(StreamData data) {
  printStreamData(data);
}
void streamCallback6(StreamData data) {
  printStreamData(data);
}
void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("Stream timeout, reconnecting...");
        Firebase.beginStream(fbdo, paths);
        Firebase.beginStream(fbdo2, paths2);
        Firebase.beginStream(fbdo3, paths3); 
        Firebase.beginStream(fbdo5, paths5);
        Firebase.beginStream(fbdo6, paths6);
    }
}
void IRAM_ATTR button1Push(){
  button1State = true;
}
void IRAM_ATTR button2Push(){
  button2State = true;
}
void IRAM_ATTR button3Push(){
  button3State = true;
}
void IRAM_ATTR onTimer() {
  timer_flag = true;  
}
void IRAM_ATTR onTimer2() {
  timer_flag2 = true;  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup(void) {
//////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(9600);
  //Khởi tạo TM1628;  hx711; Preferences;
  dvdLED.begin(ON, 7);
  scale.begin(DT, SCK);
  
  pinMode(PIN_CLK, OUTPUT);  // TM1628 CLK pin
  pinMode(PIN_DIO, OUTPUT);  // TM1628 DIO pin
  pinMode(PIN_STB, OUTPUT);  // TM1628 STB pin
  digitalWrite(PIN_STB, HIGH); // TM1628 STB pin
  digitalWrite(PIN_CLK, HIGH);  // TM1628 CLK pin
  config.host = ""****"";
    config.api_key = ""****"";
    auth.user.email = "candientu@gmail.com";
    auth.user.password = "namdt@";
  WiFi.begin(ssid, pass);
  // WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < wifiTimeout)
  { 
      for (uint8_t d = 0; d < 6; d++) {
      ledGRID[d] = digitMap[10];
      updateDisplay();
      delay(200);
      } 
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // configTime(7 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
  // config.timeout.serverResponse = 10 * 1000;
    Firebase.begin(&config, &auth);
    Serial.println("WiFi connected!");
    Firebase.reconnectWiFi(true);
    Firebase.beginStream(fbdo, paths);
    Firebase.beginStream(fbdo2, paths2);
    Firebase.beginStream(fbdo3, paths3);
    Firebase.beginStream(fbdo5, paths5);
    Firebase.beginStream(fbdo6, paths6);
    Firebase.setStreamCallback(fbdo, streamCallback1, streamTimeoutCallback);
    Firebase.setStreamCallback(fbdo2, streamCallback2, streamTimeoutCallback);
    Firebase.setStreamCallback(fbdo3, streamCallback3, streamTimeoutCallback);
    Firebase.setStreamCallback(fbdo5, streamCallback5, streamTimeoutCallback);
    Firebase.setStreamCallback(fbdo6, streamCallback6, streamTimeoutCallback);
    firebaseInitialized = true;
    lastWifiConnected = true;
  } else {
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    Serial.println("WiFi failed. Switching to BLE...");
    startBLE();
    bleStarted = true;
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button1Push, FALLING);
  // pinMode(BUTTON_PIN2, INPUT_PULLUP); 
  // attachInterrupt(digitalPinToInterrupt(BUTTON_PIN2), button2Push, FALLING);
  pinMode(BUTTON_PIN3, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN3), button3Push, FALLING);
//////////////////////////////////////////////////////////////////////////////////////////////////////
  timer = timerBegin(0, 80, true); 
  timerAttachInterrupt(timer, &onTimer, true); 
  timerAlarmWrite(timer, 400000, true); 
  timerAlarmEnable(timer);
  timer2 = timerBegin(1, 80, true); 
  timerAttachInterrupt(timer2, &onTimer2, true); 
  timerAlarmWrite(timer2, 100000, true); 
  timerAlarmEnable(timer2);
//////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println("Khởi động HX711...");
  while (!scale.is_ready()) {
    Serial.println("HX711 chưa sẵn sàng...");
    delay(500);
  }
  Serial.println("HX711 đã sẵn sàng");
//////////////////////////////////////////////////////////////////////////////////////////////////////
prefs.begin("hx711", false);
  if(prefs.isKey("standard_kg")){
    standard_kg = prefs.getFloat("standard_kg", 1.0);
    Serial.print("Đọc hệ số standard_kg từ flash: ");
    Serial.println(standard_kg);
    
  }
prefs.end();
//////////////////////////////////////////////////////////////////////////////////////////////////////
prefs.begin("hx711", false);
  if (prefs.isKey("cal")) {
  // Có giá trị đã lưu => đọc lại
  calibration_factor = prefs.getFloat("cal", 1.0);
  // calibration_factor=1.05;
  Serial.print("Đọc hệ số calibration từ flash: ");
  Serial.println(calibration_factor);
  scale.set_scale(calibration_factor);
  scale.tare();
  } else {
    Serial.print("không có giá trị có sẵn, chuẩn bị hiệu chỉnh giá trị mới trong hàm setup");
    raw1 = scale.read();  // giá trị ADC gốc
    Serial.print("Giá trị ADC trước khi đặt vật nặng: ");
    Serial.println(raw1);
  
    delay(5000);
    Serial.print("Place a known weight on the scale...");
    delay(5000);
    
    raw = scale.read();  // giá trị sau khi đặt vật nặng
    Serial.print("Giá trị ADC sau khi đặt vật nặng: ");
    Serial.println(raw);
  
    delay(5000);
    calibration_factor  =(raw - raw1) / standard_kg;
    scale.set_scale(calibration_factor);
    scale.tare();
    prefs.putFloat("cal", calibration_factor);  // lưu vào flash
    Serial.println("Đã lưu hệ số vào flash.");
    Serial.println(calibration_factor);
  }
  prefs.end();
  setBrightness(currentBrightness);
  clearDisplay();
  ledGRID[5] = 0xEB;//SỐ 0
  updateDisplay();
  Serial.println(max_kg);
  Serial.println(standard_kg);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void loop(void) {
////////////////////////////////////////////////////////////////////////////////////////////////////// 
  wifiConnected();
////////////////////////////////////////////////////////////////////////////////////////////////////// 
  if(lastWifiConnected && !wifiConnected()){
    Serial.println("WiFi đã mất kết nối!");
    firebaseInitialized = false;
    lastWifiConnected= false;
    firebaseData.clear();
    fbdo.clear();
    fbdo2.clear();
    fbdo3.clear();
    fbdo4.clear();
    fbdo5.clear();
    fbdo6.clear();
    WiFi.disconnect(true); 
    WiFi.mode(WIFI_OFF);
    if(!bleStarted){
      startBLE();
      bleStarted = true;
    }
  }
  else if(!lastWifiConnected && wifiConnected()){
    bleStarted = false;
    Serial.println("WiFi đã có kết nối trở lại!");
  }

  // Xử lý lệnh CAL và MAX từ BLE
  if (receivedCommand.startsWith("CAL") && receivedCommand.indexOf("MAX") > 0) {
    String calPart = receivedCommand.substring(3, receivedCommand.indexOf("MAX"));
    String maxPart = receivedCommand.substring(receivedCommand.indexOf("MAX") + 3);
    standard_kg = calPart.toFloat();
    max_kg = maxPart.toFloat();
    Serial.println("Received CAL: " + String(standard_kg) + ", MAX: " + String(max_kg));
    // Thêm logic sử dụng calibrationWeight và maxWeight (ví dụ: cập nhật cấu hình cân)
    receivedCommand = "";
    prefs.begin("hx711", false); 
    prefs.putFloat("standard_kg", standard_kg);  // lưu vào flash
    prefs.end();
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////
  if (button1State) {  
    if((millis() - pretime2Press) > buttonDebounce){
        pretime2Press = millis();
        btn1 =false;
        update2Firebase();
    }
    button1State = false;
  }
  if(receivedCommand == "CALIBRATE"){
    if((millis() - pretime2Press) > buttonDebounce){
        pretime2Press = millis();
        btn1 =false;
        receivedCommand = "";
        Serial.println("Received CALIBRATE command via BLE");
    }
  }
  if(!btn1){
    Serial.println(">>> ĐANG Ở CHẾ ĐỘ HIỆU CHỈNH ở vòng loop <<<");
    clearDisplay();
    ledGRID[0] = 0x63  ; //CAL
    ledGRID[1] = 0xF9  ;
    ledGRID[2] = 0x43  ;
    updateDisplay();
    delay(1000);
    raw1 = scale.read_average(5);   // giá trị ADC gốc
    Serial.print("Giá trị ADC trước khi đặt vật nặng: ");
    Serial.println(raw1);
    delay(3000);
    clearDisplay();
    ledGRID[0] = 0xF9  ; //ADLOAD
    ledGRID[1] = 0x9B  ;
    ledGRID[2] = 0x43  ;
    ledGRID[3] = 0xEB  ;
    ledGRID[4] = 0xF9  ;
    ledGRID[5] = 0x9B  ;
    updateDisplay();
    delay(2000);
    Serial.print("Place a known weight on the scale...");
    delay(3000);
    raw = scale.read_average(5);  // giá trị sau khi đặt vật nặng
    Serial.print("Giá trị ADC sau khi đặt vật nặng: ");
    Serial.println(raw);
    Serial.print("Chênh lệch ADC: ");
    Serial.println(raw - raw1);
    delay(2000);
    calibration_factor  =(raw - raw1) / standard_kg;
    scale.set_scale(calibration_factor);
    scale.tare();
    delay(2000);
    Serial.print("giá trị standard ");
    Serial.println(standard_kg);
    ledGRID[0] = 0x63  ; //CALEND
    ledGRID[1] = 0xF9  ;
    ledGRID[2] = 0x43  ;
    ledGRID[3] = 0x73  ;
    ledGRID[4] = 0x19  ;
    ledGRID[5] = 0x9B  ;
    updateDisplay();
    delay(2000);
    prefs.begin("hx711", false); 
    prefs.putFloat("cal", calibration_factor);  // lưu vào flash
    prefs.end();
    Serial.print("Lưu vào flash");
    Serial.print(calibration_factor);
    btn1 =true;
    update2Firebase();
    delay(1000);
    clearDisplay();
    ledGRID[5] = 0xEB;
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////
  if (button2State) {  // Xử lý nút nhấn TARE
    if(timer_flag){
      // btn2 =! btn2;
      btn2 = false;
      update3Firebase();
      timer_flag = false;
    }
    button2State = false;
  }
  if(receivedCommand == "TARE"){
    if((millis() - pretime2Press) > buttonDebounce){
        pretime2Press = millis();
        btn2 = false;
        receivedCommand = "";
        Serial.println("Received TARE command via BLE");
    }
  }
  if(!btn2){
    scale.tare();
    Serial.println("đã tare");
    btn2 = true; 
    update3Firebase();
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
  weight = scale.get_units(3) / 1000.0;
  if(abs(weight) < noiseThreshold2) weight = 0.0;
  roundedWeight = roundf(weight * 1000.0f) / 1000.0f;
  dtostrf(roundedWeight, 6, 3, buffer_weight);  
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
  if(abs(roundedWeight - prev_weight) > noiseThreshold){
    prev_weight = roundedWeight;
    if (wifiConnected()) {
      if(timer_flag2){
        timer_flag2 = false;  
        update1Firebase(weight);
        // displayFloat(roundedWeight, 3, 0x04); ?
        // Serial.println("Đã cập nhật Firebase");   
        // Serial.println(roundedWeight);  
      }    
    }
    // else{
    //   if (millis() - lastBLEUpdate >= 50) {
    //     if(pServer != nullptr && pCharacteristic != nullptr){
    //       // if(timer_flag2){
    //       //   timer_flag2 = false;  
    //         if (deviceConnected) {
    //           pCharacteristic->setValue(buffer_weight);
    //           pCharacteristic->notify();
    //           // displayFloat(roundedWeight, 3, 0x04);
    //           Serial.println(roundedWeight);
    //           Serial.println("Sent with BLE success");
    //         }
    //       // }
    //     }
    //     lastBLEUpdate = millis();
    //   }
    //   if (millis() - lastESPNOWSend >= 100) {
    //     if (esp_now_init() == ESP_OK) {
    //       // esp_now_add_peer(esp2Mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    //       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&roundedWeight, sizeof(roundedWeight));
    //       if (result == ESP_OK) {
    //         Serial.println("Sent with espnow success");
    //       }
    //       else {
    //         Serial.println("Error sending the data");
    //       }
    //     }
    //     lastESPNOWSend = millis();
    //   }
    // } 
    else if (pServer != nullptr && pCharacteristic != nullptr) {
      // Serial.print("Loop: deviceConnected = ");
      // Serial.println(deviceConnected);
      if(timer_flag2){
        timer_flag2 = false;  
        if (deviceConnected) {
          pCharacteristic->setValue(buffer_weight);
          pCharacteristic->notify();
          displayFloat(roundedWeight, 3, 0x04);
          Serial.println(roundedWeight);
        }
      }
    } 
    else {
      Serial.println("Lỗi: Không có kết nối WiFi hoặc BLE");
    }
  }
}


