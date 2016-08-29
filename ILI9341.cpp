/*
 * ILI9341.c
 *
 *  Created on: 30 Jul 2016
 *      Author: ralim
 */
#include "GUI/ILI9341.h"

ILI9341::ILI9341(SPI_HandleTypeDef* spi) {
	spi_port = spi;
	cursor_x = cursor_y = 0; //used for tracking text position
	textcolor = ILI9341_WHITE;
	textbgcolor = ILI9341_BLACK; //text colour and background colour
	textsize = 3;
}
/* LCD Library */
void ILI9341::spiwrite(uint8_t c) {
	HAL_SPI_Transmit(spi_port, &c, 1, 1000);
}

void ILI9341::writecommand(uint8_t c) {
	LCD_DC_LOW
	LCD_CS_LOW
	HAL_SPI_Transmit(spi_port, &c, 1, 100);
	LCD_CS_HIGH
}
void ILI9341::writedata(uint8_t c) {
	LCD_DC_HIGH
	LCD_CS_LOW
	HAL_SPI_Transmit(spi_port, &c, 1, 1000);
	LCD_CS_HIGH
}
void ILI9341::writecommand_last(uint8_t c) {

	HAL_SPI_Transmit(spi_port, &c, 1, 100);
	LCD_CS_HIGH
}

HAL_StatusTypeDef ILI9341::spiwrite16Rpt(uint16_t b, uint32_t count) {
	spi_port->Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(spi_port);
	LCD_DC_HIGH

	if (spi_port->State == HAL_SPI_STATE_READY) {
		/* Process Locked */
		__HAL_LOCK(spi_port);
		/* Check if the SPI is already enabled */
		if ((spi_port->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE) {
			/* Enable SPI peripheral */
			__HAL_SPI_ENABLE(spi_port);
		}
		/* Configure communication */
		spi_port->State = HAL_SPI_STATE_BUSY_TX;
		spi_port->ErrorCode = HAL_SPI_ERROR_NONE;

		while (count > 0) {
			/* Wait until TXE flag is set to send data */
			while (__HAL_SPI_GET_FLAG(spi_port, SPI_FLAG_TXE) == RESET)
				;
			spi_port->Instance->DR = *((uint16_t*) &b);
			count--;
		}

		while (__HAL_SPI_GET_FLAG(spi_port, SPI_FLAG_TXE) == RESET)
			;
		/* Wait until TXE flag is set to send data */

		while (__HAL_SPI_GET_FLAG(spi_port, SPI_FLAG_BSY) == SET)
			;
		/* Wait until Busy flag is reset before disabling SPI */

		/* Clear OVERUN flag in 2 Lines communication mode because received is not read */
		if (spi_port->Init.Direction == SPI_DIRECTION_2LINES) {
			__HAL_SPI_CLEAR_OVRFLAG(spi_port);
		}

		spi_port->State = HAL_SPI_STATE_READY;

		/* Process Unlocked */
		__HAL_UNLOCK(spi_port);

	}
	spi_port->Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(spi_port);
	return HAL_OK;
}

void ILI9341::setBacklight(uint8_t PWM) {
	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = (uint32_t) PWM;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) {
		//Error_Handler();
	}
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIMEx_PWMN_Start(&htim2, TIM_CHANNEL_4);
}
/*
 * This method taken from pauls lib
 * https://github.com/PaulStoffregen/ILI9341_t3/blob/master/ILI9341_t3.cpp
 *
 */
static const uint8_t init_commands[] = { 4, 0xEF, 0x03, 0x80, 0x02, // From adafruit
		4, 0xCF, 0x00, 0XC1, 0X30, // From adafruit
		5, 0xED, 0x64, 0x03, 0X12, 0X81, // From adafruit
		4, 0xE8, 0x85, 0x00, 0x78, // From adafruit
		6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02, // From adafruit
		2, 0xF7, 0x20, // From adafruit
		3, 0xEA, 0x00, 0x00, // From adafruit
		2, ILI9341_PWCTR1, 0x23, // Power control
		2, ILI9341_PWCTR2, 0x10, // Power control
		3, ILI9341_VMCTR1, 0x3e, 0x28, // VCM control
		2, ILI9341_VMCTR2, 0x86, // VCM control2
		2, ILI9341_MADCTL, 0x48, // Memory Access Control
		2, ILI9341_PIXFMT, 0x55, // From adafruit
		3, ILI9341_FRMCTR1, 0x00, 0x18, // From adafruit
		4, ILI9341_DFUNCTR, 0x08, 0x82, 0x27, // Display Function Control
		2, 0xF2, 0x00, // Gamma Function Disable
		2, ILI9341_GAMMASET, 0x01, // Gamma curve selected
		16, ILI9341_GMCTRP1, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00, // Set GammaP
		16, ILI9341_GMCTRN1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F, // Set GammaN
		0 };
