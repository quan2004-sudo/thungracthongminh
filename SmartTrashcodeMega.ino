#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include "DFRobotDFPlayerMini.h"
#include <Servo.h>

uint32_t last_check = 0;
uint32_t lastRead = 0;
uint32_t lastSendData = 0;



SoftwareSerial mySoftwareSerial(11, 10);  // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
//SoftwareSerial sim800l(8, 9);  // RX, TX
//SoftwareSerial gps_ss(5, 18);    // gps

Servo myservo;
int pos = 0;

const int servo_pin = 12;
const int relay1 = 8;
const int relay2 = 9;
const int rung = 3;

const int MQ135_PIN = A0;
const int MQ2_PIN = A1;
const int trigPin = 6;  // chân trig của HC-SR04
const int echoPin = 7;  // chân echo của HC-SR04
const int nutmokhoa = 2;
bool check1 = false; // thung rac day
bool check2 = false;// khoa
bool check3 = false;// phun suong

int average, MQ2_val;
int  trangthaiphunsuong, trangthaikhoa, trangthaithungrac, val_nut;
long int MQ135_ppm, MQ2_ppm, Latitude, Longitude;

//MQ135 mq135_sensor = MQ135(MQ135_PIN);  //Khai báo đối tượng thư viện
//String phoneNumber = "+84961230706";    // Số điện thoại nhận tin nhắn
DFRobotDFPlayerMini dfplayer;
//TinyGPSPlus gps;

//float temperature = 25.0;
//float humidity = 60.0;
int test = 1;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(servo_pin);  //D10
  Serial.begin(115200);
  // gps_ss.begin(9600);
  mySoftwareSerial.begin(9600);
  //sim800l.begin(9600);
  Serial3.begin(57600);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(nutmokhoa, INPUT_PULLUP);  // khai báo chân số 2 là ngõ vào sử dụng điện trở kéo lên.​
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  pinMode(rung, INPUT);
  pinMode(trigPin, OUTPUT);  // Thiết lập chân trig là OUTPUT
  pinMode(echoPin, INPUT);   // Thiết lập chân echo là INPUT
                             // pinMode(nutmokhoa, INPUT_PULLUP);
                             // attachInterrupt(0, mokhoa, LOW);
                             //attachInterrupt(0, mokhoa, LOW);​
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  delay(3000);
  while (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    delay(500);
    while (true)
      ;
  }



  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
}

void loop() {


  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());  //Print the detail message from DFPlayer to handle different errors and states.
  }
  val_nut = digitalRead(nutmokhoa);
  docackhi();  // đo các khí
  //readGPS();        // đọc gps
  read_distance();  // đo khoảng cách
  bool val_rung = digitalRead(rung);


  if (average < 30) {
if (check1 == false){
  check1 = true;
  check2 = true;
}
    // trangthaithungrac = 1;
    // trangthaikhoa = 1;
    myservo.write(180);  // khoa

    Serial.println(" thung rac da duoc khoa ");
    // myDFPlayer.play(3);

    if (val_nut == 0) {
      mokhoa();
    }
    if (val_rung == 0) {
      myDFPlayer.play(2);
      delay(100);
      Serial.println(" canh bao rung");
    }
  }


  else {
    myservo.write(0);

    
    Serial.println("thung rac khong khoa, chua day rac ");
    check1= false;
    check2 = false;
    
    if (val_rung == 0) {

      myDFPlayer.play(4);
      delay(100);

      Serial.println(" cam on ban da cho toi rac");
    }
  }
  if (MQ135_ppm < 9000 || MQ2_ppm < 9000) {
    // dfplayer.play(4);
    if (check3 == false)
    {
      check3= true;
    }
    digitalWrite(relay2, HIGH);
    digitalWrite(relay1, HIGH);
    delay(100);
    digitalWrite(relay2, LOW);
    digitalWrite(relay1, LOW);
    delay(15000);
    Serial.println("phun dung dich khu mui");

    



    digitalWrite(relay2, HIGH);
    digitalWrite(relay1, HIGH);
    delay(100);
    digitalWrite(relay2, LOW);
    digitalWrite(relay1, LOW);
    Serial.println(" Dung phun sung dich khu mui");
   
  } else  {
    
check3 = false;
  
    delay(10);
    Serial.println(" khong phun dung dich");
  }



  if (millis() - lastSendData > 2000) {
    update_dulieu();
    Serial.println("upload done ");

    lastSendData = millis();
  }
}

void docackhi() {

  MQ2_val = analogRead(MQ2_PIN);
  MQ2_ppm = map(MQ2_val, 0, 4095, 10000, 0);


  int MQ135_val = analogRead(MQ135_PIN);
  MQ135_ppm = map(MQ135_val, 0, 4095, 10000, 0);
  Serial.print("MQ135 :");
  Serial.print(MQ135_ppm);

  Serial.print("\t MQ2 :");
  Serial.println(MQ2_ppm);

  delay(1000);
}





void read_distance() {
  int distance = 0;  // Khởi tạo biến khoảng cách bằng 0
  int sum = 0;       // Khởi tạo biến tổng khoảng cách bằng 0

  for (int i = 0; i < 10; i++) {
    digitalWrite(trigPin, LOW);  // Đưa chân trig về mức thấp
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);  // Đưa chân trig về mức cao
    delayMicroseconds(10);        //xung có độ dài 10 microSeconds
    digitalWrite(trigPin, LOW);   // Đưa chân trig về mức thấp

    int duration = pulseIn(echoPin, HIGH);  // Đọc thời gian nhận được echo
    distance = duration / 29.412 / 2;       // Tính khoảng cách từ thời gian nhận được echo
    sum += distance;                        // Cộng dồn khoảng cách
    delay(500);                             // Đợi 100ms trước khi đo lại
  }

  average = sum / 10;  // Tính trung bình khoảng cách
  Serial.print("Average distance: ");
  Serial.print(average);
  Serial.println(" cm");
}




void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      myDFPlayer.play(1);  //Play the first mp3
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
  }
}

void mokhoa() {
myDFPlayer.play(5);
check2 = false;

  myservo.write(0);
  delay(12000);
  Serial.println(" cam on bac lao cong");
  myservo.write(180);
  delay(100);
  check2= true;
}


void update_dulieu() {

 

  float data[6] = { test, MQ135_ppm, MQ2_ppm, check3, check2, check1 };  // Dữ liệu của 5 cảm biến
  Serial3.write((byte*)data, sizeof(data));                                                            // Gửi dữ liệu qua UART
  delay(100);
}