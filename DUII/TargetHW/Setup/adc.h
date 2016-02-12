#ifndef ADC_H
#define ADC_H
#endif

void InitADC(void);
// KMC TEST - try 10 bit mode
//unsigned char GetADC(unsigned char);
unsigned int GetADC(unsigned char);

#define		ADC_LED_BOARD_TEMP		5
#define 	ADC_LIGHT_LEVEL			4
#define		ADC_VEHICLE_12V			0

// Multiplier from the ADC voltage to Vbatt voltage is 5.2. (Vbatt = ADC reading (dec) X 3.3 X 5.2)
// 3.3 * 5.2 = 17.16
// divide by 4 for oversampling
#define BATTERY_VOLTAGE_CONVERSION		(17.16 / 4.0)

#define SHUTDOWN_VOLTAGE 5.2			// TraqDash will close files and save if power below this voltage