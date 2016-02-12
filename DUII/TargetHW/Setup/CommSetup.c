/*
----------------------------------------------------------------------
File    : CommSetup.c adapted from RTOSINIT_AT91SAM9RL64.c
          for ATMEL AT91SAM9Rx64 CPU

Purpose : Initializes and handles the hardware for the App as far
          as required by the OS.
          Feel free to modify this file acc. to your
          target system.

Limitations: App can only have single UART enabled.

No hardware flow control. N-8-1. baudrate definable.

EmbOsView can be independently operated on UART 1
Debug statements can be independently operated on DBGU

--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.H"
#include "commondefs.h"
#include "DUII.h"
#include "CommTask.h"

/*
external functions
*/
extern OS_ISR_HANDLER* OS_ARM_InstallISRHandler (int , OS_ISR_HANDLER* );
extern void OS_ARM_EnableISR(int );
extern void OS_ARM_DisableISR(int );
extern int OS_ARM_ISRSetPrio(int , int );
extern void App_Comm_Setup(void);

#define APP_UART (0)				// use USART 0 for application
#define APP_UART_PRIO    (0x03)		// (0-7) priority for application UART interrupts
#define APP_BAUDRATE (230400)		// SU communicatio = 230kbaud

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define OS_FSYS 200000000uL

#ifndef   OS_PCLK_TIMER
  #define OS_PCLK_TIMER (OS_FSYS / 2)
#endif

/*********************************************************************
*
*       UART settings for Application
*       If you do not want (or can not due to hardware limitations)
*       to dedicate a UART to Application, please define it to be -1
*/
#ifndef   APP_PCLK_UART
  #define APP_PCLK_UART (OS_FSYS / 2)
#endif

#define APP_UART_USED ((APP_UART == 0) || (APP_UART == 1) || (APP_UART == 2))

/*********************************************************************
*
*       Local defines (sfrs used in RTOSInit.c)
*
**********************************************************************
*/

/*      USART, used for Application communication */
#define _USART0_BASE_ADDR  (0xFFFB0000)
#define _USART1_BASE_ADDR  (0xFFFB4000)
#define _USART2_BASE_ADDR  (0xFFFB8000)

/*      PIO control register */
#define _PIOA_BASE_ADDR    (0xfffff400)
#define _PIOB_BASE_ADDR    (0xfffff600)
#define _PIOC_BASE_ADDR    (0xfffff800)

/*      Power management controller */
#define _PMC_BASE_ADDR     (0xFFFFFC00)

#define _PMC_CKGR_PLLAR (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x28))  /* PLLA register */
#define _PMC_CKGR_PLLBR (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x2c))  /* PLLB register */

#define _PMC_PCER     (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x10))  /* Peripheral clock enable register */
#define _PMC_MOR      (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x20))  /* main oscillator register */
#define _PMC_PLLR     (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x2c))  /* PLL register */
#define _PMC_MCKR     (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x30))  /* Master clock register */
#define _PMC_SR       (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x68))  /* status register */
#define _PMC_IMR      (*(volatile OS_U32*) (_PMC_BASE_ADDR + 0x6C))  /* interrupt mask register */

#define _PMC_MOSCS    (1 <<  0)
#define _CKGR_MAINRDY (1 << 16)
#define _PMC_LOCKA    (1 <<  1)
#define _PMC_LOCKB    (1 <<  2)
#define _PMC_MCKRDY   (1 <<  3)
#define _PMC_PCKRDY2  (1 << 10)
#define _PMC_PCKRDY1  (1 <<  9)
#define _PMC_PCKRDY0  (1 <<  8)
#define _PMC_MASK_ALL (_PMC_PCKRDY2 | _PMC_PCKRDY1 | _PMC_PCKRDY0 | \
                       _PMC_MCKRDY  | _PMC_LOCKB    | _PMC_MOSCS)

