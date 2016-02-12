#include "RTOS.H"

// this function will put processor in reset
//
#define _RSTC_BASE_ADDR    (0xFFFFFD00)
#define _RSTC_CR      (*(volatile OS_U32*) (_RSTC_BASE_ADDR + 0x00))

void reset(void);
void reset() {
	_RSTC_CR = 0xA500000D;			// reset processor and peripherals
};