void ILI9341::begin() {
	LCD_CS_LOW

	const uint8_t *addr = init_commands;
	while (1) {
		uint8_t count = *addr++;
		if (count-- == 0)
			break;
		writecommand_cont(*addr++);
		while (count-- > 0) {
			writedata8_cont(*addr++);
		}
	}
	LCD_CS_HIGH

	writecommand(ILI9341_SLPOUT);    //Exit Sleep

	HAL_Delay(120);

	writecommand(ILI9341_DISPON);    //Display on
	setRotation(3);    //Set display into landscape mode
}

void ILI9341::fillScreen(uint16_t color) {
	ILI9341::fillRect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, color);
}

// fill a rectangle
void ILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

	// rudimentary clipping (drawChar w/big text requires this)
	/*if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
	 return;
	 if ((x + w - 1) >= ILI9341_TFTWIDTH)
	 w = ILI9341_TFTWIDTH - x;
	 if ((y + h - 1) >= ILI9341_TFTHEIGHT)
	 h = ILI9341_TFTHEIGHT - y;
	 */
	ILI9341::setAddrWindow(x, y, x + w - 1, y + h - 1);
//We need to change SPI to 16 bit interface
	LCD_CS_LOW
	spiwrite16Rpt(color, ((uint32_t) w) * ((uint32_t) h));
	LCD_CS_HIGH
}

void ILI9341::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

	writecommand(ILI9341_CASET); // Column addr set
	spi_port->Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(spi_port);
	LCD_DC_HIGH
	LCD_CS_LOW
	HAL_SPI_Transmit(spi_port, (uint8_t*) &x0, 1, 100);
	HAL_SPI_Transmit(spi_port, (uint8_t*) &x1, 1, 100);
	LCD_CS_HIGH

	spi_port->Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(spi_port);

	writecommand(ILI9341_PASET); // Row addr set

	spi_port->Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(spi_port);
	LCD_DC_HIGH
	LCD_CS_LOW
	HAL_SPI_Transmit(spi_port, (uint8_t*) &y0, 1, 100);
	HAL_SPI_Transmit(spi_port, (uint8_t*) &y1, 1, 100);
	LCD_CS_HIGH
	spi_port->Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(spi_port);
	writecommand(ILI9341_RAMWR); // write to RAM follows
}

uint16_t ILI9341::color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
void ILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {

	if ((x < 0) || (x >= ILI9341_TFTWIDTH) || (y < 0) || (y >= ILI9341_TFTHEIGHT))
		return;

	ILI9341::setAddrWindow(x, y, x + 1, y + 1);
	LCD_DC_HIGH
	spi_port->Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(spi_port);

	LCD_CS_LOW
	HAL_SPI_Transmit(spi_port, (uint8_t *) &color, 1, 200);
	LCD_CS_HIGH
	spi_port->Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(spi_port);
}

void ILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {

	if ((y + h - 1) >= ILI9341_TFTHEIGHT)
		h = ILI9341_TFTHEIGHT - y;

	setAddrWindow(x, y, x, y + h - 1);
	LCD_CS_LOW
	spiwrite16Rpt(color, ((uint32_t) h));
	LCD_CS_HIGH
}

void ILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {

	// Rudimentary clipping
	if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
		return;
	if ((x + w - 1) >= ILI9341_TFTWIDTH)
		w = ILI9341_TFTWIDTH - x;

	ILI9341::setAddrWindow(x, y, x + w - 1, y);

	LCD_CS_LOW
	spiwrite16Rpt(color, ((uint32_t) w));
	LCD_CS_HIGH

	//writecommand(ILI9341_NOP);
}

void ILI9341::setRotation(uint8_t m) {

	writecommand(ILI9341_MADCTL);
	switch (m % 4) {
	case 0:
		writedata(MADCTL_MX | MADCTL_BGR);
		break;
	case 1:
		writedata(MADCTL_MV | MADCTL_BGR);
		break;
	case 2:
		writedata(MADCTL_MY | MADCTL_BGR);
		break;
	case 3:
		writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		break;
	}

}