/*      Advanced interrupt controller (AIC) */
#define _AIC_BASE_ADDR      (0xfffff000)
#define _AIC_SMR_BASE_ADDR  (_AIC_BASE_ADDR + 0x00)
#define _AIC_SVR_BASE_ADDR  (_AIC_BASE_ADDR + 0x80)
#define _AIC_SVR0      (*(volatile OS_U32*) (_AIC_SVR_BASE_ADDR + 0x00))
#define _AIC_SVR1      (*(volatile OS_U32*) (_AIC_SVR_BASE_ADDR + 0x04))
#define _AIC_IVR       (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x100))
#define _AIC_ISR       (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x108))
#define _AIC_IPR       (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x10c))
#define _AIC_IDCR      (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x124))
#define _AIC_ICCR      (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x128))
#define _AIC_IECR      (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x120))
#define _AIC_EOICR     (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x130))
#define _AIC_SPU       (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x134))
#define _AIC_DCR       (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x138))
#define _AIC_FFDR      (*(volatile OS_U32*) (_AIC_BASE_ADDR + 0x144))

/*      AIC interrupt sources and peripheral IDs        */
#define _SYSTEM_IRQ_ID  (1)   /* System IRQ ID             */
#define _US0IRQ_ID      (6)   /* USART Channel 0 interrupt */
#define _US1IRQ_ID      (7)   /* USART Channel 1 interrupt */
#define _US2IRQ_ID      (8)   /* USART Channel 2 interrupt */

#ifndef   _NUM_INT_SOURCES
  #define _NUM_INT_SOURCES   (32)
#endif

#define _INT_PRIORITY_MASK (0x07)
#define _NUM_INT_PRIORITIES   (8)

/*      MATRIX + EBI interface */
#define _MATRIX_BASE_ADDR   (0xFFFFEE00)                                // MATRIX Base Address

//#define _MATRIX_MCFG   (*(volatile OS_U32*) (_MATRIX_BASE_ADDR + 0x00)) // MATRIX Master configuration register
#define _MATRIX_EBICSA (*(volatile OS_U32*) (_MATRIX_BASE_ADDR + 0x120)) // MATRIX EBI Chip Select Assignment register

/*      PIOC, used as data BUS */
#define _PIOB_PDR      (*(volatile OS_U32*) (_PIOB_BASE_ADDR + 0x04))    // PIOB disable register
#define _PIOB_MDDR     (*(volatile OS_U32*) (_PIOB_BASE_ADDR + 0x54))    // PIOB multi driver disable register
#define _PIOB_ASR      (*(volatile OS_U32*) (_PIOB_BASE_ADDR + 0x70))    // PIOB peripheral A select register

/*********************************************************************
*
*       local data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Communication for Application via UART
*
**********************************************************************
*/

#if (APP_UART == 0)
	#define _PIO_US_BASE_ADDR     (_PIOA_BASE_ADDR)
	#define _APP_UART_BASE_ADDR    (_USART0_BASE_ADDR)
	#define APP_UART_ID           (_US0IRQ_ID)
	#define APP_UART_RX_PIN       (7)
	#define APP_UART_TX_PIN       (6)
#elif (APP_UART == 1)
	#define _PIO_US_BASE_ADDR     (_PIOA_BASE_ADDR)
	#define _APP_UART_BASE_ADDR    (_USART1_BASE_ADDR)
	#define APP_UART_ID            (_US1IRQ_ID)
	#define APP_UART_RX_PIN        (12)
	#define APP_UART_TX_PIN        (11)
#elif (APP_UART == 2)
	#define _PIO_US_BASE_ADDR     (_PIOA_BASE_ADDR)
	#define _APP_UART_BASE_ADDR    (_USART2_BASE_ADDR)
	#define APP_UART_ID            (_US2IRQ_ID)
	#define APP_UART_RX_PIN        (14)
	#define APP_UART_TX_PIN        (13)
