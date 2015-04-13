#include "libdht.h"
#define      DHT_INPUT_PIN P2_3

void setup()
{
  Serial.begin(9600);  
  Serial.println("READY");
  dht::initPin(DHT_INPUT_PIN);
}

void printDec(uint32_t ui) 
{  
  Serial.print(ui/10, DEC);
  Serial.print(".");
  Serial.print(ui%10, DEC);
}

void loop()
{ 
  float    temperature = 0.0;
  float    humidity = 0.0;
  
  Serial.print("Reading data...");
  if(!dht::readFloatData(DHT_INPUT_PIN, &temperature, &humidity, true)) {
      Serial.println("[  OK  ]");   
      Serial.print(millis());
      Serial.print(" result:");
      Serial.print("T: ");
      Serial.print(temperature);
      Serial.print(" H: ");
      Serial.print(humidity);   
      Serial.println();
  } else {
      Serial.println("[FAILED]");    
  }
  
  delay(500);   
}

