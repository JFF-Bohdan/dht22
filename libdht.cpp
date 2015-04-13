/*
  Library for DHT11 and DHT22/RHT03/AM2302
  
  Based on https://github.com/MORA99/Stokerbot/tree/master/Libraries/dht
  Released into the public domain - http://unlicense.org
  
  Modified by JFF-Bohdan
  https://github.com/JFF-Bohdan/DHT22
*/

#include "libdht.h"

void dht::initPin(unsigned char pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);  
}  

int8_t dht::readRawData(unsigned char pin, int16_t *temperature, int16_t *humidity, boolean dht22) {
    uint8_t     bits[5];
    uint8_t     i = 0,j = 0;

    memset(bits, 0, sizeof(bits));

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    if (dht22)
        delayMicroseconds(1200);
    else
        delay(18);
        
    noInterrupts();

    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);

    //check start condition 1
    if(digitalRead(pin)==HIGH) {
        interrupts();
        return -1;
    }
    
    delayMicroseconds(80);
    //check start condition 2
    if(digitalRead(pin)==LOW) {
        interrupts();
        return -2;    
    }

    //Sensor init ok, now read 5 bytes ...
    for (j=0; j<5; j++)
    {
        for (int8_t i=7; i>=0; i--)
        {
            if (pulseIn(pin, HIGH, 1000) > 30)
                bitSet(bits[j], i);
        }
    }

    //reset port
    pinMode(pin, INPUT);

    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
        //return temperature and humidity
        if (dht22) {
            *humidity = bits[0]<<8 | bits[1];
            *temperature = bits[2]<<8 | bits[3];
        } else {
            *humidity = bits[0];
            *temperature = bits[2];
        }
        
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        interrupts();
        return 0;
    }

    interrupts();
    return -5;
}

int8_t dht::readFloatData(unsigned char pin, float *temperature, float *humidity, boolean dht22)
{
    int16_t rawtemperature, rawhumidity;
    int8_t res = dht::readRawData(pin, &rawtemperature, &rawhumidity, dht22);
    if (res != 0)
    {
        return res;
    }
    if (dht22) {
        *temperature = dht::convertTemperature(rawtemperature);
        *humidity = dht::convertHumidity(rawhumidity);
    } else {
        *temperature = (float)rawtemperature;
        *humidity = (float)rawhumidity;
    }
    return 0;
}

//dht22 only, since dht11 does not have decimals
float dht::convertTemperature(int16_t rawtemperature)
{
    if(rawtemperature & 0x8000) {
        return (float)((rawtemperature & 0x7FFF) / 10.0) * -1.0;
    } else {
        return (float)rawtemperature/10.0;
    }
}

float dht::convertHumidity(int16_t rawhumidity)
{
    return (float)rawhumidity/10.0;
}

DhtBuffered::DhtBuffered(const unsigned char pin, const boolean dht22):
    m_Pin(pin),
    m_Temperature(0),
    m_Humidity(0),
    m_LastMillis(0),
    m_HaveData(false),
    m_IsDht22(dht22)
{
    //
}

DhtBuffered::~DhtBuffered()
{
    //
}

void DhtBuffered::clear()
{
    m_Temperature = 0;
    m_Humidity = 0;
    m_LastMillis = 0;
    m_HaveData = false;
}

boolean DhtBuffered::haveData() const
{
    return m_HaveData;
}

boolean DhtBuffered::update(boolean *realReadingWasMade)
{
    if(!needReloadData()) {
        if(realReadingWasMade)
            *realReadingWasMade = false;

        return true;
    }

    return readData(realReadingWasMade);
}

int16_t DhtBuffered::temperature(const boolean onlyFromCache) const
{
    if( (onlyFromCache) && (m_HaveData) )
        return m_Temperature;

    if( (!m_HaveData) || (needReloadData()) ) {
        if(!readData()) {

            //if we have old data - let's return it
            if(m_HaveData)
                return m_Temperature;

            return -100;
        }
    }

    return m_Temperature; // return last correct measurement
}

int16_t DhtBuffered::humidity(const boolean onlyFromCache) const
{
    if( (onlyFromCache) && (m_HaveData) )
        return m_Humidity;

    if( (!m_HaveData) || (needReloadData()) ) {
        if(!readData()) {

            //if we have old data - let's return it
            if(m_HaveData)
                return m_Humidity;

            return -100;
        }
    }

    return m_Humidity; // return last correct measurement
}

boolean DhtBuffered::needReloadData() const
{
    return (!m_HaveData) || (millis() >= m_LastMillis);
}

boolean DhtBuffered::readData(boolean *realReadingWasMade) const
{  
    if(realReadingWasMade)
        *realReadingWasMade = false;

    if(!needReloadData())
        return true;

    pinMode(m_Pin, OUTPUT);
    digitalWrite(m_Pin, HIGH);
    
    if(dht::readRawData(m_Pin, &m_Temperature, &m_Humidity, m_IsDht22) != 0)
        return false;

    m_LastMillis = millis() + DHT_REFRESH_TIMEOUT;

    if(!m_HaveData)
        m_HaveData = true;

    if(realReadingWasMade)
        *realReadingWasMade = true;

    return true;
}


DhtBufferedFloat::DhtBufferedFloat(const unsigned char pin, const boolean dht22):
    DhtBuffered(pin, dht22)
{
    //
}

DhtBufferedFloat::~DhtBufferedFloat()
{
    //
}

boolean DhtBufferedFloat::update(boolean *realReadingWasMade)
{
    return DhtBuffered::update(realReadingWasMade);
}

float DhtBufferedFloat::temperature(const boolean onlyFromCache) const
{
    return dht::convertTemperature(DhtBuffered::temperature(onlyFromCache));
}

float DhtBufferedFloat::humidity(const boolean onlyFromCache) const
{
    return dht::convertHumidity(DhtBuffered::humidity(onlyFromCache));
}

boolean DhtBufferedFloat::haveData() const
{
    return DhtBuffered::haveData();
}
