#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "commondefs.h"
#include "tmtypes.h"
#include "RTOS.h"
#include "FS.h"
#include "sounds.h"

// function declarations
void AudioTask(void);
void Speak( char * );

// two wire interface hardware specifications
#define TWI_CLOCK		100000		// 100 KHz
#define TWI_SAMP_RATE	8000		// 8Kbps

// definitions
#define DAC_SLAVE_ADDR		0x46	// 7 bits, 100011x where x is set to 0
#define AMP_SLAVE_ADDR		0x60	// 7 bits, 1100000

// dac registers
#define DAC_VOLUME_L		64
#define DAC_VOLUME_R		65
#define DAC_ATTEN_L			68
#define DAC_ATTEN_R			69
#define DAC_POWERUP			73
#define DAC_MONO_STEREO		74
#define DAC_MASTER_MODE		84
#define DAC_SAMPLING1		85
#define DAC_SAMPLING2		86

// dac definitions
#define DAC_SYSCLK1			0x24	// 00100100, no reset, no swap lr, 12Mhz at 8Kbps
#define DAC_SYSCLK2			0x60	// 01100000, no burst, 12Mhz at 8Kbps, no zero cross

// amp registers
#define AMP_CONTROL_REGISTER	1
#define AMP_VOLUME_REGISTER		2

// amp definitions
#define	AMP_ENABLE			0xD0	// 11010000, LR enable, mono
#define MID_VOLUME			0x20	// 00100000. unmute LR, mid volume

#ifdef __cplusplus
}
#endif

#undef AUDIO_TASK_H

