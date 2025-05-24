#include <WiFi.h>        // เรียกใช้ไลบรารี WiFi สำหรับเชื่อมต่อเครือข่ายอินเทอร์เน็ต
#include <WiFiClient.h>  // เรียกใช้ไลบรารี WiFiClient สำหรับสร้างคลาสลูกค้า (client) ที่ใช้ในการสื่อสารผ่านเครือข่าย

char ssid[] = "for_pro";       // กำหนดชื่อ SSID ของเครือข่าย WiFi ที่จะเชื่อมต่อ
char pass[] = "113333555555";  // กำหนดรหัสผ่านสำหรับเชื่อมต่อเครือข่าย WiFi

#include <Wire.h>              // เรียกใช้ไลบรารี Wire สำหรับการสื่อสารแบบ I2C
#include <Adafruit_GFX.h>      // เรียกใช้ไลบรารี Adafruit GFX สำหรับการวาดกราฟิกพื้นฐานบนจอแสดงผล
#include <Adafruit_SSD1306.h>  // เรียกใช้ไลบรารีสำหรับควบคุมจอ OLED รุ่น SSD1306
Adafruit_SSD1306 OLED(-1);     // สร้างออบเจ็กต์ OLED สำหรับควบคุมจอ OLED โดยใช้ค่า -1 (หมายถึงไม่มีพินรีเซ็ตที่กำหนด)

#include <DHT.h>           // เรียกใช้ไลบรารี DHT สำหรับเซ็นเซอร์วัดอุณหภูมิและความชื้น
#define DHTTYPE DHT22      // กำหนดชนิดของเซ็นเซอร์ DHT ให้เป็นรุ่น DHT22
#define DHTPIN 25          // กำหนดให้ขาที่ 25 ของ ESP32 ใช้ต่อกับเซ็นเซอร์ DHT
DHT dht(DHTPIN, DHTTYPE);  // สร้างออบเจ็กต์ dht สำหรับการสื่อสารกับเซ็นเซอร์ DHT โดยระบุขาและชนิดของเซ็นเซอร์

float humid;    // ประกาศตัวแปรสำหรับเก็บค่าความชื้นที่อ่านได้จากเซ็นเซอร์
float temp;     // ประกาศตัวแปรสำหรับเก็บค่าอุณหภูมิที่อ่านได้จากเซ็นเซอร์
float average;  // ประกาศตัวแปรสำหรับเก็บค่าเฉลี่ย (อาจใช้สำหรับคำนวณภายหลัง)

#define BLYNK_PRINT Serial                                   // กำหนดให้ Blynk ส่ง log ผ่านทาง Serial Monitor
#define BLYNK_TEMPLATE_ID "*************"                    // กำหนด Template ID สำหรับโปรเจคบน Blynk
#define BLYNK_TEMPLATE_NAME "**********"                     // กำหนดชื่อ Template สำหรับโปรเจคบน Blynk
#define BLYNK_AUTH_TOKEN "********************************"  // กำหนด Authentication Token สำหรับเชื่อมต่อกับ Blynk

#include <BlynkMultiClient.h>       // เรียกใช้ไลบรารี BlynkMultiClient สำหรับเชื่อมต่อหลาย client กับ Blynk
static WiFiClient blynkWiFiClient;  // สร้างออบเจ็กต์ WiFi client สำหรับใช้กับการเชื่อมต่อของ Blynk
BlynkTimer timer;                   // สร้างออบเจ็กต์ Timer ของ Blynk สำหรับจัดการฟังก์ชันที่ต้องทำซ้ำตามช่วงเวลา
WidgetLED led1(V1);                 // สร้างวิดเจ็ต LED สำหรับควบคุม LED ที่ virtual pin V1 บนแอป Blynk
WidgetLED led2(V6);                 // สร้างวิดเจ็ต LED สำหรับควบคุม LED ที่ virtual pin V6 บนแอป Blynk

const int analogInPin01 = 33;  // กำหนดขา 33 ของ ESP32 สำหรับอ่านค่าสัญญาณอนาล็อกจากเซ็นเซอร์ความชื้นในดินตัวที่ 1
const int analogInPin02 = 32;  // กำหนดขา 32 ของ ESP32 สำหรับอ่านค่าสัญญาณอนาล็อกจากเซ็นเซอร์ความชื้นในดินตัวที่ 2
const int digittalPin = 19;    // กำหนดขาดิจิตอลที่ 19 ของ ESP32 สำหรับอ่านสถานะของสวิตช์หรืออินพุตดิจิตอลอื่น ๆ

