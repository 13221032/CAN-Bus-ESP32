#include <esp32_can.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5

//output LED lamp&gear dari CAN Bus, input parking sensor dikirim ke CAN Bus
const int lamp_r = 2;
const int lamp_l = 4;
const int gear_p = 5;
const int gear_d = 12;
const int gear_r = 13;
const int interruptPin = 26;

//Variable for Read Parking Sensor
volatile unsigned long lastRiseTime = 0; // Time of the last rising edge
volatile unsigned long pulseWidth = 0; // Duration of the pulse
volatile bool pulseDetected = false; // Flag to indicate a pulse was detected
volatile bool measuringPulse = false; // Flag to indicate that we are currently measuring a pulse
volatile uint32_t packet = 0; // 32-bit packet to store received bits
volatile int bitCount = 0; // Index to track the current bit in the packet
byte A;
byte B;
byte C;
byte D;

void IRAM_ATTR handleFalling();
void IRAM_ATTR handleRising() {
  if (!measuringPulse) {
    lastRiseTime = micros(); // Start measuring pulse width
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleFalling, FALLING); // Switch to falling edge
    measuringPulse = true; // Start measuring
  }
}

void IRAM_ATTR handleFalling() {
  unsigned long currentTime = micros(); // Get the current time in microseconds
  pulseWidth = currentTime - lastRiseTime; // Calculate pulse width while the signal was high
  measuringPulse = false; // Stop measuring

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleRising, RISING); // Switch back to rising edge

  pulseDetected = true; // Set flag to indicate a pulse was detected
}

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

void blinking(int pin) {
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
  delay(250);
}

void setup() {
  Serial.begin(500000);
  // Initialize toggle switch pin
  pinMode(lamp_r, OUTPUT);
  pinMode(lamp_l, OUTPUT);
  pinMode(gear_p, OUTPUT);
  pinMode(gear_d, OUTPUT);
  pinMode(gear_r, OUTPUT);

  CAN0.setCANPins(GPIO_NUM_22, GPIO_NUM_21); //rx, tx

  CAN0.begin(500000);

  CAN0.watchFor();

  pinMode(interruptPin, INPUT); // Set interrupt pin as input
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleRising, RISING); // Attach interrupt on rising edge

  esp_task_wdt_init(WDT_TIMEOUT, true); // Initialize ESP32 Task WDT
  esp_task_wdt_add(NULL);               // Subscribe to the Task WDT
}

void loop() {
  CAN_FRAME message_lamp;
  CAN_FRAME message_gear;
  CAN_FRAME message_sensor;
  //READ PARKING SENSOR
  // Check if a pulse was detected
  if (pulseDetected) {
    
    // Serial.println(pulseWidth);
    if (pulseWidth > 50 && pulseWidth < 300) {
      if (bitCount == 0) {
        bitCount++;
      }
      else{
        if (pulseWidth > 150) {
        packet = (packet << 1) | 1; // Shift left and add a 1
        bitCount++;
        } else if (pulseWidth <= 150) {
        packet = (packet << 1); // Shift left and add a 0
        bitCount++;
        }
      }
    }

    // If we have received 32 bits, process the packet
    if (bitCount == 33) {
      A = packet >> 24;        // Extract byte A (bits 0-7)
      D = (packet >> 16) & 255; // Extract byte D (bits 8-15)
      C = (packet >> 8) & 255;  // Extract byte C (bits 16-23)
      B = (packet & 0xFF);         // Extract byte B (bits 24-31)

      if (A != 255 && B != 255 && C != 255 && D != 255) {
        Serial.print("A (cm): ");
        Serial.println(A * 10); // Print byte A in cm
        Serial.print("B (cm): ");
        Serial.println(B * 10); // Print byte B in cm
        Serial.print("C (cm): ");
        Serial.println(C * 10); // Print byte C in cm
        Serial.print("D (cm): ");
        Serial.println(D * 10); // Print byte D in cm
      }
    
      bitCount = 0; // Reset bit index for next packet
      packet = 0; // Clear the packet for the next data
    }

    pulseDetected = false; // Reset pulse detected flag
  }

  //READ CAN BUS LAMP
  if(CAN0.read(message_lamp) && message_lamp.id == 0x160){
    printFrame(&message_lamp);
    int lamp = message_lamp.data.uint8[0];
    Serial.print("Lamp: ");
    Serial.println(lamp);
    if (lamp == 'R'){
      blinking(lamp_r);
      digitalWrite(lamp_l, LOW);
    }
    else if(lamp == 'L'){
      blinking(lamp_l);
      digitalWrite(lamp_r, LOW);
    }
    else if (lamp == 0){
      digitalWrite(lamp_l, LOW);
      digitalWrite(lamp_r, LOW);
    }
  } 

  //READ CAN BUS GEAR
  if(CAN0.read(message_gear) && message_gear.id == 0x180){
    printFrame(&message_gear);
    int gear = message_gear.data.uint8[0];
    Serial.print("Gear: ");
    Serial.println(gear);
    if (gear == 'P'){
      digitalWrite(gear_p, HIGH);
      digitalWrite(gear_d, LOW);
      digitalWrite(gear_r, LOW);
    }
    else if(gear == 'D'){
      digitalWrite(gear_p, LOW);
      digitalWrite(gear_d, HIGH);
      digitalWrite(gear_r, LOW);
    }
    else if (gear == 'R'){
      digitalWrite(gear_p, LOW);
      digitalWrite(gear_d, LOW);
      digitalWrite(gear_r, HIGH);
      message_sensor.rtr = 0;
      message_sensor.id = 0x170;
      message_sensor.extended = false;
      message_sensor.length = 4;
      message_sensor.data.uint8[0]= A;
      message_sensor.data.uint8[1]= B;
      message_sensor.data.uint8[2]= C;
      message_sensor.data.uint8[3]= D;
      CAN0.sendFrame(message_sensor);
    }
  } 
  esp_task_wdt_reset();
}
