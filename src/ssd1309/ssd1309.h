#ifndef __SSD_1309_H__
#define __SSD_1309_H__

#include <Arduino.h>
#include <SPI.h>

#define SSD_1309_FLASH_FONT
// #define SSD_1309_REVERSE
// #define SSD_1309_RANK_MODE

#ifdef SSD_1309_RANK_MODE
#define SSD_1309_RANK_COUNT 8
#define SSD_1309_BUFFER_SIZE 128
#else
#define SSD_1309_BUFFER_SIZE 1024
#endif

#ifdef SSD_1309_FLASH_FONT
#include <avr/pgmspace.h>
#endif
#include "fonts.h"

class Ssd1309 {
public:
	Ssd1309(uint8_t chipSelectPin, uint8_t readWritePin, uint8_t resetPin);
	void init();
	void clearBuffer();
#ifdef SSD_1309_RANK_MODE
	void startRankDisplay();
	bool haveRankToDisplay();
#endif
	void pixel(uint8_t x, uint8_t y);
	void pixel(uint8_t x, uint8_t y, uint8_t scale);
	void line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
	void hline(uint8_t x1, uint8_t x2, uint8_t y);
	void vline(uint8_t x, uint8_t y1, uint8_t y2);
	void fillbox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
	void sprite(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* sprite);
	void print(uint8_t x, uint8_t y, String str);
	void print(uint8_t x, uint8_t y, String str, uint8_t scale);
	void display();

private:
	char buffer[SSD_1309_BUFFER_SIZE];
#ifdef SSD_1309_RANK_MODE
	uint8_t rank = 0;
#endif
	SPISettings setting;
	uint8_t cs, rw, rs;
	void sendCommand(uint8_t cmd);
	void sendCommand(uint8_t cmd, uint8_t value);
};

#endif
