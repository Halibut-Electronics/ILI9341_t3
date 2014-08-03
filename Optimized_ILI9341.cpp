// https://github.com/PaulStoffregen/Optimized_ILI9341
// http://forum.pjrc.com/threads/26305-Highly-optimized-ILI9341-(320x240-TFT-color-display)-library

/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "Optimized_ILI9341.h"
#include <SPI.h>

#define SPICLOCK 30000000

#define WIDTH  ILI9341_TFTWIDTH
#define HEIGHT ILI9341_TFTHEIGHT

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Optimized_ILI9341::Optimized_ILI9341(uint8_t cs, uint8_t dc, uint8_t rst)
// : Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT)
{
  _width    = WIDTH;
  _height   = HEIGHT;
  rotation  = 0;
  cursor_y  = cursor_x    = 0;
  textsize  = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap      = true;
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
}

    
void Optimized_ILI9341::writecommand_cont(uint8_t c)
{
	SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
}

void Optimized_ILI9341::writecommand_last(uint8_t c)
{
	SPI0_SR = SPI_SR_TCF;
	SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
	while (!(SPI0_SR & SPI_SR_TCF)) ; // wait until transfer complete
}

void Optimized_ILI9341::writedata8_cont(uint8_t c)
{
	SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
}

void Optimized_ILI9341::writedata8_last(uint8_t c)
{
	SPI0_SR = SPI_SR_TCF;
	SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0);
	while (!(SPI0_SR & SPI_SR_TCF)) ; // wait until transfer complete
}

void Optimized_ILI9341::writedata16_cont(uint16_t d)
{
	SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
}

void Optimized_ILI9341::writedata16_last(uint16_t d)
{
	SPI0_SR = SPI_SR_TCF;
	SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1);
	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	while (!(SPI0_SR & SPI_SR_TCF)) ; // wait until transfer complete
}


void Optimized_ILI9341::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	writecommand_cont(ILI9341_CASET); // Column addr set
	writedata16_cont(x0);   // XSTART 
	writedata16_cont(x1);   // XEND
	writecommand_cont(ILI9341_PASET); // Row addr set
	writedata16_cont(y0);   // YSTART
	writedata16_cont(y1);   // YEND
	writecommand_cont(ILI9341_RAMWR); // write to RAM
}



void Optimized_ILI9341::pushColor(uint16_t color)
{
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	writedata16_last(color);
	SPI.endTransaction();
}

void Optimized_ILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddrWindow(x, y, x+1, y+1);
	writedata16_last(color);
	SPI.endTransaction();
}


void Optimized_ILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height) h = _height-y;
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddrWindow(x, y, x, y+h-1);
	while (h-- > 1) {
		writedata16_cont(color);
	}
	writedata16_last(color);
	SPI.endTransaction();
}

void Optimized_ILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddrWindow(x, y, x+w-1, y);
	while (w-- > 1) {
		writedata16_cont(color);
	}
	writedata16_last(color);
	SPI.endTransaction();
}

void Optimized_ILI9341::fillScreen(uint16_t color)
{
	fillRect(0, 0, _width, _height, color);
}

// fill a rectangle
void Optimized_ILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	// TODO: this can result in a very long transaction time
	// should break this into multiple transactions, even though
	// it'll cost more overhead, so we don't stall other SPI libs
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddrWindow(x, y, x+w-1, y+h-1);
	for(y=h; y>0; y--) {
		for(x=w; x>1; x--) {
			writedata16_cont(color);
		}
		writedata16_last(color);
	}
	SPI.endTransaction();
}