#endif

  #define _PIO_US_PDR  (*(volatile OS_U32*)(_PIO_US_BASE_ADDR + 0x04)) // PIO disable register
  #define _PIO_US_ASR  (*(volatile OS_U32*)(_PIO_US_BASE_ADDR + 0x70)) // PIO peripheral "A" select register
  #define _PIO_US_BSR  (*(volatile OS_U32*)(_PIO_US_BASE_ADDR + 0x74)) // PIO peripheral "B" select register

  #define PIO_PDR_USART ((1 << APP_UART_RX_PIN) | (1 << APP_UART_TX_PIN))

  #define _US_CR   (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x00))
  #define _US_MR   (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x04))
  #define _US_IER  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x08))
  #define _US_IDR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x0c))
  #define _US_IMR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x10))
  #define _US_CSR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x14))
  #define _US_RHR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x18))
  #define _US_THR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x1c))
  #define _US_BRGR (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x20))
  #define _US_RTOR (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x24))
  #define _US_TTGR (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x28))

  #define _US_TPR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x108))
  #define _US_TCR  (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x10c))
  #define _US_PTCR (*(volatile OS_U32*)(_APP_UART_BASE_ADDR + 0x120))

  #define US_RXRDY		(1 << 0)		// Rx status flag
  #define US_TXRDY		(1 << 1)		// Tx RDY Status flag
  #define US_ENDTX		(1 << 4)		// Tx end flag
  #define US_TXEMPTY	(1 << 9)		// Tx EMPTY Status flag
  #define US_TXBUFE		(1 << 11)		// Tx Buffer Empty Status flag
  #define US_RXBUFF		(1 << 12)		// Rx Buffer Full Status flag

  #define USART_RX_ERROR_FLAGS (0xE0) /* Parity, framing, overrun error  */

// -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register --------
#define AT91C_PDC_RXTEN       (0x1 <<  0) // (PDC) Receiver Transfer Enable
#define AT91C_PDC_RXTDIS      (0x1 <<  1) // (PDC) Receiver Transfer Disable
#define AT91C_PDC_TXTEN       (0x1 <<  8) // (PDC) Transmitter Transfer Enable
#define AT91C_PDC_TXTDIS      (0x1 <<  9) // (PDC) Transmitter Transfer Disable

#ifdef UARTDMA
/*********************************************************************
*
*       App_putstring()
*/

void App_putstring(void *buffer, unsigned int size) {


	while((_US_CSR & US_ENDTX) == 0) ;	// Wait until buffer becomes available

	_US_TPR = (unsigned int) buffer;	// the non-volatile buffer
	_US_TCR = size;						// number of chars
	_US_PTCR = AT91C_PDC_TXTEN;			// enable transfer
}
#else			// single character transmit

#if 0		// in case of datatrace
typedef struct {
	int rcvdid;
	int rcvdlen;
	int sentid;
	int sentlen;
	float maxxg;
} messagehistorytype;

messagehistorytype messagehistory[100];
int messagecounter = 0;
#endif


// This function starts an interrupt-driven transmit of the trasmit buffer
// on designated UART. Blocks only if previous transmission is in progress.
void XmitUART(comporthandle *handle, u16 numbytes) {

	while (0 != handle->txbytes)
		OS_Delay(5) ;		// wait for previous transmission to finish
//	while (0 != CTS[handle->port]);
	
	handle->txptr = handle->txbuff;
	handle->txbytes = numbytes;

	// put the first character in the buffer to get things started
//	_US_THR = *(handle->txptr);

	// adjust counters
//	(com[0].txptr)++;
//	(com[0].txbytes)--;
		
	// enable interrupts
	_US_IER = US_TXRDY;	
} // XmitUART
	
#endif


/*********************************************************************
*
*       App_ISR_Usart() Application USART interrupt handler
*       handles both Rx and Tx interrupt
*		processes single character receive
*		sends messages via DMA buffer
*
*		parses TCAT Traqmate Communications Protocol character by character
*/

