const int interruptPin = 26; // Pin number for the interrupt
volatile unsigned long lastRiseTime = 0; // Time of the last rising edge
volatile unsigned long pulseWidth = 0; // Duration of the pulse
volatile bool pulseDetected = false; // Flag to indicate a pulse was detected
volatile bool measuringPulse = false; // Flag to indicate that we are currently measuring a pulse
volatile uint32_t packet = 0; // 32-bit packet to store received bits
volatile int bitCount = 0; // Index to track the current bit in the packet

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

void setup() {
  Serial.begin(500000); // Start serial communication for debugging
  pinMode(interruptPin, INPUT); // Set interrupt pin as input
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleRising, RISING); // Attach interrupt on rising edge
}

void loop() {
  // Check if a pulse was detected
  if (pulseDetected) {
    // Serial.println(pulseWidth);
    if (pulseWidth > 50 && pulseWidth <300) {
      if (bitCount == 0) {
        bitCount++;
      }
      else{
        if (pulseWidth > 150) {
        packet = (packet << 1) | 1; // Shift left and add a 1
        bitCount++;
        } else if (pulseWidth < 150) {
        packet = (packet << 1); // Shift left and add a 0
        bitCount++;
        }
      }
    }

      // If we have received 32 bits, print the packet
    if (bitCount == 33) {
      byte A = packet >> 24 ;          // Extract byte A (bits 0-7)
      byte D = (packet >> 16) & 255;   // Extract byte D (bits 8-15)
      byte C = (packet >> 8) & 255;  // Extract byte C (bits 16-23)
      byte B = (packet & 0xFF);  // Extract byte B (bits 24-31)

      if( A!=255 && B!= 255 && C!= 255 && D!= 255){
        Serial.print("A (cm): ");
        Serial.println(A*10); // Print byte A in binary format
        Serial.print("B (cm): ");
        Serial.println(B*10); // Print byte B in binary format
        Serial.print("C (cm): ");
        Serial.println(C*10); // Print byte C in binary format
        Serial.print("D (cm): ");
        Serial.println(D*10); // Print byte D in binary format
      }
      bitCount = 0; // Reset bit index for next packet
      packet = 0; // Clear the packet for the next data
    }

    pulseDetected = false; // Reset pulse detected flag
  }
}
