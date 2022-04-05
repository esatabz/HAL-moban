#ifndef __I2C_H
#define __I2C_H

#include "main.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);

#endif


void Iic_Eeprom_Write(uint8_t *string, uint16_t add, uint16_t num);

void Iic_Eeprom_Read(uint8_t *string, uint16_t add, uint16_t num);

void Iic_Res_Write(uint8_t res);

uint8_t Iic_Res_Read(void);