int sensorValue01 = 0;  // ประกาศตัวแปรเพื่อเก็บค่าสัญญาณอ่านจากเซ็นเซอร์ความชื้นดินตัวที่ 1
int moisture01 = 0;     // ประกาศตัวแปรเพื่อเก็บค่าความชื้นในดินที่ได้จากการแปลงค่า sensorValue01
int sensorValue02 = 0;  // ประกาศตัวแปรเพื่อเก็บค่าสัญญาณอ่านจากเซ็นเซอร์ความชื้นดินตัวที่ 2
int moisture02 = 0;     // ประกาศตัวแปรเพื่อเก็บค่าความชื้นในดินที่ได้จากการแปลงค่า sensorValue02

#define FanPin 16         // กำหนดขา 16 ของ ESP32 สำหรับควบคุมพัดลม
#define WaterPumpnPin 17  // กำหนดขา 17 ของ ESP32 สำหรับควบคุมปั้มน้ำ
#define LEDPin 23         // กำหนดขา 23 ของ ESP32 สำหรับควบคุม LED
int moistureThreshold;                  // ประกาศตัวแปรเพิ่มเติม 

void connectWiFi()  // นิยามฟังก์ชันสำหรับเชื่อมต่อกับเครือข่าย WiFi
{
  Serial.print("Connecting to ");  // พิมพ์ข้อความ "Connecting to" ลงใน Serial Monitor
  Serial.println(ssid);            // พิมพ์ชื่อ SSID ลงใน Serial Monitor

  if (pass && strlen(pass))  // ตรวจสอบว่ามีการกำหนดรหัสผ่านหรือไม่ (เช็คว่าตัวแปร pass ไม่เป็นค่าว่าง)
  {
    WiFi.begin(ssid, pass);  // เริ่มต้นเชื่อมต่อ WiFi โดยใช้ SSID และรหัสผ่านที่กำหนดไว้
  } else {
    WiFi.begin(ssid);  // ถ้าไม่มีรหัสผ่าน ให้เชื่อมต่อด้วย SSID เท่านั้น
  }

  // วนลูปรอจนกว่า WiFi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    OLED.clearDisplay();         // ล้างหน้าจอ OLED เพื่อเตรียมแสดงข้อความสถานะ
    OLED.setTextColor(WHITE);    // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
    OLED.setCursor(38, 9);       // กำหนดตำแหน่งเริ่มต้นของข้อความบน OLED
    OLED.setTextSize(1.5);       // ตั้งขนาดตัวอักษรให้มีขนาด 1.5 เท่า
    OLED.println("Connecting");  // แสดงข้อความ "Connecting" บนหน้าจอ OLED
    OLED.setCursor(45, 18);      // เปลี่ยนตำแหน่งข้อความเพื่อแสดงบรรทัดที่สอง
    OLED.setTextSize(1.5);       // กำหนดขนาดข้อความในบรรทัดที่สอง
    OLED.println("to Wifi");     // แสดงข้อความ "to Wifi" บนหน้าจอ OLED
    OLED.display();              // อัปเดตหน้าจอ OLED เพื่อแสดงข้อความที่กำหนด
    delay(500);                  // หน่วงเวลา 500 มิลลิวินาที (0.5 วินาที) ก่อนที่จะวนลูปใหม่
    Serial.print(".");           // พิมพ์จุดใน Serial Monitor เพื่อแสดงความคืบหน้าของการเชื่อมต่อ
  }
}

