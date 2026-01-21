// ///KHỐI HIỂN THỊ 
// //ESP32 Board MAC Address: c8:2e:18:25:e4:7c
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <Preferences.h>
#include <TM1628.h>
#define PIN_DIO 21  // TM1628 DIO pin
#define PIN_CLK 22  // TM1628 CLK pin
#define PIN_STB 23  // TM1628 STB pin

#define RELAY_PIN1 19  // relay 1
#define RELAY_PIN2 18  // relay 2
#define RELAY_PIN3 17  // relay 3
#define RELAY_PIN4 16  // relay 4
#define LED_PIN 25

#define BUTTONPIN 2
#define BUTTONPIN2 4
#define LED_DATA_LENGTH 12 // 6 digits * 2 bytes per digit
#define SERVICE_UUID ""********************""
#define CHARACTERISTIC_UUID "********************"
TM1628 dvdLED(21, 22, 23); 
// Định nghĩa BLE UUID
hw_timer_t *timer; 
hw_timer_t *timer2; 
uint16_t physicalPos;
uint8_t dotPos;
uint16_t segment;
String receivedCommand = "";
bool muccan = false;
float muccannang1 = 0.0;
float muccannang2 = 0.0;
float muccannang3 = 0.0;
float muccannang4 = 0.0;
unsigned long lastBLEUpdate = 0;
unsigned long lastESPNOWSend = 0;
bool deviceConnected = false;  
bool oldDeviceConnected = false;

bool bleStarted = false;
char buffer_weight[20];
volatile bool timer_flag = false;  
volatile bool timer_flag2 = false;  
bool btn1 = true;
bool btn2 = true;
bool btn3 = true;
float weight;
float cannang;
float calibration_factor; // giá trị này bạn lấy từ code hiệu chỉnh
float raw;
float raw1;
volatile bool button1State = false;  
volatile bool button2State = false;
volatile bool button3State = false;
int digit;
float max_kg = 30.0;
float standard_kg; // 1kg = 1000g
float noiseThreshold = 0.001;  // Ngưỡng nhiễu để xác định khi nào giá trị ổn định: 1g
float noiseThreshold2 = 0.001;
float noiseThreshold3 = 0.0005; // Ngưỡng nhiễu để xác định khi nào giá trị ổn định quanh điểm 0: 5g
float prev_weight = 0;
float roundedWeight;
const int buttonDebounce = 200;
unsigned long pretime2Press = 0;
//////////////////////////////// KHAI BÁO ESP-NOW ///////////////////////////////////////////////////////
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
volatile String weightStr ;
volatile float receivedWeight = 0;
esp_now_peer_info_t peerInfo;
typedef struct struct_message {
  float weight_hx711;
  bool btn_calib; // Trạng thái nút cân
  bool btn_tare; // Trạng thái nút tare
  float standard_kg;
  float max_kg;
} struct_message;
struct_message myData;
BLEService* pService = nullptr;
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
///////////////////////////////////////KHAI BÁO 7SEG///////////////////////////////////////////////////////////////
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
  // 0xB4A,  // 9
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

/////////////////////////////////////////// HÀM HIỂN THỊ 7 SEGMENT /////////////////////////////////////////////////

