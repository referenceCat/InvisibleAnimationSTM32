#include "display.h"

void i2c_init(void) {
  dma_channel_reset(DMA1, DMA_CHANNEL6);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL6);
  dma_set_read_from_memory(DMA1, DMA_CHANNEL6);
  dma_disable_transfer_error_interrupt(DMA1, DMA_CHANNEL6);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL6, (uint32_t)&I2C1_DR);

  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO8);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO9);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ, AFIO_MAPR_I2C1_REMAP);

  i2c_peripheral_disable(I2C1);
  i2c_enable_dma(I2C1);
  i2c_set_own_7bit_slave_address(I2C1, 0x00);
  i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
  i2c_peripheral_enable(I2C1);
}

void i2c_deinit(void)
{
  i2c_send_stop(I2C1);

  i2c_reset(I2C1);
  i2c_peripheral_disable(I2C1); /* disable i2c during setup */
}

uint8_t i2c_check(uint32_t i2c, uint8_t address)
{
  i2c_send_start(i2c);
  // i2c_enable_ack(i2c);
  int timeout = 20000;

  /* Wait for master mode selected */
  while (!((I2C_SR1(i2c) & I2C_SR1_SB)))
  { //  & (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))
    // if (timeout > 0) {
    // 	timeout--;
    // } else {
    // 	return 0;
    // }
  }

  i2c_send_7bit_address(i2c, address, I2C_WRITE);

  timeout = 20000;
  /* Waiting for address is transferred. */
  while (!(I2C_SR1(i2c) & I2C_SR1_ADDR))
  {
    if (timeout > 0)
    {
      timeout--;
    }
    else
    {
      return 0;
    }
  }

  /* Cleaning ADDR condition sequence. */
  uint32_t reg32 = I2C_SR2(i2c);
  (void)reg32; /* unused */

  i2c_send_stop(i2c);
  return 1;
}

uint32_t reg32 __attribute__((unused));

void ssd1306_start(void) {
  i2c_send_start(I2C1);
  while (_IF_SB(I2C1));
  i2c_send_7bit_address(I2C1, OLED_ADDRESS, I2C_WRITE);
  while (_IF_ADDR(I2C1));
  /* Cleaning ADDR condition sequence. */
  reg32 = I2C_SR2(I2C1);
}

void ssd1306_stop(void) {
  i2c_send_stop(I2C1);
  while (_IF_BTF(I2C1));
}

void ssd1306_send(uint8_t spec) {
  i2c_send_data(I2C1, spec);
  while (_IF_TxE(I2C1));
}

void ssd1306_send_data(uint8_t spec, uint8_t data) {
  ssd1306_start();
  ssd1306_send(spec);
  ssd1306_send(data);
  ssd1306_stop();
}

void ssd1306_init(uint8_t width, uint8_t height) {
  // now we can and should send a lot commands
  ssd1306_switchOLEDOn(false); // 0xae
  ssd1306_setOscillatorFrequency(0x80);  // D5h 0x80
  ssd1306_setMultiplexRatio(height-1);
  ssd1306_setInverse(false); // normal display
  ssd1306_chargePump(true);
  ssd1306_setContrast(0x3F);
  ssd1306_setPrecharge(0x22);
  ssd1306_setCOMPinsHardwareConfiguration(0x02);
  ssd1306_adjustVcomDeselectLevel(0x20);
  ssd1306_setDisplayOn(true);
  ssd1306_switchOLEDOn(true);
  ssd1306_setMemoryAddressingMode(Horizontal);
  ssd1306_setColumnAddressScope(0,width-1);
  ssd1306_setPageAddressScope(0,height/8-1);
  // ssd1306_refresh();
}

void ssd1306_setMemoryAddressingMode(MODE mode) {
  // send initial command to the device
  ssd1306_send_data(COMMAND, 0x20);
  ssd1306_send_data(COMMAND, mode);
}

/** Set Column Address [Space] (21h)
  *
  *  3 byte
  *  Command specifies column start address and end address of the display data RAM. This
  *  command also sets the column address pointer to column start address. This pointer is used to define the
  *  current read/write column address in graphic display data RAM.
  *
  *  It setup column start and end address
  *      A[6:0] : Column start address, range : 0-127d, (RESET=0d)
  *      B[6:0]: Column end address, range : 0-127d, (RESET =127d)
  *
  * @param lower  -- up to 127
  * @param higher -- up to 127
  *
  * Note: This command is only for horizontal or vertical addressing mode!
  */

void ssd1306_setColumnAddressScope(uint8_t lower, uint8_t upper) {
  ssd1306_send_data(COMMAND, 0x21);
  ssd1306_send_data(COMMAND, lower);
  ssd1306_send_data(COMMAND, upper);
}

