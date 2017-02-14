/*
 * SSD1306.cpp
 *
 *  Created on: 19 Oct 2016
 *      Author: ralim
 */

#include "SSD1306.hpp"
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

#ifndef abs
#define abs(x) ((x)<0 ? -(x) : (x))
#endif
SSD1306::SSD1306(I2C_HandleTypeDef* i2cPort) {
	//create the object
	_i2cPort = i2cPort; //store the i2c port
	cursor_x=cursor_y=0;
	textsize=1;

}
void SSD1306::begin() {

	//init the screen
	// Init sequence for 128x32 OLED module
	command(SSD1306_DISPLAYOFF);                    // 0xAE
	command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
	command(0x80);                                  // the suggested ratio 0x80
	command(SSD1306_SETMULTIPLEX);                  // 0xA8
	command(SSD1306_HEIGHT - 1);
	command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
	command(0x0);                                   // no offset
	command(SSD1306_SETSTARTLINE | 0x0);            // line #0
	command(SSD1306_CHARGEPUMP);                    // 0x8D
#ifdef SSD1306_USE_EXTERNALVCC
			command(0x10);
#else
	command(0x14);
#endif
	command(SSD1306_MEMORYMODE);                    // 0x20
	command(0x00);                                  // 0x0 act like ks0108
	command(SSD1306_SEGREMAP | 0x1);
	command(SSD1306_COMSCANDEC);
#if SSD1306_HEIGHT == 64
	command(SSD1306_SETCOMPINS);                    // 0xDA
	command(0x12);
	command(SSD1306_SETCONTRAST);                   // 0x81
#ifdef SSD1306_USE_EXTERNALVCC
			command(0x9F);
#else
	command(0xCF);
#endif
#else
	command(SSD1306_SETCOMPINS);                    // 0xDA
	command(0x02);
	command(SSD1306_SETCONTRAST);// 0x81
	command(0x8F);
#endif
	command(SSD1306_SETPRECHARGE);                  // 0xd9
#ifdef SSD1306_USE_EXTERNALVCC
			command(0x22);
#else
	command(0xF1);
#endif

	command(SSD1306_SETVCOMDETECT);                 // 0xDB
	command(0x40);
	command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
	command(SSD1306_NORMALDISPLAY);                 // 0xA6
	command(SSD1306_DISPLAYON);                 //--turn on oled panel
}
void SSD1306::clear() {
	//clear the display to off
}
void SSD1306::setInversion(uint8_t invert) {
	//Invert screen?
	  if (invert) {
		  command(SSD1306_INVERTDISPLAY);
	  } else {
		  command(SSD1306_NORMALDISPLAY);
	  }
}

void SSD1306::setContrast(uint8_t contrast)
{
	command(SSD1306_SETCONTRAST);
	 command(contrast);
}
void SSD1306::update() {
	//writes the buffer out to the screen
	command(SSD1306_COLUMNADDR);
	command(0);   // Column start address (0 = reset)
	command(SSD1306_WIDTH - 1); // Column end address (127 = reset)

	command(SSD1306_PAGEADDR);
	command(0); // Page start address (0 = reset)
	command((SSD1306_HEIGHT == 64) ? 7 : 3); // Page end address

	HAL_I2C_Mem_Write(_i2cPort, SSD1306_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT, _lcdBuffer,
			((SSD1306_HEIGHT * SSD1306_WIDTH) / 8), 100);

}

void SSD1306::drawPixel(uint8_t x, uint8_t y, uint8_t color) {
	if (color)
		_lcdBuffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y & 7));
	else
		_lcdBuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
}

