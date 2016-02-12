
#include <tmtypes.h>
#ifdef DISPLAY
#include <display.h>
#else
#include <sensor.h>
#endif
#include <fdl.h>

#define DFWAIT do {int q; for (q=0; q<64; q++) ; } while(0)  // 64 seems to work well. should be verified.


// this function will jump to location zero
void reset(void){
	((void (code *) (void)) 0x0000)  ();
}

// simultaneously write	and read one unsigned char to SPI (mode 3). Blocking.
unsigned char FDL_xferSPI (unsigned char ch) {

	u08 SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	SFRPAGE = SPI0_PAGE;

	SPI0DAT = ch;

	// wait for shifting
	while (!SPIF) ;

	SPIF = 0;			// clear the xfer complete flag

	ch = SPI0DAT;
	
	SFRPAGE = SFRPAGE_SAVE;

	return (ch);
}


// returns the value of DataFlash Status Register. Non-blocking.
u08 FDL_DataFlash_Read_Status(void)
{
   u08 dat;

	LO(DF_CS);
	DFWAIT;

	//command
	FDL_xferSPI(STATUS_REGISTER_READ);

	dat = FDL_xferSPI(0);

	HI(DF_CS);
  
   return (dat);
}



void CopytoCodeSpace(u16 num_pages){
		
		xdata flashpagetype flashpage;
		u08 read_page = 0, j=0;
		unsigned char SFRPAGE_SAVE;
		char xdata* data pwrite;
		char EA_save;
		u16 code_count=0, i=0;

		u08 status;
		u08 temp;
		int k;


	

		pwrite = 0x0;						// initialize code pointer

		EA_save = EA;						// save interrupt status
		EA = 0;								// disable interrupts
		SFRPAGE_SAVE = SFRPAGE;
		SFRPAGE = 0x00;
		FLSCL |= 0x01;						// enable flash writes/erases from user software

		PSCTL = 0x03;						// movx writes erase flash page
		while (pwrite < 0x5000){			//erase code space
		
			*pwrite = 0x88;						// initiate page erase
			pwrite += CODE_PAGE_SIZE;			// point to next page
		}
		PSCTL = 0x00;						// movx writes to target ram
		pwrite = 0x00;						// reset code pointer

		
		while (j <= num_pages){
			i=0;

			SFRPAGE = SFRPAGE_SAVE;


//			FDL_DataFlash_Page_Read(MAX_PAGE_SIZE, flashpage.bigbuff, read_page);
// function is put in line here to avoid passing pointer which uses code put in by 
// compiler in lower code space


		// wait for Serial Flash to be available
			do {
				status = FDL_DataFlash_Read_Status();
			} while (!(status & STATUS_READY_MASK));

			// set CS to low
			LO(DF_CS);
			DFWAIT;

			// shift out command
			FDL_xferSPI(MEMORY_PAGE_READ);
		
			// shift out reserved bits and upper bits of page address
			temp = read_page >> 6;
			FDL_xferSPI(temp);	

			// shift out bottom 6 bits of page address and top bits of byte address
			temp = (u08) read_page << 2;
			FDL_xferSPI(temp);

			// shift out bottom 8 bits of unsigned char address
			FDL_xferSPI(0);

			// shift out 4 bytes of don't cares
			FDL_xferSPI(0);	FDL_xferSPI(0);	FDL_xferSPI(0);	FDL_xferSPI(0);
	
			// shift in the data
			for (k=0; k<MAX_PAGE_SIZE; k++){
				flashpage.bigbuff[k] =  FDL_xferSPI(0);
		
			}

			// raise CS to end operation
			HI(DF_CS);

// end of inline function






			SFRPAGE = 0x00;
			PSCTL = 0x01;						// movx writes to codeflash
			while (i<MAX_PAGE_SIZE){
				*pwrite ++ = flashpage.bigbuff[i]; // write new code
				i++;
			}
			PSCTL = 0x00;
			read_page++;
								

			j++;
		}


		PSCTL = 0;							// movx writes target ram

		FLSCL &= ~0x01;						//  disable flash writes from user sw
		EA = EA_save;
		PSW = 0;
		PSBANK = 0;

		
		reset();		//jump to location zero

//		f = (void code *)0x0000;
//		f();

}

// this routine will calculate  and return th checksum of the code
// space startingat address zero
u08 check_sum(unsigned int len){

	u08 *addr = 0;
	u08 checksum = 0;

	while (len){
		checksum += *addr++;
		len--;
	}
	return(checksum);
}