/** Set Page Address (22h)
  *
  *  This triple byte command specifies page start address and end address of the display data RAM. This
  *  command also sets the page address pointer to page start address. This pointer is used to define the current
  *  read/write page address in graphic display data RAM. If vertical address increment mode is enabled by
  *  command 20h, after finishing read/write one page data, it is incremented automatically to the next page
  *  address. Whenever the page address pointer finishes accessing the end page address, it is reset back to start
  *  page address.
  *
  *  Setup page start and end address
  *      A[2:0] : Page start Address, range : 0-7d, (RESET = 0d)
  *      B[2:0] : Page end Address, range : 0-7d, (RESET = 7d)
  *
  *  @param lower  -- from 0 up to 7
  *  @param higher -- from 0 up to 7
  *
  *  Note: This command is only for horizontal or vertical addressing mode.
  */

void ssd1306_setPageAddressScope(uint8_t lower, uint8_t upper) {
  ssd1306_send_data(COMMAND, 0x22);
  ssd1306_send_data(COMMAND, lower);
  ssd1306_send_data(COMMAND, upper);
}

/** Set Page Start Address For Page Addressing Mode (0xB0-0xB7) command
 *  According the documentation Page MUST be from 0 to 7
 *  @param pageNum -- from 0 to 7
 */
void ssd1306_setPageStartAddressForPageAddressingMode(uint8_t pageNum) {
  ssd1306_send_data(COMMAND, (uint8_t) (0xb0 | (pageNum & 0b00000111)));
}

/** Set Display Start Line (40h~7Fh)
 *  This command sets the Display Start Line register to determine starting address of display RAM, by selecting
 *  a value from 0 to 63. With value equal to 0, RAM row 0 is mapped to COM0. With value equal to 1, RAM
 *  row 1 is mapped to COM0 and so on.
 *  @param startLine -- from 0 to 63
 */

void ssd1306_setDisplayStartLine(uint8_t startLine) {
  ssd1306_send_data(COMMAND, (uint8_t) (0x40 | (startLine & 0b00111111)));
}

/** Set Contrast Control for BANK0 (81h)
 *
 * This command sets the Contrast Setting of the display. The chip has 256 contrast steps from 00h to FFh. The
 * segment output current increwhile (_IF_TxE(I2C1));ases as the contrast step value increases.
 * @param value from 0 to 255
 */
void ssd1306_setContrast(uint8_t value) {
  ssd1306_send_data(COMMAND, 0x81);
  ssd1306_send_data(COMMAND, value);
}

/** Set Pre-charge Period (D9h)
 *
 * This command is used to set the duration of the pre-charge period. The interval is counted in number of
 * DCLK, where RESET equals 2 DCLKs.
 *
 * Note:
 * @param value -- experimental typical value is 0xF1
 */

void ssd1306_setPrecharge(uint8_t value) {
  ssd1306_send_data(COMMAND, 0xd9);
  ssd1306_send_data(COMMAND, value);
}

/**
 * Entire Display ON (A4h/A5h)
 * A4h command enable display outputs according to the GDDRAM contents.
 * If A5h command is issued, then by using A4h command, the display will resume to the GDDRAM contents.
 * In other words, A4h command resumes the display from entire display “ON” stage.
 * A5h command forces the entire display to be “ON”, regardless of the contents of the display data RAM.
 *
 * @param resume -- if it will be true, then DISPLAY will go ON and redraw content from RAM
 */
void ssd1306_setDisplayOn(bool resume) {
  uint8_t cmd = (uint8_t) (resume ? 0xA4 : 0xA5);
  ssd1306_send_data(COMMAND, cmd);
}

/** Set Normal/Inverse Display (A6h/A7h)
 *
 *  This command sets the display to be either normal or inverse. In normal display a RAM data of 1 indicates an
 *  “ON” pixel while in inverse display a RAM data of 0 indicates an “ON” pixel.
 *  @param inverse -- if true display will be inverted
 */
void ssd1306_setInverse(bool inverse) {
  uint8_t cmd = (uint8_t) (inverse ? 0xA7 : 0xA6);
  ssd1306_send_data(COMMAND, cmd);
}

/** Set Display ON/OFF (AEh/AFh)
 *
 * These single byte commands are used to turn the OLED panel display ON or OFF.
 * When the display is ON, the selected circuits by Set Master Configuration command will be turned ON.
 * When the display is OFF, those circuits will be turned OFF and the segment and common output are in V SS
 * tate and high impedance state, respectively. These commands set the display to one of the two states:
 *  AEh : Display OFF
 *  AFh : Display ON
 */

void ssd1306_switchOLEDOn(bool goOn) {
  if (goOn) {
    ssd1306_send_data(COMMAND, 0xAF);
  } else
    ssd1306_send_data(COMMAND, 0xAE);
}
 /** Charge Pump Capacitor (8D)
 *
 *  The internal regulator circuit in SSD1306 accompanying only 2 external capacitors can generate a
 *  7.5V voltage supply, V CC, from a low voltage supply input, V BAT . The V CC is the voltage supply to the
 *  OLED driver block. This is a switching capacitor regulator circuit, designed for handheld applications.
 *  This regulator can be turned on/off by software command setting.
 *
 * @param goOn -- if true OLED will going to ON
 * @param enableChargePump -- if On Charge Pump WILL be on when Display ON
 *
 * Note: There are two state in the device: NormalMode <-> SleepMode. If device is in SleepMode then the OLED panel power consumption
 * is close to zero.
 */

 void ssd1306_chargePump(bool chargeOn) {
   ssd1306_send_data(COMMAND, 0x8D);
   if (chargeOn)
     ssd1306_send_data(COMMAND, 0x14);
   else
     ssd1306_send_data(COMMAND, 0x10);
 }
