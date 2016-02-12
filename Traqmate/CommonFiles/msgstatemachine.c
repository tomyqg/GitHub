// msgstatemachine.c
// part of traqmate
// included by cygfx020.c
// GAStephens, 16 SEP 2004
//
// this code will not compile on its own.
// it is included by the ISR routines in order to have a single instance of the code for
// maintainability.
//
// define COM as 0 or 1 depending on which ISR
//
// optimized with pointers, GAS 23-SEP-04
//
	u08 tempch;
	static int msg_state = LOOKING_FOR_SYNC;	// init state machine variable
	static int msg_length ; 					// variables to build message
	static u08 chk = 0;							// init the checksum variable
	static int index = 0;						// init the receive character storage index
	static u08 *msgbuff;							// for quicker access to buffer

#if (COM == 0)
#undef SCON
#undef SBUF
#define SCON SCON0
#define SBUF SBUF0
#elif (COM == 1)
#undef SCON
#undef SBUF
#define SCON SCON1
#define SBUF SBUF1
#endif // elif

	// determine if tx or rx or both caused interrupt

	if (SCON & 0x01) {			// byte received
		tempch = SBUF;
		SCON &= ~0x01;						// clear receive interrupt flag

		switch(msg_state) { 					// based on the state of reception we are in
		case LOOKING_FOR_SYNC:				// state = 1
			index = 0;       				// init the storage index
			if (tempch==SOH) {				// if start of header byte received
				msgbuff = com[COM].rxbuff+(com[COM].rxnextidx*MSGSIZE);	// for quicker access to buffer
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
#if defined(SENSOR) && (COM == 0)
			// turn on the COM LED
			LED3 = 1;
#endif
			msgbuff[index++]=tempch ;		// store the data
			if (index == DATA_START+msg_length)		// see if finished
				msg_state=MESSAGE_COMPLETE ;			// set message complete
			break;

		default:            // should never get here if state machine works
			break ;
		} // switch
									// state = 5
		if(msg_state==MESSAGE_COMPLETE) { // check to see if complete msg received
#if defined(SENSOR) && (COM == 0)
			// turn off the COM LED
			LED3 = 0;
#endif
			msg_state=LOOKING_FOR_SYNC ; // if so, set up for new msg
			index = 0 ;              // reset index
			chk = 0;                 // reset checksum variable
			if (NULL == com[COM].rxptr)	{	// if last buffer processed
				com[COM].rxptr = msgbuff; 		// set semaphore
				com[COM].rxnextidx = 1 - com[COM].rxnextidx;	// swap buffers
			} // if
		} // if
	} // if

	if (SCON & 0x02) {		// byte transmitted
		if (0 != com[COM].txbytes) {
#if defined(SENSOR) && (COM == 0)
			// turn on the COM LED
			LED3 = 1;
#endif
			SBUF = *(com[COM].txptr);
			(com[COM].txptr)++;
			(com[COM].txbytes)--;
		}
		else {	 
#if defined(SENSOR) && (COM == 0)
			// turn off the COM LED
			LED3 = 0;
#endif
			com[COM].txptr = NULL;
		}
		SCON &= ~0x02;						// clear transmit interrupt flag
	} // if

#undef SCON
#undef SBUF