void ILI9341::invertDisplay(bool i) {

	LCDwritecommand(i ? ILI9341_INVON : ILI9341_INVOFF);

}

void ILI9341::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	drawPixel(x0, y0 + r, color);
	drawPixel(x0, y0 - r, color);
	drawPixel(x0 + r, y0, color);
	drawPixel(x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 + x, y0 - y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + y, y0 + x, color);
		drawPixel(x0 - y, y0 + x, color);
		drawPixel(x0 + y, y0 - x, color);
		drawPixel(x0 - y, y0 - x, color);
	}
}

void ILI9341::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			drawPixel(x0 + x, y0 + y, color);
			drawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			drawPixel(x0 + x, y0 - y, color);
			drawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			drawPixel(x0 - y, y0 + x, color);
			drawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			drawPixel(x0 - y, y0 - x, color);
			drawPixel(x0 - x, y0 - y, color);
		}
	}
}

void ILI9341::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	drawFastVLine(x0, y0 - r, 2 * r + 1, color); //draw line down center
	fillCircleHelper(x0, y0, r, 3, 0, color); //draw the chunks around the center
}

// Used to do circles and roundrects
void ILI9341::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		++x;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}
#ifndef abs
int16_t abs(int16_t i) {
	if (i < 0)
		return -i;
	else
		return i;
}
#endif
// Bresenham's algorithm - thx wikpedia
void ILI9341::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			drawPixel(y0, x0, color);
		} else {
			drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

// Draw a rectangle
void ILI9341::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	drawFastHLine(x, y, w, color);
	drawFastHLine(x, y + h - 1, w, color);
	drawFastVLine(x, y, h, color);
	drawFastVLine(x + w - 1, y, h, color);
}

// Draw a rounded rectangle
void ILI9341::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	drawFastHLine(x + r, y, w - 2 * r, color); // Top
	drawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
	drawFastVLine(x, y + r, h - 2 * r, color); // Left
	drawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
	// draw four corners
	drawCircleHelper(x + r, y + r, r, 1, color);
	drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

// Fill a rounded rectangle
void ILI9341::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	fillRect(x + r, y, w - 2 * r, h, color);

	// draw four corners
	fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

// Draw a triangle
void ILI9341::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	drawLine(x0, y0, x1, y1, color);
	drawLine(x1, y1, x2, y2, color);
	drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void ILI9341::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}
	if (y1 > y2) {
		_swap_int16_t(y2, y1);
		_swap_int16_t(x2, x1);
	}
	if (y0 > y1) {
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}

	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a)
			a = x1;
		else if (x1 > b)
			b = x1;
		if (x2 < a)
			a = x2;
		else if (x2 > b)
			b = x2;
		drawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2)
		last = y1;   // Include y1 scanline
	else
		last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		 a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		 b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			_swap_int16_t(a, b);
		drawFastHLine(a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		 a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		 b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			_swap_int16_t(a, b);
		drawFastHLine(a, y, b - a + 1, color);
	}
}
/*
 * ----------------------------------------
 * Printing Text Section
 * ----------------------------------------
 */
void ILI9341::print(char* c) {
	while (*c)
		ILI9341::write(*c++);
}
void ILI9341::printA(char* c, int length) {
	for (int i = 0; i < length; i++)
		ILI9341::write(c[i]);
}
void ILI9341::write(char c) {

	if (c == '\n') {
		cursor_y += textsize * 8;
		cursor_x = 0;
	} else {
		ILI9341::drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
		cursor_x += textsize * 6;
	}

}

// Draw a character
/*
 * This code adapted from Paul's awesome work
 * https://github.com/PaulStoffregen/ILI9341_t3/blob/master/ILI9341_t3.cpp
 */
void ILI9341::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t fgcolor, uint16_t bgcolor, uint8_t size) {

	//This works by unrolling the drawRect code into here
	setAddrWindow(x, y, x + 6 * size - 1, y + 8 * size - 1);
	spi_port->Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(spi_port);
	uint8_t mask = 0x01;
	uint16_t color;
	uint8_t yr;
	LCD_DC_HIGH
	LCD_CS_LOW

	for (y = 0; y < 8; y++) {
		for (yr = 0; yr < size; yr++) {
			for (x = 0; x < 5; x++) {
				if (font[c * 5 + x] & mask) {
					color = fgcolor;
				} else {
					color = bgcolor;
				}
				//push pixels fast (this function has less overhead)
				spiwrite16Rpt(color, size);
			}
			spiwrite16Rpt(bgcolor, size);
		}
		mask = mask << 1;
	}

	LCD_CS_HIGH
}

