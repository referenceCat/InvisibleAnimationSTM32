#include <stdint.h>
#include <stdio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <stdlib.h>
#include <libopencm3/stm32/dma.h>

#ifndef SRC_DISPLAY_H
    #define SRC_DISPLAY_H

#define OLED_ADDRESS 0x3C
#define I2C_COMMAND 0x00
#define I2C_DATA 0x40

#define _IF_SB(i2c) ((I2C_SR1(i2c) & I2C_SR1_SB) == 0)
#define _IF_BTF(i2c) ((I2C_SR1(i2c) & I2C_SR1_BTF) == 0)
#define _IF_ADDR(i2c) ((I2C_SR1(i2c) & I2C_SR1_ADDR) == 0)
#define _IF_TxE(i2c) (I2C_SR1(i2c) & I2C_SR1_TxE) == 0
#define IS_SET(a, b)  (((a) & (b)) != 0)
#define NOT_SET(a, b) (!IS_SET(a, b))
#define BUFFERLENGTH 1024
#define DATAONLY (uint8_t)0b01000000
#define COMMAND (uint8_t)0b00000000

// Fundamental Command Table
#define SSD1306_SET_CONTROL     0x81  // Double byte command to select 1 out of 256 contrast steps. Contrast increases as the value increases.
#define SSD1306_RESET           0x7F
#define SSD1306_DISPLAY_ON_RAM  0xA4  // Resume to RAM content display (RESET)
#define SSD1306_DISPLAY_NO_RAM  0xA5  // Output ignores RAM content
#define SSD1306_SET_NORMAL      0xA6  // Normal display (RESET)
#define SSD1306_SET_INVERSE     0xA7  // Inverse display
#define SSD1306_SET_DISPLAY_OFF 0xAE  // Display OFF (sleep mode)
#define SSD1306_SET_DISPLAY_ON  0xAF  // Display ON in normal mode

typedef enum SSD1306_AddressingMode {
  Horizontal  = 0b00,
  Vertical    = 0b01,
  Page        = 0b10, // RESET
  INVALID     = 0b11  // You MUST NOT USE IT
} MODE;

void i2c_init(void);
void i2c_deinit(void);
uint8_t i2c_check(uint32_t i2c, uint8_t address);
void ssd1306_start(void);
void ssd1306_stop(void);
void ssd1306_send(uint8_t spec);
void ssd1306_send_data(uint8_t spec, uint8_t data);
void ssd1306_init(uint8_t width, uint8_t height);
void ssd1306_setMemoryAddressingMode(MODE mode);
void ssd1306_setColumnAddressScope(uint8_t lower, uint8_t upper);
void ssd1306_setPageAddressScope(uint8_t lower, uint8_t upper);
void ssd1306_setPageStartAddressForPageAddressingMode(uint8_t pageNum);
void ssd1306_setDisplayStartLine(uint8_t startLine);
void ssd1306_setContrast(uint8_t value);
void ssd1306_setPrecharge(uint8_t value);
void ssd1306_setDisplayOn(bool resume);
void ssd1306_setInverse(bool inverse);
void ssd1306_switchOLEDOn(bool goOn);
void ssd1306_chargePump(bool chargeOn);
void ssd1306_setDisplayOffset(uint8_t verticalShift);
void ssd1306_adjustVcomDeselectLevel(uint8_t value);
void ssd1306_setOscillatorFrequency(uint8_t value);
void ssd1306_setMultiplexRatio(uint8_t ratio);
void ssd1306_setCOMPinsHardwareConfiguration(uint8_t val);
void ssd1306_setPage(uint8_t page);
void ssd1306_setColumn(uint8_t column);
void ssd1306_clear(int screenBufferSize);
void ssd1306_refresh(uint8_t *screenBuffer, int screenBufferSize);

#endif