void writeSingleCommand(const uint8_t command) {
  digitalWrite(PIN_STB, LOW);
  shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, command);
  digitalWrite(PIN_STB, HIGH);
  delayMicroseconds(1); // Пауза до следующей команды
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void updateDisplay(void) {
  writeSingleCommand(0x40);  // запись данных, автоматический адрес
  digitalWrite(PIN_STB, LOW);
  shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, 0xC0); // Установка адреса в 0
  uint8_t * p = (uint8_t *) ledGRID;
  for (int8_t i = 0; i < LED_DATA_LENGTH; i++, p++) shiftOut(PIN_DIO, PIN_CLK, LSBFIRST, *p); // запись данных
  digitalWrite(PIN_STB, HIGH);
  delayMicroseconds(1); // Пауза до следующей команды

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
static inline uint8_t setBrightness(const uint8_t newBrighness) {

    const uint8_t res = currentBrightness;

    currentBrightness = (newBrighness > 8) ? 8 : newBrighness;

    if (currentBrightness == 0) writeSingleCommand(0x80); // Выключить дисплей

    else writeSingleCommand(0x88 + (currentBrightness - 1)); // Установить яркость

    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void setDigit(const int8_t digit, const uint16_t value) {
  if (digit < 0 || digit > 6) return; // цифры у нас с 1-ой по 4-ую слева направо
  ledGRID[digit] = value; // не обижать дополнительный символ, если есть
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

  // value = roundf(value * pow(10, decimalPlaces)) / pow(10, decimalPlaces);
  //   // Kiểm tra giá trị gần số nguyên
  //   if (fabs(value - roundf(value)) < 0.001) {
  //       value = roundf(value);
  //   } 
  int highestDigitUsed = -1;
  bool isNegative = false;
  if (value < 0) {
  isNegative = true;
  value = -value;
  }
  // Nhân lên để thành số nguyên
  int intValue = (int)(value * pow(10, decimalPlaces));
  // int intValue = (int)(value * 1000);
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


//////////////////////////////////////// KHAI BÁO BLE //////////////////////////////////////////////////////////////
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    if (pServer != nullptr) {
      deviceConnected = true;
    } 
  }
  void onDisconnect(BLEServer* pServer) {
    if (pServer != nullptr) {
      deviceConnected = false;
      delay(100);
      BLEDevice::startAdvertising();
    }
  }
};
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override  {
    if (pCharacteristic != nullptr) {
      std::string value = pCharacteristic->getValue();
///////////////////////////////////////////////////////////////
      if (value.length() > 0 ) {
        receivedCommand = String(value.c_str());
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++) {
          Serial.print(value[i]);
        }
        Serial.println(receivedCommand);  
        Serial.println("*********");
//////////////////// NHẬN LỆNH TARE TỪ ESP32-BLE-CLIENT /////////////////////////
        if(receivedCommand == "TARE"){
          Serial.println("Received TARE command via BLE");
          if((millis() - pretime2Press) > buttonDebounce){
              pretime2Press = millis();
              myData.btn_tare = false;
              esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
              if (result == ESP_OK) {
                Serial.println("đã gửi sang esp 1 thành công lệnh TARE");
              }
              else {
                Serial.println("Error sending the data");
              }
              myData.btn_tare = true;
              receivedCommand = "";
          }
        }
////////////////////////////// NHẬN LỆNH CALIBRATE TỪ ESP32-BLE-CLIENT /////////////////////////
        else if(receivedCommand == "CALIBRATE"){
          Serial.println("Received CALIBRATE command via BLE");
          if((millis() - pretime2Press) > buttonDebounce){
              pretime2Press = millis();
              myData.btn_calib = false;
              esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
              if (result == ESP_OK) {
                Serial.println("đã gửi sang esp 1 thành công lệnh CALIBRATE");
              }
              else {
                Serial.println("Error sending the data");
              }
              digitalWrite(LED_PIN, HIGH);
              clearDisplay();
              ledGRID[0] = 0x63  ; //CAL
              ledGRID[1] = 0xF9  ;
              ledGRID[2] = 0x43  ;
              updateDisplay(); 
              digitalWrite(LED_PIN, LOW);
              delay(3800); /// đọc giá trị gốc ban đầu và sau 3 giây 
              Serial.print("Place a known weight on the scale...");
              digitalWrite(LED_PIN, HIGH);
              clearDisplay();
              ledGRID[0] = 0xF9  ; //ADLOAD
              ledGRID[1] = 0x9B  ;
              ledGRID[2] = 0x43  ;
              ledGRID[3] = 0xEB  ;
              ledGRID[4] = 0xF9  ;
              ledGRID[5] = 0x9B  ;
              updateDisplay();
              digitalWrite(LED_PIN, LOW);
              delay(5000);
              digitalWrite(LED_PIN, HIGH);
              clearDisplay();
              ledGRID[0] = 0x63  ; //CALEND
              ledGRID[1] = 0xF9  ;
              ledGRID[2] = 0x43  ;
              ledGRID[3] = 0x73  ;
              ledGRID[4] = 0x19  ;
              ledGRID[5] = 0x9B  ;
              updateDisplay();
              digitalWrite(LED_PIN, LOW);
              myData.btn_calib = true;
              receivedCommand = "";           
          }
        }
///////////////////// Xử lý lệnh VVB và MAX từ BLE //////////////////////////////
        else if (receivedCommand.startsWith("CAL") && receivedCommand.indexOf("MAX") > 0) {
          String calPart = receivedCommand.substring(3, receivedCommand.indexOf("MAX"));
          String maxPart = receivedCommand.substring(receivedCommand.indexOf("MAX") + 3);

          myData.standard_kg = calPart.toFloat();
          myData.max_kg = maxPart.toFloat();

          Serial.println("Received CAL:, MAX: ");
          Serial.print(myData.standard_kg);
          Serial.println(myData.max_kg);
          esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
              if (result == ESP_OK) {
                Serial.println("đã gửi dữ liệu standard_kg sang esp1");
              }
              else {
                Serial.println("Error sending the data");
              }
          receivedCommand = "";
        }
        else if (receivedCommand.startsWith("M1") && receivedCommand.indexOf("M2") > 0 && 
                    receivedCommand.indexOf("M3") > 0 && receivedCommand.indexOf("M4") > 0) {

          String m1 = receivedCommand.substring(2, receivedCommand.indexOf("M2"));
          String m2 = receivedCommand.substring(receivedCommand.indexOf("M2") + 2, receivedCommand.indexOf("M3"));
          String m3 = receivedCommand.substring(receivedCommand.indexOf("M3") + 2, receivedCommand.indexOf("M4"));
          String m4 = receivedCommand.substring(receivedCommand.indexOf("M4") + 2);

           muccannang1 = m1.toFloat() / 1000.0f; // Chuyển đổi từ kg sang g
           muccannang2 = m2.toFloat() / 1000.0f;
           muccannang3 = m3.toFloat() / 1000.0f;
           muccannang4 = m4.toFloat() / 1000.0f;
                       
          
          if (muccannang1 > 0 && muccannang2 > 0 && 
            muccannang3 > 0 && muccannang4 > 0) {
            muccan = true; 
            Serial.println("Received M1, M2, M3, M4: ");
            Serial.println(muccannang1);
            Serial.println(muccannang2);
            Serial.println(muccannang3);
            Serial.println(muccannang4);
          }else{
            muccan = false; // Nếu bất kỳ giá trị nào không hợp lệ, tắt chế độ cân
            Serial.println("Invalid M1, M2, M3, M4 values received. Disabling muccan.");
          }
          receivedCommand = "";
        }
        else{
          /////nop
        }
///////////////////////////////////////////////////////////////
      }
    }
  }
  // void onRead(BLECharacteristic *pCharacteristic) {
  //       if (pCharacteristic != nullptr) {
  //           Serial.println("Characteristic read");
  //           pCharacteristic->setValue("BLE read");
  //       }
  //   }
};