void setup() {
  Blynk.addClient("WiFi", blynkWiFiClient, 80);  // เพิ่ม WiFi client สำหรับ Blynk โดยใช้ชื่อ "WiFi" และพอร์ต 80
  Blynk.config(BLYNK_AUTH_TOKEN);                // กำหนดค่า Authentication Token สำหรับการเชื่อมต่อกับ Blynk
  dht.begin();                                   // เริ่มต้นการทำงานของเซ็นเซอร์ DHT ให้พร้อมใช้งาน
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);        // เริ่มต้นจอ OLED โดยใช้โหมดพลังงาน SSD1306_SWITCHCAPVCC และกำหนดที่อยู่ I2C เป็น 0x3C
  Serial.begin(115200);                          // เริ่มต้น Serial Monitor ด้วยความเร็ว 115200 baud
  pinMode(digittalPin, INPUT);                   // กำหนดขาดิจิตอล (digittalPin) เป็น input เพื่อรับสถานะสวิตช์
  pinMode(LEDPin, OUTPUT);                       // กำหนดขา LEDPin เป็น output เพื่อควบคุม LED
  pinMode(FanPin, OUTPUT);                       // กำหนดขา FanPin เป็น output เพื่อควบคุมพัดลม
  pinMode(WaterPumpnPin, OUTPUT);                // กำหนดขา WaterPumpnPin เป็น output เพื่อควบคุมปั้มน้ำ
  connectWiFi();                                 // เรียกใช้ฟังก์ชันเพื่อเชื่อมต่อกับเครือข่าย WiFi
  timer.setInterval(1000, sendDHTdata);          // ตั้งค่าให้งาน sendDHTdata ถูกเรียกทุกๆ 1000 มิลลิวินาที (1 วินาที)

  OLED.clearDisplay();       // ล้างหน้าจอ OLED ก่อนการแสดงข้อความ
  OLED.setTextColor(WHITE);  // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
  OLED.setCursor(27, 10);    // กำหนดตำแหน่งเริ่มต้นของข้อความบน OLED
  OLED.setTextSize(2);       // ตั้งขนาดตัวอักษรให้มีขนาดใหญ่ขึ้น (2 เท่า)
  OLED.println("Welcome");   // แสดงข้อความ "Welcome" บนหน้าจอ OLED
  OLED.display();            // อัปเดตหน้าจอ OLED เพื่อแสดงข้อความที่ตั้งค่าไว้
  delay(3000);               // หน่วงเวลา 3000 มิลลิวินาที (3 วินาที) เพื่อให้ผู้ใช้เห็นข้อความ Welcome
}

// ฟังก์ชัน BLYNK_WRITE จะถูกเรียกเมื่อค่าของ virtual pin V3 บนแอป Blynk มีการเปลี่ยนแปลง
BLYNK_WRITE(V3) {
  int pinValue03 = param.asInt();             // รับค่า input จาก virtual pin V3 แล้วแปลงเป็นจำนวนเต็ม
  Serial.print("value of water pump is : ");  // พิมพ์ข้อความเพื่อบอกค่าที่ได้รับสำหรับปั้มน้ำ
  Serial.println(pinValue03);                 // พิมพ์ค่าที่ได้ลงใน Serial Monitor
  analogWrite(WaterPumpnPin, pinValue03);     // ส่งค่าที่ได้ไปยังพิน WaterPumpnPin เพื่อควบคุมความเร็ว/กำลังของปั้มน้ำด้วย analogWrite

  if (pinValue03 > 0)  // ถ้าค่าที่ได้มากกว่า 0 (หมายความว่าปั้มน้ำควรทำงาน)
  {
    OLED.clearDisplay();         // ล้างหน้าจอ OLED เพื่อเตรียมการแสดงข้อความใหม่
    OLED.setTextColor(WHITE);    // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
    OLED.setCursor(35, 9);       // กำหนดตำแหน่งแรกของข้อความบน OLED
    OLED.setTextSize(1.5);       // ตั้งขนาดตัวอักษรเป็น 1.5 เท่า
    OLED.println("Water pump");  // แสดงข้อความ "Water pump" บนหน้าจอ OLED
    OLED.setCursor(35, 18);      // เปลี่ยนตำแหน่งของข้อความสำหรับบรรทัดที่สอง
    OLED.setTextSize(1.5);       // ยืนยันขนาดตัวอักษรของบรรทัดที่สอง
    OLED.println("is working");  // แสดงข้อความ "is working" บนหน้าจอ OLED เพื่อแจ้งว่าปั้มน้ำทำงานอยู่
    OLED.display();              // อัปเดตหน้าจอ OLED ให้แสดงข้อความที่ตั้งค่าไว้
    delay(1000);                 // หน่วงเวลา 1000 มิลลิวินาที (1 วินาที) เพื่อให้ข้อความแสดงได้ชัดเจน
  }
}

