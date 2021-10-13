#include "ssd1309.h"

Ssd1309::Ssd1309(uint8_t chipSelectPin, uint8_t readWritePin, uint8_t resetPin) {
	cs = chipSelectPin;
	rw = readWritePin;
	rs = resetPin;
}

void Ssd1309::init() {
	setting = SPISettings(4000000UL, MSBFIRST, SPI_MODE0); // max 4000000

	pinMode(rs, OUTPUT);
	pinMode(cs, OUTPUT);
	pinMode(rw, OUTPUT);

	digitalWrite(cs, LOW);
	digitalWrite(rs, HIGH);
	digitalWrite(rs, LOW);
	delay(100);
	digitalWrite(rs, HIGH);
	digitalWrite(cs, HIGH);
	delay(100);

	SPI.begin();
	SPI.beginTransaction(setting);
	this->sendCommand(0xAE); // display off
	this->sendCommand(0xD5, 0x20); // display clock
	this->sendCommand(0xA8, 0x3F); // Multiplex ratio
	this->sendCommand(0xD3, 0x00); // display offset
	this->sendCommand(0x40); // display start line
	this->sendCommand(0x8D, 0x10 | 0x04); // charge pump
	this->sendCommand(0x20, 0x02); // memory acces mode (00 : horizontal, 01 vertical, 02 page)
	this->sendCommand(0xA0 || 0x01); // segment remap (00 : 0 to seg0, 01 : 0 to seg 127)
	this->sendCommand(0xC0 || 0x08);// output scan directory (00 : 0 to 63, 08 : 63 to 00)
	this->sendCommand(0xDA, 0x12); // Common HW coinfig ?
	this->sendCommand(0x81, 0xCF); // contrast
	this->sendCommand(0xD9, 0xF1); // pre charge period (resou le probleme de pixel mal allumé)
	this->sendCommand(0xDB, 0x40); // VCom lvl commande ?
	this->sendCommand(0xA4); // entire display on (0xA4 : normal, OxA5 : entire on)
	this->sendCommand(0xA6 | 0x00); // Inverted_Display Color (00 : normal, 01 : inverted)
	this->sendCommand(0xAF); // display on
	SPI.endTransaction();
	delay(100);
}

void Ssd1309::clearBuffer() {
	for (uint16_t i = 0;i < SSD_1309_BUFFER_SIZE;i++) {
		buffer[i] = 0x00;
	}
}

void Ssd1309::pixel(uint8_t x, uint8_t y) {
	if (x > 128 || y > 64)
		return;
#ifdef SSD_1309_REVERSE
	buffer[(8 - y / 8) * 128 - x - 1] |= 1 << (7 - y % 8);
#else
#ifdef SSD_1309_RANK_MODE 
	if (y / 8 == rank) {
		buffer[x] |= 1 << (y % 8);
	}
#else 
	buffer[(y / 8) * 128 + x] |= 1 << (y % 8);
#endif
#endif
}

void Ssd1309::pixel(uint8_t x, uint8_t y, uint8_t scale) {
	fillbox(x * scale, y * scale, (x + 1) * scale - 1, (y + 1) * scale - 1);
}

void Ssd1309::line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	if (x2 - x1 > y2 - y1) {
		for (uint8_t x = x1;x <= x2;x++) {
			pixel(x, y1 + (y2 - y1) * (x - x1 + 1) / (x2 - x1));
		}
	} else {
		for (uint8_t y = y1;y <= y2;y++) {
			pixel(x1 + (x2 - x1) * (y - y1) / (y2 - y1), y);
		}
	}
}

void Ssd1309::hline(uint8_t x1, uint8_t x2, uint8_t y) {
	for (;x1 <= x2;x1++) {
		pixel(x1, y);
	}
}

void Ssd1309::vline(uint8_t x, uint8_t y1, uint8_t y2) {
	for (;y1 <= y2;y1++) {
		pixel(x, y1);
	}
}

void Ssd1309::fillbox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	for (;y1 <= y2;y1++) {
		hline(x1, x2, y1);
	}
}

void Ssd1309::sprite(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* sprite) {
	for (uint8_t i = 0;i < w;i++) {
		for (uint8_t j = 0;j < h;j++) {
			uint8_t z = j * w + i;
			if (sprite[z / 8] & 1 << (7 - z % 8)) {
				pixel(x + i, y + j);
			}
		}
	}
}

void Ssd1309::print(uint8_t x, uint8_t y, String str) {
	uint8_t c = 0, i, d, pattern;
	while (str[c] != '\0') {
		for (i = 0;i < 6;i++) {
		#ifdef SSD_1309_FLASH_FONT
			pattern = pgm_read_byte(&(font[str[c] - 32][i]));
		#else
			pattern = font[str[c] - 32][i];
		#endif
			for (d = 0; d < 8;d++) {
				if (pattern & (1 << d)) {
					pixel(x + c * 6 + i, y + d);
				}
			}
		}
		c++;
	}
}

void Ssd1309::print(uint8_t x, uint8_t y, String str, uint8_t scale) {
	uint8_t c = 0, i, d, pattern;
	while (str[c] != '\0') {
		for (i = 0;i < 6;i++) {
		#ifdef SSD_1309_FLASH_FONT
			pattern = pgm_read_byte(&(font[str[c] - 32][i]));
		#else
			pattern = font[str[c] - 32][i];
		#endif
			for (d = 0; d < 8;d++) {
				if (pattern & (1 << d)) {
					pixel(x + c * 6 + i, y + d, scale);
				}
			}
		}
		c++;
	}
}


#ifdef SSD_1309_RANK_MODE
void Ssd1309::startRankDisplay() {
	rank = 0;
}

bool Ssd1309::haveRankToDisplay() {
	return rank < SSD_1309_RANK_COUNT;
}
#endif


void Ssd1309::display() {
	SPI.beginTransaction(setting);
#ifdef SSD_1309_RANK_MODE
	sendCommand(0xB0 + rank);
	sendCommand(0x00); // lower column start
	sendCommand(0x10); // high column start
	digitalWrite(rw, HIGH);
	digitalWrite(cs, LOW);

	SPI.transfer(buffer, 128);
	digitalWrite(cs, HIGH);
	rank++;
#else 
	for (uint8_t page = 0; page < 8; page++) {
		sendCommand(0xB0 + page);
		sendCommand(0x00); // lower column start
		sendCommand(0x10); // high column start
		digitalWrite(rw, HIGH);
		digitalWrite(cs, LOW);

		SPI.transfer(buffer + (page * 128), 128);
		digitalWrite(cs, HIGH);
	}
#endif
	SPI.endTransaction();
}

void Ssd1309::sendCommand(uint8_t data) {
	digitalWrite(rw, LOW);
	digitalWrite(cs, LOW);
	SPI.transfer(data);
	digitalWrite(cs, HIGH);
}

void Ssd1309::sendCommand(uint8_t data, uint8_t value) {
	digitalWrite(rw, LOW);
	digitalWrite(cs, LOW);
	SPI.transfer(data);
	SPI.transfer(value);
	digitalWrite(cs, HIGH);
}