#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Optimized_ILI9341::setRotation(uint8_t m)
{
	writecommand_cont(ILI9341_MADCTL);
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
	case 0:
		writedata8_last(MADCTL_MX | MADCTL_BGR);
		_width  = ILI9341_TFTWIDTH;
		_height = ILI9341_TFTHEIGHT;
		break;
	case 1:
		writedata8_last(MADCTL_MV | MADCTL_BGR);
		_width  = ILI9341_TFTHEIGHT;
		_height = ILI9341_TFTWIDTH;
		break;
	case 2:
		writedata8_last(MADCTL_MY | MADCTL_BGR);
		_width  = ILI9341_TFTWIDTH;
		_height = ILI9341_TFTHEIGHT;
		break;
	case 3:
		writedata8_last(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		_width  = ILI9341_TFTHEIGHT;
		_height = ILI9341_TFTWIDTH;
		break;
	}
}


void Optimized_ILI9341::invertDisplay(boolean i)
{
	writecommand_last(i ? ILI9341_INVON : ILI9341_INVOFF);
}










uint8_t Optimized_ILI9341::spiread(void)
{
	uint8_t r = 0;
	r = SPI.transfer(0x00);
	return r;
}

uint8_t Optimized_ILI9341::readdata(void)
{
  uint8_t r;
       // Try to work directly with SPI registers...
       // First wait until output queue is empty
        uint16_t wTimeout = 0xffff;
        while (((SPI0.SR) & (15 << 12)) && (--wTimeout)) ; // wait until empty
        
//       	SPI0_MCR |= SPI_MCR_CLR_RXF; // discard any received data
//		SPI0_SR = SPI_SR_TCF;
        
        // Transfer a 0 out... 
        writedata8_cont(0);   
        
        // Now wait until completed. 
        wTimeout = 0xffff;
        while (((SPI0.SR) & (15 << 12)) && (--wTimeout)) ; // wait until empty
        r = SPI0.POPR;  // get the received byte... should check for it first...
    return r;
}
 
 

uint8_t Optimized_ILI9341::readcommand8(uint8_t c, uint8_t index)
{
    uint16_t wTimeout = 0xffff;
    uint8_t r;
    while (((SPI0.SR) & (15 << 12)) && (--wTimeout)) ; // wait until empty
    
    // Make sure the last frame has been sent...
    SPI0.SR = SPI_SR_TCF;   // dlear it out;
    wTimeout = 0xffff;
    while (!((SPI0.SR) & SPI_SR_TCF) && (--wTimeout)) ; // wait until it says the last frame completed

    // clear out any current received bytes
    wTimeout = 0x10;    // should not go more than 4...
    while ((((SPI0.SR) >> 4) & 0xf) && (--wTimeout))  {
        r = SPI0.POPR;
    }
    
    //writecommand(0xD9); // sekret command
	SPI0.PUSHR = 0xD9 | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
//	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full

    // writedata(0x10 + index);
	SPI0.PUSHR = (0x10 + index) | (pcs_data << 16) | SPI_PUSHR_CTAS(0);
//	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full

    // writecommand(c);
   	SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
//	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full

    // readdata
	SPI0.PUSHR = 0 | (pcs_data << 16) | SPI_PUSHR_CTAS(0);
//	while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
        
    // Now wait until completed. 
    wTimeout = 0xffff;
    while (((SPI0.SR) & (15 << 12)) && (--wTimeout)) ; // wait until empty

    // Make sure the last frame has been sent...
    SPI0.SR = SPI_SR_TCF;   // dlear it out;
    wTimeout = 0xffff;
    while (!((SPI0.SR) & SPI_SR_TCF) && (--wTimeout)) ; // wait until it says the last frame completed

    wTimeout = 0x10;    // should not go more than 4...
    // lets get all of the values on the FIFO
    while ((((SPI0.SR) >> 4) & 0xf) && (--wTimeout))  {
        r = SPI0.POPR;
    }
    return r;  // get the received byte... should check for it first...
}


// KJE Added functions to read pixel data...
uint16_t Optimized_ILI9341::readPixel(int16_t x, int16_t y) {
  writecommand_cont(ILI9341_CASET); // Column addr set
  writedata16_cont(x);  // XSTART
  x++;
  writedata16_cont(x);  // XEND

  writecommand_cont(ILI9341_PASET); // Row addr set
  writedata16_cont(y);     // YSTART
  y++;
  writedata16_cont(y);     // YEND

  writecommand_cont(ILI9341_RAMRD); // write to RAM
  
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
  uint16_t r = spiread();
  r <<= 8;
  r |= spiread();
  digitalWrite(_cs, HIGH);
   
  return r;

}



// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Optimized_ILI9341::commandList(uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand_last(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata8_last(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


void Optimized_ILI9341::begin(void)
{
	SPI.begin();
	if (SPI.pinIsChipSelect(_cs, _dc)) {
		pcs_data = SPI.setCS(_cs);
		pcs_command = pcs_data | SPI.setCS(_dc);
	} else {
		pcs_data = 0;
		pcs_command = 0;
		return;
	}
	// TODO: use transactions on all access to SPI
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	SPI.endTransaction();

	// toggle RST low to reset
	if (_rst < 255) {
		pinMode(_rst, OUTPUT);
		digitalWrite(_rst, HIGH);
		delay(5);
		digitalWrite(_rst, LOW);
		delay(20);
		digitalWrite(_rst, HIGH);
		delay(150);
	}

  /*
  uint8_t x = readcommand8(ILI9341_RDMODE);
  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDMADCTL);
  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDPIXFMT);
  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDIMGFMT);
  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
*/
  //if(cmdList) commandList(cmdList);
  
  writecommand_cont(0xEF);
  writedata8_cont(0x03);
  writedata8_cont(0x80);
  writedata8_cont(0x02);

  writecommand_cont(0xCF);  
  writedata8_cont(0x00); 
  writedata8_cont(0XC1); 
  writedata8_cont(0X30); 

  writecommand_cont(0xED);  
  writedata8_cont(0x64); 
  writedata8_cont(0x03); 
  writedata8_cont(0X12); 
  writedata8_cont(0X81); 
 
  writecommand_cont(0xE8);  
  writedata8_cont(0x85); 
  writedata8_cont(0x00); 
  writedata8_cont(0x78); 

  writecommand_cont(0xCB);  
  writedata8_cont(0x39); 
  writedata8_cont(0x2C); 
  writedata8_cont(0x00); 
  writedata8_cont(0x34); 
  writedata8_cont(0x02); 
 
  writecommand_cont(0xF7);  
  writedata8_cont(0x20); 

  writecommand_cont(0xEA);  
  writedata8_cont(0x00); 
  writedata8_cont(0x00); 
 
  writecommand_cont(ILI9341_PWCTR1);    //Power control 
  writedata8_cont(0x23);   //VRH[5:0] 
 
  writecommand_cont(ILI9341_PWCTR2);    //Power control 
  writedata8_cont(0x10);   //SAP[2:0];BT[3:0] 
 
  writecommand_cont(ILI9341_VMCTR1);    //VCM control 
  writedata8_cont(0x3e); //对比度调节
  writedata8_cont(0x28); 
  
  writecommand_cont(ILI9341_VMCTR2);    //VCM control2 
  writedata8_cont(0x86);  //--
 
  writecommand_cont(ILI9341_MADCTL);    // Memory Access Control 
  writedata8_cont(0x48);

  writecommand_cont(ILI9341_PIXFMT);    
  writedata8_cont(0x55); 
  
  writecommand_cont(ILI9341_FRMCTR1);    
  writedata8_cont(0x00);  
  writedata8_cont(0x18); 
 
  writecommand_cont(ILI9341_DFUNCTR);    // Display Function Control 
  writedata8_cont(0x08); 
  writedata8_cont(0x82);
  writedata8_cont(0x27);  
 
  writecommand_cont(0xF2);    // 3Gamma Function Disable 
  writedata8_cont(0x00); 
 
  writecommand_cont(ILI9341_GAMMASET);    //Gamma curve selected 
  writedata8_cont(0x01); 
 
  writecommand_cont(ILI9341_GMCTRP1);    //Set Gamma 
  writedata8_cont(0x0F); 
  writedata8_cont(0x31); 
  writedata8_cont(0x2B); 
  writedata8_cont(0x0C); 
  writedata8_cont(0x0E); 
  writedata8_cont(0x08); 
  writedata8_cont(0x4E); 
  writedata8_cont(0xF1); 
  writedata8_cont(0x37); 
  writedata8_cont(0x07); 
  writedata8_cont(0x10); 
  writedata8_cont(0x03); 
  writedata8_cont(0x0E); 
  writedata8_cont(0x09); 
  writedata8_cont(0x00); 
  
  writecommand_cont(ILI9341_GMCTRN1);    //Set Gamma 
  writedata8_cont(0x00); 
  writedata8_cont(0x0E); 
  writedata8_cont(0x14); 
  writedata8_cont(0x03); 
  writedata8_cont(0x11); 
  writedata8_cont(0x07); 
  writedata8_cont(0x31); 
  writedata8_cont(0xC1); 
  writedata8_cont(0x48); 
  writedata8_cont(0x08); 
  writedata8_cont(0x0F); 
  writedata8_cont(0x0C); 
  writedata8_cont(0x31); 
  writedata8_cont(0x36); 
  writedata8_cont(0x0F); 

  writecommand_last(ILI9341_SLPOUT);    //Exit Sleep 
  delay(120); 		
  writecommand_last(ILI9341_DISPON);    //Display on 

}