static void App_ISR_Usart(void) {
	int UsartStatus;
	volatile int Dummy;
	u08 tempch;
	static int msg_state = LOOKING_FOR_SYNC;	// init state machine variable
	static int msg_length ; 					// variables to build message
	static u08 chk = 0;							// init the checksum variable
	static int index = 0;						// init the receive character storage index
	static u08 *msgbuff;						// for quicker access to buffer

	UsartStatus = _US_CSR;							// get uart status
	if (UsartStatus & US_RXRDY) {				// check for data received
		if (UsartStatus & USART_RX_ERROR_FLAGS) {	// check for errors
			Dummy = _US_RHR;						// discard data with errors
			_US_CR = (1 << 8);						// reset error status bits PARE, FRAME, OVRE and RXBRK */
		} // if
		else {		// process character
			tempch = _US_RHR;

			switch(msg_state) { 					// based on the state of reception we are in
			case LOOKING_FOR_SYNC:				// state = 1
				index = 0;       				// init the storage index
				if (tempch==SOH) {				// if start of header byte received
					msgbuff = msgrxbuff[com[0].rxnextidx];	// for quicker access to buffer
					msgbuff[index++]=tempch ;	// store SOH
					chk ^= tempch ;				// start checksum calc
					msg_state = POSSIBLE_SYNC;      // update state
				}
				break ;
			
			case POSSIBLE_SYNC:					// state = 2
				msgbuff[index++] = tempch ;	// get the next byte
				chk ^= tempch ;				// continue checksum calc
				if (index == CHECKSUM)			// see if we should have
					msg_state = DO_CHECKSUM;		// received the checksum
				break ;
	
			case DO_CHECKSUM:					// state = 3
				msgbuff[index++]=tempch ;		// store rcv checksum
				chk ^= 0xff ;					// final calculation exor
				if (chk == tempch) {			// check against rcv'd checksum
												// if they match, get the message length
					msg_length = msgbuff[UPPER_BYTE_COUNT];
					msg_length = msg_length << 8 ;
					msg_length += msgbuff[LOWER_BYTE_COUNT];
	
					if(msg_length) msg_state=FOUND_SYNC ;	// remainder of message
					else msg_state=MESSAGE_COMPLETE;		// no message remaining
				}
				else {							// if checksum failed, start looking again
					index=0;					// discard all received data
					msg_state=LOOKING_FOR_SYNC;		// set new state
					chk = 0;					// re-init checksum byte
				}
				break ;
			
			case FOUND_SYNC:					// get remainder of message // state = 4
				msgbuff[index++]=tempch ;		// store the data
				if (index == DATA_START+msg_length)		// see if finished
					msg_state=MESSAGE_COMPLETE ;			// set message complete
				break;
	
			default:            // should never get here if state machine works
				break ;
			} // switch
												// state = 5
			if(msg_state==MESSAGE_COMPLETE) {	// check to see if complete msg received

#if 0				
				messagehistory[messagecounter].rcvdid = msgbuff[OPCODE];
				messagehistory[messagecounter].rcvdlen = msg_length;
				messagehistory[messagecounter].maxxg = DataValues[MAX_X_G].fVal;

				if (messagecounter < 100)
					messagecounter++;
				else
					messagecounter--;
#endif
				
				msg_state=LOOKING_FOR_SYNC ; // if so, set up for new msg
				index = 0 ;              // reset index
				chk = 0;                 // reset checksum variable
				if (NULL == com[0].rxptr)	{	// if last buffer processed
					com[0].rxptr = msgbuff; 		// set semaphore
//					com[0].rxnextidx = 1 - com[0].rxnextidx;	// swap buffers
					if (++(com[0].rxnextidx) >= NUM_RX_BUFF) com[0].rxnextidx = 0;	// go to next buffer
					OS_SignalEvent((1 << TCAT_MSG_RECEIVED), &commTCB);
				} // if
			} // if
		} // else
	} // if

//	UsartStatus = _US_CSR;					// get uart status
	
#ifdef UARTDMA
	if (UsartStatus & US_ENDTX) {			// check if transmission finished
		_US_IDR = US_ENDTX;					// disable interrupt
//		_US_PTCR = AT91C_PDC_TXTDIS;		// disable DMA transfer
		com[0].txptr = NULL;				// mark transmit buffer as empty
	} // if
#else // single character transmit
	
	if (UsartStatus & US_TXRDY ) {		// byte transmitted
		if (0 != com[0].txbytes) {
			_US_THR = *(com[0].txptr);
			(com[0].txptr)++;
			(com[0].txbytes)--;
		}
		else {
			_US_IDR = US_TXRDY;			// disable transmit interrupt
			com[0].txptr = NULL;		// clear pointer
		}
	} // if
#endif

} // App_ISR_Usart


