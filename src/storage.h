//
// Created by riverlet on 2017/9/4.
//

#ifndef IR_RECEIVER_STORAGE_H
#define IR_RECEIVER_STORAGE_H


#include "../.piolibdeps/IRremote_ID4/IRremote.h"

#define KEY_VALUES 8
#define KEYS 3

class Storage {
public:
    void begin();
    bool addToTemp(decode_results *results);
    void saveTempAs(byte key);
    void clearTemp( );
    byte getKey(decode_results * results);

private:
    unsigned long tempValues[KEY_VALUES];
    unsigned long storageValues[KEYS][KEY_VALUES];
    /**
     * 向EEPROM写入1个LONG值
     * @param address long 值地址（每个long值占4个byte地址）
     * @param value
     */
    void writeLong(int address, unsigned long value);
    unsigned long readLong(int address);
    void readAll();

};


#endif //IR_RECEIVER_STORAGE_H
