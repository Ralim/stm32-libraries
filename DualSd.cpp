/*
 * DualSd.cpp
 *
 *  Created on: 3 Nov 2016
 *      Author: ralim
 */

#include <DualSd.h>

DualSd::DualSd(SDCard* sd1, SDCard* sd2) {
	_sd1 = sd1;
	_sd2 = sd2;
	_Size = 0;
}

bool DualSd::initalize() {
	_sd1->initalize();
	_sd2->initalize();

	getSize();
	return true;
}

uint32_t DualSd::getSize() {
	if (_Size == 0) {
		//calculate
		if (_sd1->getSize() < _sd2->getSize())
			_Size = _sd1->getSize() * 2;
		else
			_Size = _sd2->getSize() * 2;

	}
	return _Size;

}

bool DualSd::readBlocks(uint32_t blockaddr, uint8_t* buffer, uint8_t count) {
	bool worked = true;
	for (uint8_t i = 0; i < count; i++) {
		if (blockaddr % 2 == 1)
			worked = _sd2->readBlock(blockaddr>>1, buffer);
		else
			worked = _sd1->readBlock(blockaddr>>1, buffer);
		if (!worked)
			return false;
		blockaddr++;
		buffer += 512;
	}
	return true;
}

bool DualSd::writeBlocks(uint32_t blockaddr, uint8_t* buffer, uint8_t count) {
	bool worked = true;
	for (uint8_t i = 0; i < count; i++) {
		if (blockaddr % 2 == 1)
			worked = _sd2->writeBlock(blockaddr>>1, buffer);
		else
			worked = _sd1->writeBlock(blockaddr>>1, buffer);
		if (!worked)
			return false;
		blockaddr++;
		buffer += 512;
	}
	return true;
}