void startBLE() {
  BLEDevice::init("ESP32-BLE-Server");
  pServer = BLEDevice::createServer();

  // serverCallbacks = new MyServerCallbacks();
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE|
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("BLE started");

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("BLE đang hoạt động...");
}

void controlRelay(float weightInt){
  
  static int lastActiveRelay = -1;
  int activeRelay = -1;
   //TẮT RELAY NẾU RELAY KÍCH HOẠT MỨC THẤP
      if (weightInt >= muccannang4) {
          activeRelay = 3;
      } else if (weightInt >= muccannang3) {
          activeRelay = 2;
      } else if (weightInt >= muccannang2) {
          activeRelay = 1;
      } else if (weightInt >= muccannang1) {
          activeRelay = 0;
      }
  
  if (activeRelay != lastActiveRelay) {
    // Tắt tất cả các relay trước khi bật relay mới
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    // Bật relay tương ứng với mức cân nặng
    switch (activeRelay) {
      case 0:
        digitalWrite(RELAY_PIN1, LOW);
        break;
      case 1:
        digitalWrite(RELAY_PIN2, LOW);
        break;
      case 2:
        digitalWrite(RELAY_PIN3, LOW);
        break;
      case 3:
        digitalWrite(RELAY_PIN4, LOW);
        break;
      default:
        break;
    }
    lastActiveRelay = activeRelay;
  }
}
void IRAM_ATTR onTimer() {
  timer_flag = true;  
}
void IRAM_ATTR onTimer2() {
  timer_flag2 = true;  
}

