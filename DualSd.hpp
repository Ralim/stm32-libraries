/*
 * DualSd.h
 *
 *  Created on: 3 Nov 2016
 *      Author: ralim
 */
/*
 * This file basically does RAID0 across two SD's by striping each block
 * This provides larger capacity at slower speeds.. But who cares, its SPI its damn slow anyway
 * This seems to benchmark around 400kb/sec on a stm32f411-disco (read).
 * Note that about 50% of that time is spent waiting for the sd card to get ready for the read operation...
 */
#ifndef DUALSD_H_
#define DUALSD_H_
#include "SDCard.hpp"
class DualSd {
public:
	DualSd(SDCard* sd1, SDCard* sd2);
	//^ You create the object by linking in two cards
	bool initalize(); //startup the cards
	uint32_t getSize(); //get the total storage space size as n *512 byte blocks
	bool readBlocks(uint32_t blockaddr, uint8_t* buffer, uint8_t count); //reads blocks
	bool writeBlocks(uint32_t blockaddr, uint8_t* buffer, uint8_t count); //writes blocks

private :
	SDCard *_sd1,*_sd2;
	uint32_t _Size;
};

#endif /* DUALSD_H_ */