/*********************************************************************
*
*       App_COM_Init()
*       Initialize UART for Application
*/
void App_COM_Init(void);
void App_COM_Init(void) {             /* Initialize UART, enable UART interrupts */
  _PMC_PCER  = (1 << APP_UART_ID);    /* Enable peripheral clock for selected USART */
  _PIO_US_PDR  = PIO_PDR_USART;      /* Enable peripheral output signals on PIO for USART (disable PIO Port) */
  _PIO_US_ASR  = PIO_PDR_USART;      /* Select "A" peripherals on PIO for USART (USART Rx, TX) */
  _PIO_US_BSR  = 0;                  /* Deselct "B" peripherals on PIO for USART */
  _US_CR   = (1 <<  2) |             /* RSTRX: Reset Receiver: 1 = The receiver logic is reset. */
             (1 <<  3)               /* RSTTX: Reset Transmitter: 1 = The transmitter logic is reset. */
           ;
  _US_CR   = (0 <<  2) |             /* RSTRX: Reset Receiver: 1 = The receiver logic is reset. */
             (0 <<  3) |             /* RSTTX: Reset Transmitter: 1 = The transmitter logic is reset. */
             (1 <<  4) |             /* RXEN: Receiver Enable: 1 = The receiver is enabled if RXDIS is 0. */
             (0 <<  5) |             /* RXDIS: Receiver Disable: 0 = Noeffect. */
             (1 <<  6) |             /* TXEN: Transmitter Enable: 1 = The transmitter is enabled if TXDIS is 0. */
             (0 <<  7) |             /* TXDIS: Transmitter Disable: 0 = Noeffect. */
             (1 <<  8) |             /* RSTSTA: Reset Status Bits: 1 = Resets the status bits PARE, FRAME, OVRE and RXBRK in the US_CSR. */
             (0 <<  9) |             /* STTBRK: Start Break: 0 = Noeffect. */
             (0 << 10) |             /* STPBRK: Stop Break: 0 = Noeffect. */
             (0 << 11) |             /* STTTO: Start Time-out: 0 = Noeffect. */
             (0 << 12)               /* SENDA: Send Address: 0 = Noeffect. */
           ;
  _US_MR   = (0 <<  4) |             /* USCLKS: Clock Selection: 0 = MCK */
             (3 <<  6) |             /* CHRL: Character Length: 3 = Eight bits */
             (0 <<  8) |             /* SYNC: Synchronous Mode Select: 0 = USART operates in Asynchronous Mode. */
           (0x4 <<  9) |             /* PAR: Parity Type: 0x4 = No parity */
             (0 << 12) |             /* NBSTOP: Number of Stop Bits: 0 = 1 stop bit */
             (0 << 14) |             /* CHMODE: Channel Mode: 0 = Normal mode */
             (0 << 17) |             /* MODE9: 9-bit Character Length: 0 = CHRL defines character length. */
             (0 << 18)               /* CKLO: Clock Output Select: 0 = The USART does not drive the SCK pin. */
           ;
  _US_BRGR = (APP_PCLK_UART / (APP_BAUDRATE * 16));
  _US_IDR  = 0xFFFFFFFF;             /* Disable all interrupts     */
  _US_IER  = US_RXRDY;               /* Enable Rx Interrupt , Do not Enable Tx Interrupt */

  /* create memory buffers for isr */
  App_Comm_Setup();

  /* Setup interrupt controller for App USART */
  OS_ARM_InstallISRHandler(APP_UART_ID, &App_ISR_Usart);  /* App UART interrupt handler vector */
  OS_ARM_ISRSetPrio(APP_UART_ID, APP_UART_PRIO);              /* Level sensitive, selected priority. */
  _AIC_ICCR = (1 << APP_UART_ID);                            /* Clear App usart interrupt.        */
  OS_ARM_EnableISR(APP_UART_ID);                             /* Enable App usart interrupts       */
}
/*****  EOF  ********************************************************/