/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!
 
Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "glcdfont.c"
// #define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// Draw a circle outline
void Optimized_ILI9341::drawCircle(int16_t x0, int16_t y0, int16_t r,
    uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while (x<y) {
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

void Optimized_ILI9341::drawCircleHelper( int16_t x0, int16_t y0,
               int16_t r, uint8_t cornername, uint16_t color) {
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
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

void Optimized_ILI9341::fillCircle(int16_t x0, int16_t y0, int16_t r,
			      uint16_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void Optimized_ILI9341::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Bresenham's algorithm - thx wikpedia
void Optimized_ILI9341::drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
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
void Optimized_ILI9341::drawRect(int16_t x, int16_t y,
			    int16_t w, int16_t h,
			    uint16_t color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

// Draw a rounded rectangle
void Optimized_ILI9341::drawRoundRect(int16_t x, int16_t y, int16_t w,
  int16_t h, int16_t r, uint16_t color) {
  // smarter version
  drawFastHLine(x+r  , y    , w-2*r, color); // Top
  drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
  drawFastVLine(x    , y+r  , h-2*r, color); // Left
  drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r    , y+r    , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void Optimized_ILI9341::fillRoundRect(int16_t x, int16_t y, int16_t w,
				 int16_t h, int16_t r, uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void Optimized_ILI9341::drawTriangle(int16_t x0, int16_t y0,
				int16_t x1, int16_t y1,
				int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void Optimized_ILI9341::fillTriangle ( int16_t x0, int16_t y0,
				  int16_t x1, int16_t y1,
				  int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
}

void Optimized_ILI9341::drawBitmap(int16_t x, int16_t y,
			      const uint8_t *bitmap, int16_t w, int16_t h,
			      uint16_t color) {

  int16_t i, j, byteWidth = (w + 7) / 8;

  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
	drawPixel(x+i, y+j, color);
      }
    }
  }
}

size_t Optimized_ILI9341::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    if (wrap && (cursor_x > (_width - textsize*6))) {
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }
  return 1;
}

// Draw a character
void Optimized_ILI9341::drawChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t color, uint16_t bg, uint8_t size) {

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

void Optimized_ILI9341::setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void Optimized_ILI9341::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void Optimized_ILI9341::setTextColor(uint16_t c) {
  // For 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

void Optimized_ILI9341::setTextColor(uint16_t c, uint16_t b) {
  textcolor   = c;
  textbgcolor = b; 
}

void Optimized_ILI9341::setTextWrap(boolean w) {
  wrap = w;
}

uint8_t Optimized_ILI9341::getRotation(void) {
  return rotation;
}