/** Set Display Offset (D3h)
 * The command specifies the mapping of the display start line to one of
 * COM0~COM63 (assuming that COM0 is the display start line then the display start line register is equal to 0).
 * @param verticalShift -- from 0 to 63
 */

void ssd1306_setDisplayOffset(uint8_t verticalShift) {
  ssd1306_send_data(COMMAND, 0xd3);
  ssd1306_send_data(COMMAND, verticalShift);
}

/** Set VcomH Deselect Level (DBh)
 * This is a special command to adjust of Vcom regulator output.
 */
void ssd1306_adjustVcomDeselectLevel(uint8_t value) {
  ssd1306_send_data(COMMAND, 0xdb);
  ssd1306_send_data(COMMAND, value);
}

/** Set Display Clock Divide Ratio/ Oscillator Frequency (D5h)
 *  This command consists of two functions:
 *  1. Display Clock Divide Ratio (D)(A[3:0])
 *      Set the divide ratio to generate DCLK (Display Clock) from CLK. The divide ratio is from 1 to 16,
 *      with reset value = 1. Please refer to section 8.3 (datasheet ssd1306) for the details relationship of DCLK and CLK.
 *  2. Oscillator Frequency (A[7:4])
 *      Program the oscillator frequency Fosc that is the source of CLK if CLS pin is pulled high. The 4-bit
 *      value results in 16 different frequency settings available as shown below. The default setting is 0b1000
 *
 * WARNING: you should NOT call this function with another parameters if you don't know why you do it
 *
 * @param value -- default value is 0x80
 */
void ssd1306_setOscillatorFrequency(uint8_t value) {
  ssd1306_send_data(COMMAND, 0xd5);
  ssd1306_send_data(COMMAND, value);
}

void ssd1306_setMultiplexRatio(uint8_t ratio) {
  ssd1306_send_data(COMMAND, 0xa8);
  ssd1306_send_data(COMMAND, ratio);
}

void ssd1306_setCOMPinsHardwareConfiguration(uint8_t val){
  ssd1306_send_data(COMMAND, 0xda);
  ssd1306_send_data(COMMAND, 0b00110010 & val);
}

/**
 * Set the page start address of the target display location by command B0h to B7h
 * @param page -- from 0 to 7
 *
 * NOTE: It command is fit ONLY for Page mode
 */
void ssd1306_setPage(uint8_t page) {
  ssd1306_send_data(COMMAND, (uint8_t) (0xb0 | (0b00000111 & page)));
}

/**
 * Set the lower and the upper column.
 * See note from datasheet:
 *
 * In page addressing mode, after the display RAM is read/written, the column address pointer is increased
 * automatically by 1. If the column address pointer reaches column end address, the column address pointer is
 * reset to column start address and page address pointer is not changed. Users have to set the new page and
 * column addresses in order to access the next page RAM content.
 *
 * In normal display data RAM read or write and page addressing mode, the following steps are required to
 * define the starting RAM access pointer location:
 *  • Set the page start address of the target display location by command B0h to B7h.
 *  • Set the lower start column address of pointer by command 00h~0Fh.
 *  • Set the upper start column address of pointer by command 10h~1Fh.
 * For example, if the page address is set to B2h, lower column address is 03h and upper column address is 10h,
 * then that means the starting column is SEG3 of PAGE2.
 *
 * According that we should send first lower value. Next we send upper value.
 *
 * @param column -- from 0 to 127
 *
 * NOTE: It command is fit ONLY for Page mode
 */
void ssd1306_setColumn(uint8_t column) {
  uint8_t cmd = (uint8_t) (0x0f & column);
  ssd1306_send_data(COMMAND, cmd);
  cmd = (uint8_t) (0x10 | (column >> 4));
  ssd1306_send_data(COMMAND, cmd);
}

/**
 * Send (and display if OLED is ON) RAM buffer to device
 */
void ssd1306_refresh(uint8_t *screenBuffer, int screenBufferSize) {
  ssd1306_start();
  ssd1306_send(DATAONLY);
  for (uint16_t i = 0; i < screenBufferSize; i++) {
    i2c_send_data(I2C1, screenBuffer[i]); //todo make it with DMA later
    while (_IF_TxE(I2C1));
  }
  ssd1306_stop();
}

void ssd1306_clear(int screenBufferSize) {
  ssd1306_start();
  ssd1306_send(DATAONLY);
  for (uint16_t i = 0; i < screenBufferSize; i++) {
    i2c_send_data(I2C1, 0x00); //todo make it with DMA later
    while (_IF_TxE(I2C1));
  }
  ssd1306_stop();
}