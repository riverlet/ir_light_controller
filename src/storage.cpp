//
// Created by riverlet on 2017/9/4.
//

#include "storage.h"
#include "EEPROM.h"
void Storage::begin() {
    EEPROM.begin();
    readAll();
}

bool Storage::addToTemp(decode_results *results) {
    bool added = false;
    for(int i = 0; i< KEY_VALUES; i++){
        if(results->value == tempValues[i] || results->overflow || results->value == 0 || results->value == 0xFFFFFFFF) {
            break;
        } else {
            if(tempValues[i] == 0) {
                tempValues[i] = results->value;

#ifdef DOG
                Serial.print("tempValues :");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(results->value);
#endif
                added = true;
                break;
            }
        }
    }
    return added;
}

void Storage::clearTemp() {
    memset(&tempValues[0], 0, sizeof(tempValues));
}

void Storage::saveTempAs(byte key) {
    int address_start = key * KEY_VALUES; //每个KEY可能有KEY_VALUES个值（最多有KEY_VALUES个遥控器按键对应一个功能）
    for (int i = 0; i < KEY_VALUES; i++) {
        writeLong(address_start + i, tempValues[i]);
#ifdef DOG
        Serial.print("saveTempAs :");
        Serial.print("key: ");
        Serial.print(key);
        Serial.print("addr: ");
        Serial.print(address_start + i);
        Serial.print(": ");
        Serial.println(tempValues[i]);
#endif
    }
    delay(100);
    readAll();
}

byte Storage::getKey(decode_results *results) {
#ifdef DOG
    Serial.print("getKey :");
    Serial.print(results->value);
    Serial.print(": ");
#endif
    for(int i = 0; i < KEYS; i++) {
        for (int j = 0; j < KEY_VALUES ; j++) {
            if(storageValues[i][j] == results->value) {
#ifdef DOG
                Serial.println(i);
#endif
                return i;
            }
        }
    }
    return -1;
}

void Storage::readAll() {
    for(int i = 0; i < KEYS; i++) {
        for (int j = 0; j < KEY_VALUES ; j++) {
            storageValues[i][j] = readLong(i*KEY_VALUES + j);
#ifdef DOG
            Serial.print("readAll, storageValues :");
            Serial.print(i);
            Serial.print(",");
            Serial.print(j);
            Serial.print(": ");
            Serial.println(storageValues[i][j] );
#endif
        }
    }
}

void Storage::writeLong(int address, unsigned long value)
{
    //Decomposition from a long to 4 bytes by using bitshift.
    //One = Most significant -> Four = Least significant byte
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);

    //Write the 4 bytes into the eeprom memory.
    address *= 4;
    EEPROM.write(address, four);
    EEPROM.write(address + 1, three);
    EEPROM.write(address + 2, two);
    EEPROM.write(address + 3, one);
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
unsigned long Storage::readLong(int address)
{
    address *= 4;
    //Read the 4 bytes from the eeprom memory.
    long four = EEPROM.read(address);
    long three = EEPROM.read(address + 1);
    long two = EEPROM.read(address + 2);
    long one = EEPROM.read(address + 3);

    //Return the recomposed long by using bitshift.
    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