//////////////////////////////////////////// HÀM THỰC THI ESP-NOW ///////////////////////////////////////////////////////

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_message)) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Bytes received HX711: ");
    Serial.println(len);
    Serial.print("Weight received HX711: ");
    Serial.println(myData.weight_hx711);
  }else {
    Serial.println("Data size mismatch");
  } 
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//////////////////////////////////////////// HÀM BẤM NÚT ///////////////////////////////////////////////////////

void IRAM_ATTR button1Push(){ //TARE
  button1State = true;
}
void IRAM_ATTR button2Push(){   //HIỆU CHUẨN
  button2State = true;
}
void IRAM_ATTR button3Push(){
  button3State = true;
}

///////////////////////////////////////// SETUP //////////////////////////////////////////////////////////////

void setup(void) {
  Serial.begin(9600);
  dvdLED.begin(ON, 7);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RELAY_PIN3, OUTPUT);
  pinMode(RELAY_PIN4, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);  // TM1628 CLK pin
  pinMode(PIN_DIO, OUTPUT);  // TM1628 DIO pin
  pinMode(PIN_STB, OUTPUT);  // TM1628 STB pin

  digitalWrite(RELAY_PIN1, HIGH);
  digitalWrite(RELAY_PIN2, HIGH);
  digitalWrite(RELAY_PIN3, HIGH);
  digitalWrite(RELAY_PIN4, HIGH);
  digitalWrite(PIN_STB, HIGH); // TM1628 STB pin
  digitalWrite(PIN_CLK, HIGH);  // TM1628 CLK pin

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

///////////////////////////////// Init ESP-NOW /////////////////////////////////////////////////////////////////
  
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
  esp_now_register_recv_cb(OnDataRecv);
  
///////////////////////////// BLE ////////////////////////////////////
  startBLE();
  bleStarted = true;
///////////////////////////// TIMER ////////////////////////////////////
  timer = timerBegin(0, 80, true); 
  timerAttachInterrupt(timer, &onTimer, true); 
  timerAlarmWrite(timer, 200000, true); 
  timerAlarmEnable(timer);
  timer2 = timerBegin(1, 80, true); 
  timerAttachInterrupt(timer2, &onTimer2, true); 
  timerAlarmWrite(timer2, 100000, true); 
  timerAlarmEnable(timer2);
  setBrightness(currentBrightness);
  

  pinMode(BUTTONPIN, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), button1Push, FALLING);
  pinMode(BUTTONPIN2, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN2), button2Push, FALLING);

  clearDisplay();
  ledGRID[5] = 0xEB;//SỐ 0

  updateDisplay();
  myData.btn_calib = true;
  myData.btn_tare = true;
  delay(1000); // Đợi 1 giây để đảm bảo mọi thứ đã sẵn sàng
  Serial.println("Setup complete. Waiting for data...");
}
///////////////////////////////////////// LOOP //////////////////////////////////////////////////////////////
void loop(void) {

  receivedWeight = myData.weight_hx711;

  roundedWeight = roundf(receivedWeight * 1000.0f) / 1000.0f;

  dtostrf(receivedWeight, 6, 3, buffer_weight);

  controlRelay(receivedWeight);

  if(timer_flag){
      displayFloat(receivedWeight, 3, 0x04);
      timer_flag = false; // Reset the flag after displaying
  }
///////////////// KIỂM TRA TRẠNG THÁI KẾT NỐI BLE ///////////////////////

  if (!deviceConnected && oldDeviceConnected) {
    delay(100);
    pServer->startAdvertising();
    Serial.println("BLE advertising restarted");
    oldDeviceConnected = false;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = true;
  }

  ///////////////////////////// GỬI DỮ LIỆU SANG BLE ///////////////////////////////////////
  // if (fabs(receivedWeight - prev_weight) > noiseThreshold2) {
  // prev_weight = receivedWeight;
  if (pServer != nullptr && pCharacteristic != nullptr && deviceConnected) {
    if(timer_flag2){
        pCharacteristic->setValue(buffer_weight);
        pCharacteristic->notify();
        Serial.print("Sent to BLE: ");
        Serial.println(buffer_weight);
        timer_flag2 = false; // Reset the flag after sending
    }
  }
// }
//////////////// XỬ LÝ DỮ LIỆU CÁC MỨC CÂN NẶNG ////////////////////////////////
  
}