void SSD1306::drawLineH(uint8_t x, uint8_t y, uint8_t width, uint8_t colour) {
	// Do bounds/limit checks
	if (y < 0 || y >= SSD1306_HEIGHT) {
		return;
	}
	// make sure we don't try to draw below 0
	if (x < 0) {
		width += x;
		x = 0;
	}
	// make sure we don't go off the edge of the display
	if ((x + width) > SSD1306_WIDTH) {
		width = (SSD1306_WIDTH - x);
	}

	// if our width is now negative, punt
	if (width <= 0) {
		return;
	}

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = _lcdBuffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_WIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = 1 << (y & 7);

	while (width--) {
		if (colour) {
			*pBuf++ |= mask;
		} else {
			*pBuf++ &= ~mask;
		}
	};
}
void SSD1306::drawLineV(uint8_t x, uint8_t __y, uint8_t __h, uint8_t colour) {

	// do nothing if we're off the left or right side of the screen
	if (x < 0 || x >= SSD1306_WIDTH) {
		return;
	}

	// make sure we don't try to draw below 0
	if (__y < 0) {
		// __y is negative, this will subtract enough from __h to account for __y being 0
		__h += __y;
		__y = 0;

	}

	// make sure we don't go past the height of the display
	if ((__y + __h) > SSD1306_HEIGHT) {
		__h = (SSD1306_HEIGHT - __y);
	}

	// if our height is now negative, punt
	if (__h <= 0) {
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t y = __y;
	register uint8_t h = __h;

	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = _lcdBuffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_WIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (y & 7);
	if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod) {
			mask &= (0XFF >> (mod - h));
		}

		if (colour) {
			*pBuf |= mask;
		} else {
			*pBuf &= ~mask;
		}

		// fast exit if we're done here!
		if (h < mod) {
			return;
		}

		h -= mod;

		pBuf += SSD1306_WIDTH;
	}

	// write solid bytes while we can - effectively doing 8 rows at a time
	if (h >= 8) {

		// store a local value to work with

		do {
			// write our value in
			if (colour)
				*pBuf = 0xFF;
			else
				*pBuf = 0;
			// adjust the buffer forward 8 rows worth of data
			pBuf += SSD1306_WIDTH;

			// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
			h -= 8;
		} while (h >= 8);

	}

	// now do the final partial byte, if necessary
	if (h) {
		mod = h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
		register uint8_t mask = postmask[mod];
		if (colour) {
			*pBuf |= mask;
		} else {
			*pBuf &= ~mask;
		}
	}
}

void SSD1306::command(uint8_t cmd) {
	//write a command
	HAL_I2C_Mem_Write(_i2cPort, SSD1306_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 10);
}
void SSD1306::drawcircle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t colour) {
	int16_t f = 1 - r;
	  int16_t ddF_x = 1;
	  int16_t ddF_y = -2 * r;
	  int16_t x = 0;
	  int16_t y = r;

	  drawPixel(x0  , y0+r, colour);
	  drawPixel(x0  , y0-r, colour);
	  drawPixel(x0+r, y0  , colour);
	  drawPixel(x0-r, y0  , colour);

	  while (x<y) {
	    if (f >= 0) {
	      y--;
	      ddF_y += 2;
	      f += ddF_y;
	    }
	    x++;
	    ddF_x += 2;
	    f += ddF_x;

	    drawPixel(x0 + x, y0 + y, colour);
	    drawPixel(x0 - x, y0 + y, colour);
	    drawPixel(x0 + x, y0 - y, colour);
	    drawPixel(x0 - x, y0 - y, colour);
	    drawPixel(x0 + y, y0 + x, colour);
	    drawPixel(x0 - y, y0 + x, colour);
	    drawPixel(x0 + y, y0 - x, colour);
	    drawPixel(x0 - y, y0 - x, colour);
	  }
}
void SSD1306::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint8_t colour) {
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

	  for (; x0<=x1; x0++) {
	    if (steep) {
	    	drawPixel(y0, x0, colour);
	    } else {
	    	drawPixel(x0, y0, colour);
	    }
	    err -= dy;
	    if (err < 0) {
	      y0 += ystep;
	      err += dx;
	    }
	  }
}
void SSD1306::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t colour) {
	drawLineH(x, y, width, colour);
	drawLineH(x, y+height-1, width, colour);
	drawLineV(x, y, height, colour);
	drawLineV(x+width-1, y, height, colour);
}
void SSD1306::drawFillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t colour) {
	for (uint8_t i = x; i < x + width; i++) {
		drawLineV(i, y, height, colour);
	}
}
void SSD1306::drawString(char* string, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		drawChar(cursor_x, cursor_y, string[i]);
		cursor_x += textsize * 6;
	}
}

void SSD1306::drawChar(uint8_t x, uint8_t y, char c) {

	if ((x >= SSD1306_WIDTH) || // Clip right
			(y >= SSD1306_HEIGHT) || // Clip bottom
			((x + 6 * textsize - 1) < 0) || // Clip left
			((y + 8 * textsize - 1) < 0))   // Clip top
		return;

	for (int8_t i = 0; i < 6; i++) {
		uint8_t line;
		if (i < 5)
			line = (font[(c * 5) + i]);
		else
			line = 0x0;
		for (int8_t j = 0; j < 8; j++, line >>= 1) {
			if (line & 0x1) {
				if (textsize == 1)
					drawPixel(x + i, y + j, 1);
				else
					drawFillRect(x + (i * textsize), y + (j * textsize), textsize, textsize, 1);
			} else
			//draw in clearing the stuff behind
			if (textsize == 1)
				drawPixel(x + i, y + j, 0);
			else
				drawFillRect(x + i * textsize, y + j * textsize, textsize, textsize, 0);
		}
	}
}

void SSD1306::setFontSize(uint8_t size) {
//set font size multiplier
	textsize = size;
}
void SSD1306::setCursor(uint8_t x, uint8_t y) {
//set cursor position
	cursor_x = x;
	cursor_y = y;
}
