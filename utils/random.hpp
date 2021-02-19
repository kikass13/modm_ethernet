#pragma once

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

static UBaseType_t ulNextRand;

UBaseType_t initializeRandomNUmbers(){
	// initialize random numbers
    time_t now;
    time(&now);
    ulNextRand = uint32_t(now);
    return ulNextRand;
}

UBaseType_t uxRand( void ){
	static constexpr uint32_t ulMultiplier = 0x015a4e35UL;
	static constexpr uint32_t ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */
	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber){
	*(pulNumber) = uxRand();
	return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t, uint16_t, uint32_t, uint16_t){
	return uxRand();
}

	