void ILI9341::setCursor(int16_t x, int16_t y) {
	cursor_x = x;
	cursor_y = y;
}

void ILI9341::setTextSize(uint8_t s) {
	textsize = s;
}

void ILI9341::setTextColor(uint16_t c, uint16_t b) {
	textcolor = c;
	textbgcolor = b;
}
/*
 * ----------------------------------------
 * Printing Numbers Section
 * ----------------------------------------
 */
//Print to the lcd a number in 'X.XX' form
void ILI9341::printIntOne_Two(uint16_t in) {
	char buffer[3];
	if (in > 999) {
		print((char*) "+++");
		return;
	}
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	write(buffer[2]);
	write('.');
	write(buffer[1]);
	write(buffer[0]);

}
//Print to the lcd a number in 'X.X' form
void ILI9341::printIntOne_One(uint16_t in) {
	char buffer[3];

	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	write(buffer[2]);
	write('.');
	write(buffer[1]);

}
//Print to the lcd a number in 'XX.XX' form
void ILI9341::printIntTwo_Two(uint16_t in) {

	char buffer[4];
	if (in > 9999) {
		print((char*) "++++");
		return;
	}
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[3] = '0' + (in % 10);
	write(buffer[3]); //Write it all out backwards
	write(buffer[2]);
	write('.');
	write(buffer[1]);
	write(buffer[0]);

}

//Print to the lcd a number in '0X.XX' form
void ILI9341::printIntThree_Two(uint16_t in) {

	char buffer[5];
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[3] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[4] = '0' + (in % 10);
	write(buffer[4]);
	write(buffer[3]); //Write it all out backwards
	write(buffer[2]);
	write('.');
	write(buffer[1]);
	write(buffer[0]);

}
void ILI9341::printIntThree_TwoHighlight(int in, int position, uint16_t textColour, uint16_t highlightColour) {
	char buffer[5];
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[3] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[4] = '0' + (in % 10);
	if (position == 4)
		setTextColor(highlightColour, ILI9341_BLACK);
	else
		setTextColor(textColour, ILI9341_BLACK);
	write(buffer[4]);
	if (position == 3)
		setTextColor(highlightColour, ILI9341_BLACK);
	else
		setTextColor(textColour, ILI9341_BLACK);
	write(buffer[3]); //Write it all out backwards
	if (position == 2)
		setTextColor(highlightColour, ILI9341_BLACK);
	else
		setTextColor(textColour, ILI9341_BLACK);
	write(buffer[2]);
	setTextColor(textColour, ILI9341_BLACK);
	write('.');
	if (position == 1)
		setTextColor(highlightColour, ILI9341_BLACK);
	else
		setTextColor(textColour, ILI9341_BLACK);
	write(buffer[1]);
	if (position == 0)
		setTextColor(highlightColour, ILI9341_BLACK);
	else
		setTextColor(textColour, ILI9341_BLACK);
	write(buffer[0]);
	setTextColor(textColour, ILI9341_BLACK);
}
//Print to the lcd a number in 'XXXXX' form
void ILI9341::printIntFive(uint16_t in) {

	char buffer[5];
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[3] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[4] = '0' + (in % 10);
	write(buffer[4]);
	write(buffer[3]); //Write it all out backwards
	write(buffer[2]);
	write(buffer[1]);
	write(buffer[0]);

}
//Print to the lcd a number in  form
void ILI9341::printInt3(uint16_t in) {

	char buffer[5];
	buffer[0] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[1] = '0' + (in % 10);
	in /= 10; //divide by 10
	buffer[2] = '0' + (in % 10);
	write(buffer[2]);
	write(buffer[1]);
	write(buffer[0]);

}

void ILI9341::printInt(uint16_t in, int last, int first) {
	char buffer[6]; //5 chars max
	for (int i = 0; i < (first + last); i++) {
		buffer[i] = '0' + (in % 10);
		in /= 10; //divide by 10
	}
	//we now have our digits as chars
	for (int i = (first + last - 1); i >= first; i--)
		write(buffer[i]);
	if (last > 0) {
		write('.');
		for (int i = (first - 1); i >= 0; i--) {
			write(buffer[i]);
		}
	}
}

