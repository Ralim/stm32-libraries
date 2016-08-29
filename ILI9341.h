/*
 * ILI9341.h
 *
 *  Created on: 30 Jul 2016
 *      Author: ralim
 */

#ifndef ILI9341_H_
#define ILI9341_H_
#ifdef __cplusplus
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "gfxfont.h"
#include "mxconstants.h"
#include "setup.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_spi.h"
#include "ILI9341_defines.h"
#include "Colours.h"
//GPIO definitions for the HAL
#define LCD_CS_LOW	HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET);
#define LCD_CS_HIGH	HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET);
#define LCD_DC_LOW	HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET);
#define LCD_DC_HIGH	HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET);

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

class ILI9341 {
public:
	ILI9341(SPI_HandleTypeDef* spi);
	void begin();
	void fillScreen(uint16_t color);

	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void setBacklight(uint8_t PWM);
	void setCursor(int16_t x, int16_t y);
	void setTextColor(uint16_t c, uint16_t b);
	void setTextSize(uint8_t s);
	void print(char* c);
	void printA(char* c, int length);
	void write(char c);
	void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
	void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
	void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
	void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
	void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
	void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
	void invertDisplay(bool i);
	void setRotation(uint8_t m);
	void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

	//Numbers are normally in X100 format
	//these functions all print to current cursor location
	void printIntOne_One(uint16_t in);
	void printIntOne_Two(uint16_t in);
	void printIntTwo_Two(uint16_t in);
	void printIntThree_Two(uint16_t in);
	void printIntFive(uint16_t in);
	void printInt3(uint16_t in);
	void printIntThree_TwoHighlight(int in, int position, uint16_t textColour, uint16_t highlightColour);
	void printInt(uint16_t in, int first, int last);
private:
	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
	void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);

	void writedata(uint8_t c);
	void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
	void LCDwritecommand(uint8_t c);
	void spiwrite(uint8_t c);
	HAL_StatusTypeDef spiwrite16Rpt(uint16_t b, uint32_t count);

	void spiwrite16(uint16_t* buffer, uint16_t size) {
		HAL_SPI_Transmit(spi_port, (uint8_t*) buffer, size, 1000);
	}
	void writedata16(uint16_t c) {
		LCD_DC_HIGH
		HAL_SPI_Transmit(spi_port, (uint8_t *) &c, 2, 100);
	}
	void writedata8_cont(uint8_t c) {
		LCD_DC_HIGH
		HAL_SPI_Transmit(spi_port, &c, 1, 100);
	}
	void writecommand(uint8_t c);
	void writecommand_cont(uint8_t data) {
		LCD_DC_LOW
		HAL_SPI_Transmit(spi_port, &data, 1, 100);
	}
	void writecommand_last(uint8_t c);
protected:
	int16_t cursor_x, cursor_y; //used for tracking text position
	uint16_t textcolor, textbgcolor; //text colour and background colour
	uint8_t textsize;
	SPI_HandleTypeDef* spi_port;

};

#endif /* ILI9341_H_ */
#endif /* c++ */
