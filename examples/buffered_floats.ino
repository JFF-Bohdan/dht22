#include "libdht.h"

#define          DHT_INPUT_PIN P2_3
DhtBufferedFloat mydht(DHT_INPUT_PIN, true);

void setup()
{
  Serial.begin(9600);  
  Serial.println("READY");
  dht::initPin(DHT_INPUT_PIN);
}

void loop()
{ 
  boolean  actualReadingMade = false;  
  Serial.print("Reading data...");
  
  if(mydht.update(&actualReadingMade)) {
      Serial.println("[  OK  ]");   
      Serial.print("Fresh data: ");
      
      if(actualReadingMade) {
        Serial.println("yes");
      } else {
        Serial.println("no");   
      }
      
      Serial.print("Result:");
      Serial.print("T: ");
      Serial.print(mydht.temperature());
      Serial.print(" H: ");
      Serial.println(mydht.humidity());   
  } else {
      Serial.println("[FAILED]");    
  }
  
  delay(500);   
}
