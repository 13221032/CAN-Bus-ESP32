//Input toggle switch 1 & 2 dikirim ke CAN Bus, menerima data parking sensor dari CAN Bus
#include <esp32_can.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5

const int lamp_r = 2;
const int lamp_l = 4;
const int gear_1 = 5;
const int gear_2 = 13;
const int oled = 23;
int A = 255;
int B = 255;
int C = 255;
int D = 255;
byte currentLamp = 0;
byte currentGear = 0;

#define SDA_PIN 18  // Replace with your desired SDA pin
#define SCL_PIN 19  // Replace with your desired SCL pin

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void printFrame(CAN_FRAME *message)
{
  Serial.print(message->id, HEX);
  if (message->extended) Serial.print(" X ");
  else Serial.print(" S ");   
  Serial.print(message->length, DEC);
  for (int i = 0; i < message->length; i++) {
    Serial.print(message->data.byte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void drawLeftArrow(int x, int y) {
  display.drawLine(x, y, x + 10, y - 5, SSD1306_WHITE);
  display.drawLine(x, y, x + 10, y + 5, SSD1306_WHITE);
  display.drawLine(x + 10, y - 5, x + 10, y + 5, SSD1306_WHITE);
}

void drawRightArrow(int x, int y) {
  display.drawLine(x, y, x - 10, y - 5, SSD1306_WHITE);
  display.drawLine(x, y, x - 10, y + 5, SSD1306_WHITE);
  display.drawLine(x - 10, y - 5, x - 10, y + 5, SSD1306_WHITE);
}

void setup() {
  Serial.begin(115200);
  // Initialize toggle switch pin
  pinMode(lamp_r, INPUT_PULLDOWN);
  pinMode(lamp_l, INPUT_PULLDOWN);
  pinMode(gear_1, INPUT_PULLDOWN);
  pinMode(gear_2, INPUT_PULLDOWN);

  Wire.begin(SDA_PIN, SCL_PIN);

  CAN0.setCANPins(GPIO_NUM_22, GPIO_NUM_21); //rx, tx

  CAN0.begin(500000);

  CAN0.watchFor(0x170); //ID Parking Sensor Data

    // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);      // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color
  display.display();

  esp_task_wdt_init(WDT_TIMEOUT, true); // Initialize ESP32 Task WDT
  esp_task_wdt_add(NULL);               // Subscribe to the Task WDT
}

void loop() {
  CAN_FRAME can_message_lamp;
  CAN_FRAME can_message_gear;
  CAN_FRAME message_sensor;

    // Receive message and store it
  if(CAN0.read(message_sensor)){
    printFrame(&message_sensor);
    A = message_sensor.data.uint8[0];
    B = message_sensor.data.uint8[1];
    C = message_sensor.data.uint8[2];
    D = message_sensor.data.uint8[3];
    Serial.print("A: ");
    Serial.print(A);
    Serial.print(",  B: ");
    Serial.print(B);
    Serial.print(",  C: ");
    Serial.print(C);
    Serial.print(",  D: ");
    Serial.println(D);
    delay(100);
  }
  
  if(digitalRead(lamp_r) == 0 && digitalRead(lamp_l) == 1){
    currentLamp ='L';
  }
  else if(digitalRead(lamp_r) == 1 && digitalRead(lamp_l) == 0){
    currentLamp = 'R';
  }
  else{
    currentLamp = 0;
  }

  Serial.print("Lamp = ");
  Serial.println((char)currentLamp);

  if(digitalRead(gear_1) == 1 && digitalRead(gear_2) == 0){
    currentGear = 'P';
    display.clearDisplay();
    display.setCursor (0, 0);
    display.print("P");

    if (currentLamp == 'L') {
      drawLeftArrow(30, 50);  // Position the left arrow
    } else if (currentLamp == 'R') {
      drawRightArrow(100, 50);  // Position the right arrow
    }
    display.display();
    
  }
  else if(digitalRead(gear_1) == 0 && digitalRead(gear_2) == 1){
    currentGear ='R';
    
    display.clearDisplay(); 

    display.setCursor (110, 0);
    display.print("R");
  
    // First row: A = ...    B = ...
    display.setCursor(0, 16);
    display.print("A = ");
    display.print(A);
    display.setCursor(64, 16);
    display.print("B = ");
    display.print(B);

    // Second row: C = ...    D = ...
    display.setCursor(0, 32);
    display.print("C = ");
    display.print(C); // Replace with actual value for C
    display.setCursor(64, 32);
    display.print("D = ");
    display.print(D); // Replace with actual value for D

    if (currentLamp == 'L') {
      drawLeftArrow(30,50);  // Position the left arrow
    } else if (currentLamp == 'R') {
      drawRightArrow(100,50);  // Position the right arrow
    }

    display.display();
  }
  else {
    currentGear = 'D';
    display.clearDisplay();
    display.setCursor (64, 0);
    display.print("D");

    if (currentLamp == 'L') {
      drawLeftArrow(30,50);  // Position the left arrow
    } else if (currentLamp == 'R') {
      drawRightArrow(100,50);  // Position the right arrow
    }

    display.display();
  }

  Serial.print("Gear = ");
  Serial.println((char)currentGear);

  //Sending message
  can_message_lamp.rtr = 0;
  can_message_lamp.id = 0x160;
  can_message_lamp.extended = false;
  can_message_lamp.length = 1;
  can_message_lamp.data.uint8[0]= currentLamp;
  CAN0.sendFrame(can_message_lamp);
  delay(100);

  can_message_gear.rtr = 0;
  can_message_gear.id = 0x180;
  can_message_gear.extended = false;
  can_message_gear.length = 1;
  can_message_gear.data.uint8[0]= currentGear;
  CAN0.sendFrame(can_message_gear);
  delay(100);

  esp_task_wdt_reset();
}