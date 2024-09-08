#include <esp32_can.h>

void printFrame(CAN_FRAME *message)
{
  Serial.print(message->id, HEX);
  if (message->extended) Serial.print(" X ");
  else Serial.print(" S ");   
  Serial.print(message->length, DEC);
  Serial.print(" ");
  for (int i = 0; i < message->length; i++) {
    Serial.print(message->data.byte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(500000);

  CAN0.setCANPins(GPIO_NUM_22, GPIO_NUM_21); //rx, tx

  CAN0.begin(500000);
  CAN0.watchFor();
}

void loop() {
  CAN_FRAME message;
  byte i = 0;
  if(CAN0.read(message)){
    
    printFrame(&message);

    // Send out a return message for each one received
    // Simply increment message id and data bytes to show proper transmission
    // Note: this will double the traffic on the network (provided it passes the filter above)
    message.id++;
    for (i = 0; i < message.length; i++) {
      message.data.uint8[i]++;
    }
  }
}
