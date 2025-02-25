#include "display.h"


int main() {
	uint8_t screenBuffer[BUFFERLENGTH] = {0};
	i2c_init();
	ssd1306_init(128, 64);
	ssd1306_start();
	for (int i = 0; i < BUFFERLENGTH; i++) {
		screenBuffer[i] = 0xFF;
	}

	ssd1306_refresh(screenBuffer, BUFFERLENGTH);
	for (volatile int i = 0; i < 5000000; i++);
	ssd1306_clear(screenBuffer, BUFFERLENGTH);
	ssd1306_stop();

	return 0;
}