// เมื่อมีการส่งค่าจาก virtual pin V0 บนแอป Blynk ฟังก์ชันนี้จะถูกเรียก
BLYNK_WRITE(V0) {
  moistureThreshold = param.asInt();  // อ่านข้อมูลที่ถูกส่งมาจาก Blynk (ในรูปแบบ integer) แล้วเก็บไว้ในตัวแปร moistureThreshold
}

// ฟังก์ชันสำหรับส่งข้อมูลจากเซ็นเซอร์ DHT ไปยังแอป Blynk
void sendDHTdata() {
  humid = dht.readHumidity();     // อ่านค่าความชื้นจากเซ็นเซอร์ DHT แล้วเก็บไว้ในตัวแปร humid
  temp = dht.readTemperature();   // อ่านค่าอุณหภูมิจากเซ็นเซอร์ DHT แล้วเก็บไว้ในตัวแปร temp
  Serial.print("Humidity: ");     // พิมพ์ข้อความ "Humidity: " ลงใน Serial Monitor
  Serial.print(humid);            // พิมพ์ค่าความชื้นที่อ่านได้ลงใน Serial Monitor
  Serial.print("% | ");           // พิมพ์เครื่องหมายเปอร์เซ็นต์และแยกด้วยเครื่องหมาย " | " เพื่อความชัดเจน
  Serial.print("Temperature: ");  // พิมพ์ข้อความ "Temperature: " ลงใน Serial Monitor
  Serial.print(temp);             // พิมพ์ค่าอุณหภูมิที่อ่านได้ลงใน Serial Monitor
  Serial.println("C");            // พิมพ์ข้อความ "C" (องศาเซลเซียส) และขึ้นบรรทัดใหม่ใน Serial Monitor
  Blynk.virtualWrite(V4, temp);   // ส่งค่าอุณหภูมิไปที่ virtual pin V4 บนแอป Blynk
  Blynk.virtualWrite(V5, humid);  // ส่งค่าความชื้นไปที่ virtual pin V5 บนแอป Blynk
}

