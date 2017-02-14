/*
 * SSD1306.hpp
 *
 *  Created on: 19 Oct 2016
 *      Author: ralim
 */

#ifndef SSD1306_HPP_
#define SSD1306_HPP_
#include "stm32f1xx.h"
#include "GUI/gfxfont.h"
#define SSD1306_HEIGHT 64		/*Height of the display*/
#define SSD1306_WIDTH 128		/*Width of the display*/
#define SSD1306_ADDR 0x78 	/*Device address*/
//#define SSD1306_USE_EXTERNALVCC

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

class SSD1306 {
public:
	SSD1306(I2C_HandleTypeDef* i2cPort); //create the object
	void begin(); //init the hardware
	void clear(); //clear the display to off
	void setInversion(uint8_t invert); //Invert screen?
	void update(); //writes the buffer out to the screen
	void drawLineH(uint8_t x, uint8_t y, uint8_t width, uint8_t colour);
	void drawLineV(uint8_t x, uint8_t y, uint8_t height, uint8_t colour);
	void drawPixel(uint8_t x, uint8_t y, uint8_t color);
	void drawcircle(uint8_t x, uint8_t y, uint8_t r,uint8_t colour);
	void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint8_t colour);
	void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,uint8_t colour);
	void drawFillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height,uint8_t colour);
	void drawString(char* string, uint8_t length);
	void drawChar(uint8_t x, uint8_t y, char c);

	void setContrast(uint8_t conc);
	void setFontSize(uint8_t size); //set font size multiplier
	void setCursor(uint8_t x, uint8_t y); //set cursor position
private:
	void command(uint8_t cmd); //write a command
	uint8_t _lcdBuffer[(SSD1306_HEIGHT * SSD1306_WIDTH) / 8];
	uint8_t cursor_x, cursor_y;
	uint8_t textsize; //text multiplier
	I2C_HandleTypeDef* _i2cPort;
};

#endif /* SSD1306_HPP_ */
