//KHỐI GỬI DỮ LIỆU
//ESP32 Board MAC Address: cc:db:a7:92:aa:2c
///////////////////////////////CHƯƠNG TRÌNH ESPNOW SENDER//////////////////////////////////////////////
#include <Arduino.h>
#include <WiFi.h>
#include "HX711.h"
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define DT 26     //HX711 Pin
#define SCK 27   //HX711 Pin
#define BUTTON_PIN 15   // Button pin
#define BUTTON_PIN2 17  // Button pin 2
#define BUTTON_PIN3 16  // Button pin 3
#define LED_DATA_LENGTH 12 // 6 digits * 2 bytes per digit

#define SERVICE_UUID        "*****************"
#define CHARACTERISTIC_UUID "*****************"

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
char buffer_weight[15];
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
float standard_kg ; // 1kg = 1000g
float noiseThreshold = 0.0015;  // Ngưỡng nhiễu để xác định khi nào giá trị ổn định: 0.8 g
float noiseThreshold2 = 0.005; // Ngưỡng nhiễu để xác định khi nào giá trị ổn định quanh điểm 0: 5g
float prev_weight = 0;
float roundedWeight;
const int buttonDebounce = 200;
unsigned long pretime2Press = 0;
unsigned long wifiStartTime ;
const unsigned long wifiTimeout  = 20000;   // Thời gian chờ kết nối WiFi (10 giây)
float lastStandardKg = 0.0;
String receivedCommand = ""; // Lưu lệnh nhận từ BLE
typedef struct struct_message {
  float weight_hx711;
  bool btn_calib; // Trạng thái nút cân
  bool btn_tare; // Trạng thái nút tare
  float standard_kg;
  float max_kg;
} struct_message;
struct_message myData;

esp_now_peer_info_t peerInfo;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void IRAM_ATTR onTimer() {
  timer_flag = true;  
}
void IRAM_ATTR onTimer2() {
  timer_flag2 = true;  
}
////////////////////////// ESP-NOW ///////////////////////////////////////////////////////
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("btn_calib: ");
  Serial.println(myData.btn_calib ? "true" : "false");
  Serial.print("btn_tare: ");
  Serial.println(myData.btn_tare ? "true" : "false");
  Serial.print("giá trị cân hiệu chuẩn");
  Serial.println(myData.standard_kg);  
}

////////////////////////////////////// MAIN /////////////////////////////////
 void setup(void) {
  Serial.begin(9600);
  scale.begin(DT, SCK);

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

///////////////////////////////////// HX711 INIT /////////////////////////////////////////////////////////////////
  Serial.println("Khởi động HX711...");
  while (!scale.is_ready()) {
    Serial.println("HX711 chưa sẵn sàng...");
    delay(500);
  }
  Serial.println("HX711 đã sẵn sàng");

/////////////////////////////// Init ESP-NOW ///////////////////////////////////////////////////////
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;     
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
/////////////// TIMER SETUP ///////////////////////////////////////////////////////
  timer = timerBegin(0, 80, true); 
  timerAttachInterrupt(timer, &onTimer, true); 
  timerAlarmWrite(timer, 100000, true); 
  timerAlarmEnable(timer);
  timer2 = timerBegin(1, 80, true); 
  timerAttachInterrupt(timer2, &onTimer2, true); 
  timerAlarmWrite(timer2, 400000, true); 
  timerAlarmEnable(timer2);


//////////////////////////////////////////////////////////////////////////////////////////////////////
  prefs.begin("hx711", false);
  if(prefs.isKey("standard_kg")){
    myData.standard_kg = prefs.getFloat("standard_kg", 1.0);
    Serial.print("Đọc hệ số standard_kg từ flash: ");
    Serial.println(myData.standard_kg);
    
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
    calibration_factor  =(raw - raw1) / myData.standard_kg;
    scale.set_scale(calibration_factor);
    delay(5000);
    scale.tare();
    prefs.putFloat("cal", calibration_factor);  // lưu vào flash
    Serial.println("Đã lưu hệ số vào flash.");
    Serial.println(calibration_factor);
  }
  prefs.end();

  myData.btn_calib = true;
  myData.btn_tare = true;

 }

void loop(){

  weight = scale.get_units(3) / 1000.0; //1000 là g thành kg

  if(fabs(weight) < noiseThreshold2) weight = 0.0;

  // roundedWeight = roundf(weight * 1000.0f) / 1000.0f;
  int temp = (int)(weight * 1000.0f + 0.5f); // Làm tròn chính xác
  roundedWeight = temp / 1000.0f;
  // myData.weight_hx711 = roundedWeight;
  Serial.println(myData.weight_hx711, 4);

  if(fabs(roundedWeight - prev_weight) > noiseThreshold){
    prev_weight = roundedWeight;
    if(timer_flag){
      myData.weight_hx711 = roundedWeight;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      timer_flag = false; 
    }  
  }else{
    roundedWeight = prev_weight;
  }

  if(myData.btn_calib == false){
    Serial.println(">>> ĐANG Ở CHẾ ĐỘ HIỆU CHỈNH ở vòng loop <<<");
    delay(500);
    raw1 = scale.read_average(5);   // giá trị ADC gốc
    Serial.print("Giá trị ADC trước khi đặt vật nặng: ");
    Serial.println(raw1);
    delay(3000);
    Serial.print("Place a known weight on the scale...");
    delay(3000);
    raw = scale.read_average(5);  // giá trị sau khi đặt vật nặng
    Serial.print("Giá trị ADC sau khi đặt vật nặng: ");
    Serial.println(raw);
    Serial.print("Chênh lệch ADC: ");
    Serial.println(raw - raw1);
    
    calibration_factor  =(raw - raw1) / myData.standard_kg;
    scale.set_scale(calibration_factor);
    delay(3000);
    scale.tare();
    Serial.print("giá trị standard ");
    Serial.println(myData.standard_kg);
    delay(1000);
    prefs.begin("hx711", false); 
    prefs.putFloat("cal", calibration_factor);  // lưu vào flash
    prefs.end();
    Serial.print("Lưu vào flash");
    Serial.print(calibration_factor);
    myData.btn_calib = true;
  }

  if(myData.btn_tare == false){
    scale.tare();
    Serial.println("đã tare");
    myData.btn_tare = true;
  }
  if (myData.standard_kg != lastStandardKg) {
    prefs.begin("hx711", false);
    prefs.putFloat("standard_kg", myData.standard_kg);
    prefs.end();
    lastStandardKg = myData.standard_kg;
    Serial.print("đã lưu giá trị cân chuẩn vào flash");
  }
}