// ฟังก์ชันหลัก loop ทำงานวนซ้ำตลอดเวลา
void loop() {
  int average, dt;                                   // ประกาศตัวแปร average สำหรับเก็บค่าเฉลี่ยความชื้นในดิน และ dt สำหรับเก็บสถานะจากสวิตช์
  dt = digitalRead(digittalPin);                     // อ่านค่าสถานะจาก digittalPin (ขา 19) ว่ามีสถานะเป็น HIGH หรือ LOW แล้วเก็บไว้ใน dt
  sensorValue01 = analogRead(analogInPin01);         // อ่านค่าสัญญาณอนาล็อกจากเซ็นเซอร์ความชื้นในดินตัวที่ 1 (ขา 33)
  moisture01 = map(sensorValue01, 4095, 0, 0, 100);  // แปลงค่าสัญญาณจาก sensorValue01 จากช่วง 4095-0 ให้เป็นช่วง 0-100 เพื่อคำนวณความชื้นในดิน
  sensorValue02 = analogRead(analogInPin02);         // อ่านค่าสัญญาณอนาล็อกจากเซ็นเซอร์ความชื้นในดินตัวที่ 2 (ขา 32)
  moisture02 = map(sensorValue02, 4095, 0, 0, 100);  // แปลงค่าสัญญาณจาก sensorValue02 จากช่วง 4095-0 ให้เป็นช่วง 0-100 เพื่อคำนวณความชื้นในดิน

  // คำนวณค่าเฉลี่ยของความชื้นในดินจากทั้งสองเซ็นเซอร์
  if ((moisture02 == 0) || (moisture01 == 0))  // ตรวจสอบว่าค่า moisture ตัวใดตัวหนึ่งเป็น 0 (อาจเกิดจากเซ็นเซอร์ไม่ทำงานหรือแห้ง)
  {
    average = (moisture01 + moisture02);  // ถ้ามีค่า 0 ให้รวมค่าจากทั้งสองเซ็นเซอร์โดยไม่หารด้วย 2 เพื่อให้ได้ค่าที่ไม่เกิดปัญหาการหาร
  } else {
    average = (moisture01 + moisture02) / 2;  // ถ้าทั้งสองเซ็นเซอร์อ่านได้ค่า (มากกว่า 0) ให้นำค่าจากทั้งสองมาคำนวณหาเฉลี่ย
  }

  Blynk.virtualWrite(V2, average);  // ส่งค่าเฉลี่ยความชื้นในดิน (average) ไปยัง virtual pin V2 ในแอป Blynk

  OLED.clearDisplay();             // เคลียร์หน้าจอ OLED เพื่อเตรียมแสดงผลใหม่
  OLED.setTextColor(WHITE);        // กำหนดสีข้อความบน OLED เป็นสีขาว
  OLED.setCursor(0, 0);            // กำหนดตำแหน่งเริ่มต้น (มุมซ้ายบน) สำหรับการแสดงข้อความบน OLED
  OLED.setTextSize(1);             // ตั้งค่าขนาดตัวอักษรเป็น 1 เท่า
  OLED.print("Temp = ");           // พิมพ์ข้อความ "Temp = " ลงบน OLED
  OLED.println(temp);              // แสดงค่าอุณหภูมิที่อ่านได้ลงบน OLED พร้อมขึ้นบรรทัดใหม่
  OLED.setCursor(0, 8);            // เปลี่ยนตำแหน่งสำหรับการแสดงข้อความบรรทัดต่อไป (ตำแหน่ง y = 8)
  OLED.setTextSize(1);             // ยืนยันขนาดข้อความเป็น 1 เท่า
  OLED.print("Humid = ");          // พิมพ์ข้อความ "Humid = " ลงบน OLED
  OLED.println(humid);             // แสดงค่าความชื้นที่อ่านได้ลงบน OLED พร้อมขึ้นบรรทัดใหม่
  OLED.setCursor(0, 17);           // เปลี่ยนตำแหน่งสำหรับข้อความบรรทัดถัดไป (ตำแหน่ง y = 17)
  OLED.setTextSize(1);             // ตั้งค่าขนาดตัวอักษรเป็น 1 เท่าอีกครั้ง
  OLED.print("Soil moisture = ");  // พิมพ์ข้อความ "Soil moisture = " บน OLED
  OLED.println(average);           // แสดงค่าเฉลี่ยของความชื้นในดินที่คำนวณได้
  OLED.display();                  // อัปเดตหน้าจอ OLED ให้แสดงผลข้อความทั้งหมดที่ตั้งค่าไว้

  Blynk.run();  // รันฟังก์ชัน Blynk.run() เพื่อจัดการการสื่อสารกับเซิร์ฟเวอร์ Blynk
  timer.run();  // รัน Timer ของ Blynk เพื่อเรียกใช้งานฟังก์ชันที่ได้ตั้งเวลาไว้

  // อ่านค่าจากเซ็นเซอร์ DHT อีกครั้งในทุก ๆ รอบของ loop
  humid = dht.readHumidity();    // อ่านค่าความชื้นจากเซ็นเซอร์ DHT อีกครั้งแล้วอัพเดตตัวแปร humid
  temp = dht.readTemperature();  // อ่านค่าอุณหภูมิจากเซ็นเซอร์ DHT อีกครั้งแล้วอัพเดตตัวแปร temp

  // พิมพ์ข้อมูลอ่านค่าจากเซ็นเซอร์ต่าง ๆ ลงใน Serial Monitor เพื่อติดตามการทำงาน
  Serial.print("Soil moisture01 = ");
  Serial.println(moisture01);  // แสดงค่าความชื้นจากเซ็นเซอร์ตัวที่ 1
  Serial.print("Soil moisture02 = ");
  Serial.println(moisture02);  // แสดงค่าความชื้นจากเซ็นเซอร์ตัวที่ 2
  Serial.print("Soil moisture average = ");
  Serial.println(average);  // แสดงค่าเฉลี่ยของความชื้นในดิน

  // ตรวจสอบสถานะของสวิตช์ที่อ่านจาก digital pin
  if (dt == 0)  // ถ้าค่าสถานะจาก digital pin เป็น LOW (0)
  {
    digitalWrite(LEDPin, LOW);  // ปิด LED ที่เชื่อมต่ออยู่กับ LEDPin
    led1.off();                 // ส่งคำสั่งปิด LED widget บนแอป Blynk (ที่ virtual pin V1)
  }

  if (dt == 1)  // ถ้าสถานะจาก digital pin เป็น HIGH (1)
  {
    OLED.setTextColor(WHITE);    // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
    OLED.setCursor(0, 25);       // กำหนดตำแหน่งสำหรับการแสดงผลข้อความใหม่บน OLED (ตำแหน่ง y = 25)
    OLED.setTextSize(1);         // กำหนดขนาดตัวอักษรเป็น 1 เท่า
    OLED.print("led");           // แสดงข้อความ "led" บน OLED เพื่อแสดงสถานะว่ามีการเปิด LED
    OLED.display();              // อัปเดตหน้าจอ OLED เพื่อแสดงข้อความที่ตั้งค่าไว้
    digitalWrite(LEDPin, HIGH);  // เปิด LED ที่เชื่อมต่อกับ LEDPin โดยส่งค่า HIGH
    led1.on();                   // ส่งคำสั่งเปิด LED widget บนแอป Blynk (ที่ virtual pin V1)
  }

  // ตรวจสอบอุณหภูมิ หากอุณหภูมิสูงกว่า 32°C ให้เปิดพัดลม
  if (temp > 32)  // ถ้าอุณหภูมิที่อ่านได้สูงกว่า 32 องศาเซลเซียส
  {
    OLED.setTextColor(WHITE);    // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
    OLED.setCursor(80, 25);      // กำหนดตำแหน่งสำหรับข้อความบน OLED (ตำแหน่ง x = 80, y = 25)
    OLED.setTextSize(1);         // ตั้งขนาดตัวอักษรเป็น 1 เท่า
    OLED.print("fan");           // แสดงข้อความ "fan" บน OLED เพื่อบอกสถานะว่าพัดลมกำลังทำงาน
    OLED.display();              // อัปเดตการแสดงผลบน OLED
    led2.on();                   // ส่งคำสั่งเปิด LED widget บนแอป Blynk (ที่ virtual pin V6) เพื่อบ่งบอกสถานะพัดลม
    digitalWrite(FanPin, HIGH);  // เปิดพัดลมโดยส่งค่า HIGH ไปที่พินที่กำหนดให้ควบคุมพัดลม (FanPin)
  }

  // ตรวจสอบอุณหภูมิ หากอุณหภูมิต่ำกว่า 32°C ให้ปิดพัดลม
  if (temp < 32)  // ถ้าอุณหภูมิที่อ่านได้ต่ำกว่า 32 องศาเซลเซียส
  {
    led2.off();                 // ส่งคำสั่งปิด LED widget บนแอป Blynk สำหรับพัดลม
    digitalWrite(FanPin, LOW);  // ปิดพัดลมโดยส่งค่า LOW ไปที่พินที่ควบคุมพัดลม
  }

  // ตรวจสอบความชื้นของดิน หากค่าเฉลี่ยความชื้นน้อยกว่า moistureThreshold (ค่าที่ได้จาก Blynk ผ่าน V0)
  if (average < moistureThreshold)  // ถ้าค่าเฉลี่ยความชื้นในดินต่ำกว่าค่าที่ตั้งไว้ในตัวแปร moistureThreshold
  {
    OLED.setTextColor(WHITE);           // ตั้งค่าสีข้อความบน OLED เป็นสีขาว
    OLED.setCursor(40, 25);             // กำหนดตำแหน่งสำหรับข้อความบน OLED (ตำแหน่ง x = 40, y = 25)
    OLED.setTextSize(1);                // ตั้งขนาดตัวอักษรเป็น 1 เท่า
    OLED.print("pump");                 // แสดงข้อความ "pump" บน OLED เพื่อบอกสถานะว่าปั๊มน้ำกำลังทำงาน
    OLED.display();                     // อัปเดตหน้าจอ OLED ให้แสดงข้อความที่ตั้งค่าไว้
    digitalWrite(WaterPumpnPin, HIGH);  // เปิดปั๊มน้ำโดยส่งค่า HIGH ไปที่พินที่ควบคุมปั๊มน้ำ (WaterPumpnPin)
  } else                                // กรณีที่ค่าเฉลี่ยความชื้นในดินไม่ต่ำกว่า ABC
  {
    digitalWrite(WaterPumpnPin, LOW);  // ปิดปั๊มน้ำโดยส่งค่า LOW ไปที่พินที่ควบคุมปั๊มน้ำ
  }

  delay(2000);  // หน่วงเวลา 2000 มิลลิวินาที (2 วินาที) ก่อนเริ่มรอบ loop ใหม่
}