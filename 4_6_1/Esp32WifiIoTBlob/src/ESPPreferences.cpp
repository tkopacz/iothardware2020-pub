#include "ESPPreferences.h"

ESPPreferences::ESPPreferences():ESPPreferences(true,2048,125) {
    
}

ESPPreferences::ESPPreferences(bool read):ESPPreferences(read,2048,125) {
    
}

ESPPreferences::ESPPreferences(bool read,size_t size,uint8_t version) {
    if (size<8) size = 8;
    m_size = size;
    m_version = version;
    m_lastIndex = m_lastByteIndex = 0;
    if (read==true) readAll();
}
ESPPreferences::~ESPPreferences() {

}

bool ESPPreferences::readAll() {
    m_lastIndex = m_lastByteIndex = 0;
    //Check Version
    EEPROM.begin(m_size);
    int val1 = ((int)EEPROM.read(0));
    int val2 = ((int)EEPROM.read(m_size-1-0));
    //Serial.printf("readAll, %d, %d\r\n",val1,val2);
    if (val1==m_version && val2==m_version) {
        m_lastByteIndex = 4;
        int datacnt = ((int)EEPROM.read(m_lastByteIndex++));
        //Serial.println(datacnt);
        for (byte i = 0; i < datacnt; i++)
        {
            String key,value;
            //Reading key
            while(m_lastByteIndex < (m_size - 1)){
                char b = EEPROM.read(m_lastByteIndex++);
                if (b==0) break;
                key+=b;
            }
            //Reading value
            while(m_lastByteIndex < (m_size - 1)){
                char b = EEPROM.read(m_lastByteIndex++);
                if (b==0) break;
                value+=b;
            }
            m_values[m_lastIndex] = value;
            m_keys[m_lastIndex] = key;
            m_lastIndex++;
        }
    }
    EEPROM.end();
    return true;
}

bool ESPPreferences::WriteAll() {
    m_lastByteIndex = 0;
    //Version
    EEPROM.begin(m_size);
    EEPROM.write(0,m_version);
    EEPROM.write(m_size-1,m_version);
    m_lastByteIndex = 4;
    EEPROM.write(m_lastByteIndex++,m_lastIndex);
    for (int8_t i = 0; i < m_lastIndex; i++)
    {
        if (m_lastByteIndex>=(m_size-2)) break;
        int l = m_keys[i].length();
        for (int j = 0; j < l; j++)
        {
            if (m_lastByteIndex>=(m_size-2)) break;
            char b = m_keys[i][j];
            EEPROM.write(m_lastByteIndex++,b);
            //Serial.print(b);
        }
        EEPROM.write(m_lastByteIndex++,0);
        l = m_values[i].length();
        for (int j = 0; j < l; j++)
        {
            if (m_lastByteIndex>=(m_size-2)) break;
            char b = m_values[i][j];
            EEPROM.write(m_lastByteIndex++,b);
            //Serial.print(b);
        }
        if (m_lastByteIndex>=(m_size-1))  break; //Not mistake - last "zero"
        EEPROM.write(m_lastByteIndex++,0);
    }
    EEPROM.end();
    return true;
}

String ESPPreferences::GetValue(String key) {
    //Serial.printf("GetValue, %s\r\n",key.c_str());
    for (uint8_t i = 0; i < m_lastIndex; i++)
    {
        if (m_keys[i].compareTo(key)==0) {
            return m_values[i];
        }
    }
    return String("");
}
bool ESPPreferences::SetValue(String key,String value) {
    //Serial.printf("SetValue, %s = %s\r\n",key.c_str(),value.c_str());
    for (uint8_t i = 0; i < m_lastIndex; i++)
    {
        //Serial.println(m_keys[i]);
        if (m_keys[i].compareTo(key)==0) {
            m_values[i] = value;
            //writeAll();
            return true;
        }
    }
    if (m_lastIndex>=m_cnt) return false;
    m_values[m_lastIndex] = value;
    m_keys[m_lastIndex] = key;
    m_lastIndex++;
    //No WriteAll - EEPROM
    return true;
}
void ESPPreferences::Clear() {
    for (uint8_t i = 0; i < m_lastIndex; i++)
    {
        m_keys[i].clear();
        m_values[i].clear();
    }
    m_lastIndex=0;
    WriteAll();
}
