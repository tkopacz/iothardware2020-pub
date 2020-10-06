#pragma once
#include <Arduino.h>
#include <EEPROM.h> //The EEPROM libray 

class ESPPreferences
{
    private:
        static const uint8_t m_cnt=10;
        uint8_t m_version;
        int m_size;
        int m_lastIndex;
        int m_lastByteIndex;
        String m_keys[m_cnt];
        String m_values[m_cnt];
        bool readAll();
    public:
        ESPPreferences();
        ESPPreferences(bool read);
        ESPPreferences(bool read,size_t size,uint8_t version);
        ~ESPPreferences();
        String GetValue(String key);
        bool SetValue(String key,String value);
        void Clear();
        bool WriteAll();

};
