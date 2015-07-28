/* --COPYRIGHT--
 *
/*  Backyard Brains 2015
 * ======== main.c ========
 */
#include <string.h>

#include "driverlib.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"
#include "USB_API/USB_HID_API/UsbHid.h"
#include "USB_app/usbConstructs.h"

/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */
#include "hal.h"


//================ Parameters ===================
#define FIRMWARE_VERSION "0.01"  // firmware version. Try to keep it to 4 characters
#define HARDWARE_TYPE "NEURONSB" // hardware type/product. Try not to go over 8 characters
#define HARDWARE_VERSION "0.01"  // hardware version. Try to keep it to 4 characters
#define COMMAND_RESPONSE_LENGTH 35  //16 is just the delimiters etc.
#define DEBOUNCE_TIME 2000
#define MAX_SAMPLE_RATE "10000" //this will be sent to Host when host asks for maximal ratings
#define MAX_NUMBER_OF_CHANNELS "2" //this will be sent to Host when host asks for maximal ratings
//===============================================

// Global flags set by events
volatile uint8_t bHIDDataReceived_event = FALSE; // Indicates data has been rx'ed
                                              // without an open rx operation
volatile uint8_t bHIDDataSent_event = FALSE;
// Application globals
uint16_t w;
volatile uint16_t rounds = 0;
char outString[32];
char pakOutString[16];

char *commandDeimiter = ";";
char *parameterDeimiter = ":";



#define MEGA_DATA_LENGTH 124//992
#define RECEIVE_BUFFER_LENGTH 62
uint8_t bufferX[MEGA_DATA_LENGTH];
uint8_t bufferY[MEGA_DATA_LENGTH];

unsigned int tempADCresult;
unsigned int writingHeadX;
unsigned int writingHeadY;
unsigned int weUseBufferX;

unsigned int tempIndex = 0;//used for parsing config parameters etc.
unsigned int counterd = 0;
unsigned int weHaveDataToSend = 0;
unsigned int numberOfChannels = 2;

unsigned int debounceTimer1 = 0;
unsigned int debounceTimer2 = 0;
unsigned int eventEnabled1 = 1;
unsigned int eventEnabled2 = 1;


void setupADC();
void setupPeriodicTimer();
void sendStringWithEscapeSequence(char * stringToSend);
void executeCommand(char * command);

void main (void)
{

	  uint8_t y;

    // Set up clocks/IOs.  initPorts()/initClocks() will need to be customized
    // for your application, but MCLK should be between 4-25MHz.  Using the
    // DCO/FLL for MCLK is recommended, instead of the crystal.  For examples 
    // of these functions, see the complete USB code examples.  Also see the 
    // Programmer's Guide for discussion on clocks/power.
    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer
    
    // Minimum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .
#ifndef DRIVERLIB_LEGACY_MODE
    PMM_setVCore(PMM_CORE_LEVEL_2);
#else
    PMM_setVCore(PMM_BASE, PMM_CORE_LEVEL_2);
#endif
    
       initPorts();           // Config GPIOS for low-power (output low)
       initClocks(16000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
       USB_setup(TRUE, TRUE); // Init USB & events; if a host is present, connect
       setupADC();            //setup ADC
       setupPeriodicTimer();  //setup periodic timer for sampling

       counterd = 0;

       weUseBufferX = 1;
       writingHeadY = 0;
       writingHeadX = 0;

       bHIDDataSent_event = TRUE;

       // Pre-fill the buffers with visible ASCII characters (0x21 to 0x7E)
       y = 0x21;
       for (w = 0; w < MEGA_DATA_LENGTH; w++)
       {
           bufferX[w] = y;
           bufferY[w] = y;
       }

       //Logical inputs sensing
       P4DIR = BIT1 + BIT2;
       P4OUT = 0;


       __enable_interrupt();  // Enable interrupts globally

       while (1)
       {
           switch (USB_connectionState())
           {
			   // This case is executed while your device is connected to the USB
			   // host, enumerated, and communication is active. Never enter LPM3/4/5
			   // in this mode; the MCU must be active or LPM0 during USB communication
               case ST_ENUM_ACTIVE:

            	   if(bHIDDataReceived_event)
            	   {

            		   // Holds the new  string
					   char receivedString[RECEIVE_BUFFER_LENGTH] = "";
					   char * command1;
					   char * command2;
					   char * command3;

					   // Add bytes in USB buffer to the string
					   hidReceiveDataInBuffer((uint8_t*)receivedString,
						   RECEIVE_BUFFER_LENGTH,
						   HID0_INTFNUM);
					   command1 = strtok(receivedString, commandDeimiter);
					   command2 = strtok(NULL, commandDeimiter);
					   command3 = strtok(NULL, commandDeimiter);
					   executeCommand(command1);
					   executeCommand(command2);
					   executeCommand(command3);

            		   bHIDDataReceived_event = FALSE;
            	   }


            	   if(weHaveDataToSend >0 && bHIDDataSent_event)
            	   {
            		   weHaveDataToSend = 0;
            		   bHIDDataSent_event = FALSE;
            		  /* counterd++;
            		   if(counterd>1000)
            		   {
            		   		counterd = 0;

            		   }*/
                	   if(weUseBufferX)
                	   {
                           if (hidSendDataInBackground((uint8_t*)bufferY,MEGA_DATA_LENGTH,
                          								   HID0_INTFNUM,0))
                           {
								   // Operation probably still open; cancel it
								   USBHID_abortSend(&w,HID0_INTFNUM);
								   break;
                           }

                	   }
                	   else
                       {

                           if (hidSendDataInBackground((uint8_t*)bufferX,MEGA_DATA_LENGTH,
                           								   HID0_INTFNUM,0))
                           {
                          	  	  // Operation probably still open; cancel it
                           	   	   USBHID_abortSend(&w,HID0_INTFNUM);
                           			break;
                           }
                       }
            	   }

                   break;

               // These cases are executed while your device is disconnected from
               // the host (meaning, not enumerated); enumerated but suspended
               // by the host, or connected to a powered hub without a USB host
               // present.
               case ST_PHYS_DISCONNECTED:// physically disconnected from the host
               case ST_ENUM_SUSPENDED:// connected/enumerated, but suspended
               case ST_PHYS_CONNECTED_NOENUM_SUSP:// connected, enum started, but host unresponsive

            	   // In this example, for all of these states we enter LPM3. If
            	   // the host performs a "USB resume" from suspend, the CPU will
            	   // automatically wake. Other events can also wake the CPU, if
            	   // their event handlers in eventHandlers.c are configured to return TRUE.
                  // __bis_SR_register(LPM3_bits + GIE);
                  // _NOP();

            	   TA0CCTL0 &= ~CCIE;//stop timer -> this will stop sampling -> this will stop stream
            	   //prepare buffer for next connection
            	   weUseBufferX = 1;
				   writingHeadY = 0;
				   writingHeadX = 0;
				   //setup flag in case somebody pull the USB plug while it was sending
            	   bHIDDataSent_event = TRUE;
                   break;

               // The default is executed for the momentary state
               // ST_ENUM_IN_PROGRESS.  Usually, this state only last a few
               // seconds.  Be sure not to enter LPM3 in this state; USB
               // communication is taking place here, and therefore the mode must
               // be LPM0 or active-CPU.
               case ST_ENUM_IN_PROGRESS:
               default:;
           }
       }  //while(1)
   } //main()

//
// Respond to a command. Send response if necessary
//
void executeCommand(char * command)
{
	if(command == NULL)
	{
		return;
	}
   char * parameter = strtok(command, parameterDeimiter);
   if (!(strcmp(parameter, "h"))){//stop
	   TA0CCTL0 &= ~CCIE;
	   return;
   }
   if (!(strcmp(parameter, "?"))){//get parameters of MSP


	   char responseString[COMMAND_RESPONSE_LENGTH] = "";

	   strcat(responseString, "FWV:");
	   strcat(responseString, FIRMWARE_VERSION );
	   strcat(responseString, ";");

	   strcat(responseString, "HWT:");
	   strcat(responseString, HARDWARE_TYPE );
	   strcat(responseString, ";");

	   strcat(responseString, "HWV:");
	   strcat(responseString, HARDWARE_VERSION);
	   strcat(responseString, ";");

	   sendStringWithEscapeSequence(responseString);

	   return;
   }
   if (!(strcmp(parameter, "max"))){//get parameters of MSP


   	   char responseString[COMMAND_RESPONSE_LENGTH] = "";

   	   strcat(responseString, "MSF:");
   	   strcat(responseString, MAX_SAMPLE_RATE );
   	   strcat(responseString, ";");

   	   strcat(responseString, "MNC:");
   	   strcat(responseString, MAX_NUMBER_OF_CHANNELS );
   	   strcat(responseString, ";");

   	   sendStringWithEscapeSequence(responseString);

   	   return;
      }
   if (!(strcmp(parameter, "start"))){//start sampling
	   TA0CCTL0 = CCIE;
	   return;
   }
   if (!(strcmp(parameter, "s"))){//sample rate

	   return;
   }
   if (!(strcmp(parameter, "conf s"))){//sample rate

	   return;
   }
   if (!(strcmp(parameter, "c"))){//number of channels

	   return;
   }
}


void sendStringWithEscapeSequence(char * stringToSend)
{
	char responseBufferWithEscape[COMMAND_RESPONSE_LENGTH+12] = "";

	//Start of escape sequence
	responseBufferWithEscape[0] = 255;
	responseBufferWithEscape[1] = 255;
	responseBufferWithEscape[2] = 1;
	responseBufferWithEscape[3] = 1;
	responseBufferWithEscape[4] = 128;
	responseBufferWithEscape[5] = 255;


	int i = 0;
	//copy string that we want to send
	while(stringToSend[i]!=0)
	{
		responseBufferWithEscape[i+6] = stringToSend[i]; //copy character
		i = i+1;
	}

	//End of escape sequence
	responseBufferWithEscape[i+6] = 255;
	responseBufferWithEscape[i+7] = 255;
	responseBufferWithEscape[i+8] = 1;
	responseBufferWithEscape[i+9] = 1;
	responseBufferWithEscape[i+10] = 129;
	responseBufferWithEscape[i+11] = 255;

	int length= i+12;

	//check if sampling timer is turned ON
	int weAreSendingSamples = TA0CCTL0 & CCIE;

	if(weAreSendingSamples)
	{
		//now put it to output buffer/s
		for(i=0;i<length;i++)
		{
				if(weUseBufferX==1)
				{
					bufferX[writingHeadX++] = responseBufferWithEscape[i];
					if(writingHeadX>=MEGA_DATA_LENGTH)
					{
						writingHeadY = 0;
						weUseBufferX = 0;
						weHaveDataToSend = 1;
					}
				}
				else
				{
					bufferY[writingHeadY++] = responseBufferWithEscape[i];
					if(writingHeadY>=MEGA_DATA_LENGTH)
					{
						writingHeadX = 0;
						weUseBufferX = 1;
						weHaveDataToSend = 1;
					}
				}
		}
	}
	else   //if we are not sending samples right now
	{
		//if we are not sending samples just use buffer X
		//fill it with data and trigger sending flag
		writingHeadX = 0;
		weUseBufferX = 0;//this means that buffer X is ready for sending the data

		for(i=0;i<length;i++)
		{
			bufferX[writingHeadX++] = responseBufferWithEscape[i];
			if(writingHeadX>=MEGA_DATA_LENGTH)
			{
				break;
			}
		}
		//always trigger sending flag
		weHaveDataToSend = 1;
	}
}



   /*
    * ======== UNMI_ISR ========
    */
   #if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
   #pragma vector = UNMI_VECTOR
   __interrupt void UNMI_ISR (void)
   #elif defined(__GNUC__) && (__MSP430__)
   void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
   #else
   #error Compiler not found!
   #endif
   {
       switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
       {
           case SYSUNIV_NONE:
               __no_operation();
               break;
           case SYSUNIV_NMIIFG:
               __no_operation();
               break;
           case SYSUNIV_OFIFG:
   #ifndef DRIVERLIB_LEGACY_MODE
               UCS_clearFaultFlag(UCS_XT2OFFG);
               UCS_clearFaultFlag(UCS_DCOFFG);
               SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
   #else
               UCS_clearFaultFlag(UCS_BASE, UCS_XT2OFFG);
               UCS_clearFaultFlag(UCS_BASE, UCS_DCOFFG);
               SFR_clearInterrupt(SFR_BASE, SFR_OSCILLATOR_FAULT_INTERRUPT);
   #endif
               break;
           case SYSUNIV_ACCVIFG:
               __no_operation();
               break;
           case SYSUNIV_BUSIFG:
               // If the CPU accesses USB memory while the USB module is
               // suspended, a "bus error" can occur.  This generates an NMI.  If
               // USB is automatically disconnecting in your software, set a
               // breakpoint here and see if execution hits it.  See the
               // Programmer's Guide for more information.
               SYSBERRIV = 0; //clear bus error flag
               USB_disable(); //Disable
       }
}
//Released_Version_4_20_00

//----------- PERIODIC TIMER ---------------------------

void setupPeriodicTimer()
{
	TA0CCR0 = 799;//32768=1sec;
	TA0CTL = TASSEL_2+MC_1+TACLR; // ACLK, count to CCR0 then roll, clear TAR
	//TA0CCTL0 = CCIE;
}


#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR (void)
#else
#error Compiler not found!
#endif
{
	//P4OUT ^= BIT1;
	ADC12CTL0 |= ADC12ENC + ADC12SC;
    //__bic_SR_register_on_exit(LPM3_bits);   // Exit LPM
}

//--------------------- ADC -----------------------------------------------

void setupADC()
{

	/*  P6SEL = 0x0F;                             // Enable A/D channel inputs
	  ADC12CTL0 = ADC12ON+ADC12MSC+BITA;//+BIT8;//+BIT9;//ADC12SHT0_8; // Turn on ADC12, extend sampling time
	                                            // to avoid overflow of results
	  ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3+ADC12DIV_5+ADC12SSEL_0;       // Use sampling timer, repeated sequence

	  ADC12CTL2 = ADC12RES_1;
	  ADC12MCTL0 = ADC12INCH_0+ADC12EOS;                 // ref+=AVcc, channel = A0
	 // ADC12MCTL1 = ADC12INCH_1+ADC12EOS;                 // ref+=AVcc, channel = A1
	 // ADC12MCTL2 = ADC12INCH_2;                 // ref+=AVcc, channel = A2
	 // ADC12MCTL3 = ADC12INCH_3;        // ref+=AVcc, channel = A3, end seq.
	  ADC12IE = 0x01;                           // Enable ADC12IFG.3
	  ADC12CTL0 |= ADC12ENC;                    // Enable conversions
	  ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger
*/

   P6SEL |= BIT0+BIT1+BIT2; // config first 3 channels to be used with ADC

   // ADC configuration
   //ADC12SHT02 Sampling time 16 cycles,
   //ADC12ON  ADC12 on
   ADC12CTL0 = ADC12SHT02 + ADC12ON +ADC12MSC;

   //ADC12CSTARTADD_0 start conversation address 0;
   //ADC12SHP - SAMPCON signal is sourced from the sampling timer.
   //ADC12CONSEQ_1 Sequence-of-channels (no automatic repeat)
   ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHP + ADC12CONSEQ_1;

   ADC12CTL2 = ADC12RES_1;//resolution to 10 bits
   // Use A0 as input to register 0
   ADC12MCTL0 = ADC12INCH_0;
   ADC12MCTL1 = ADC12INCH_1+ADC12EOS;
   ADC12IE = 0x02;
   // Use A0 as input to register 1
 //  ADC12MCTL1 = ADC12INCH_1;
   // Use A0 as input to register 2
   //ADC12EOS is set to 1 and it marks last conversation in sequence
 //  ADC12MCTL2 = ADC12INCH_2 + ADC12EOS;

   ADC12IE = BIT0;//trigger interupt after conversation of A2
   ADC12CTL0 &= ~ADC12SC;
 //  ADC12CTL0 |= ADC12ENC; // Enable conversion


}

#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
#else
#error Compiler not found!
#endif
{

	//============== event 1 =================

	if(debounceTimer1>0)
	{
		debounceTimer1 = debounceTimer1 -1;
	}
	else
	{
		if(eventEnabled1>0)
		{
				if(P4IN & BIT3)
				{
						eventEnabled1 = 0;
						debounceTimer1 = DEBOUNCE_TIME;
						sendStringWithEscapeSequence("EVNT:1;");
				}
		}
		else
		{
			if(!(P4IN & BIT3))
			{
				eventEnabled1 = 1;
			}
		}
	}

	//================= EVENT 2 code ======================

	if(debounceTimer2>0)
		{
			debounceTimer2 = debounceTimer2 -1;
		}
		else
		{
			if(eventEnabled2>0)
			{
					if(P4IN & BIT4)
					{
							eventEnabled2 = 0;
							debounceTimer2 = DEBOUNCE_TIME;
							sendStringWithEscapeSequence("EVNT:2;");
					}
			}
			else
			{
				if(!(P4IN & BIT4))
				{
					eventEnabled2 = 1;
				}

			}
		}


	//========== ADC code ======================



	P4OUT ^= BIT2;
	if(weUseBufferX==1)
	{
		//P4OUT ^= BIT2;
		tempIndex = writingHeadX;//remember position of begining of frame to put flag bit
		tempADCresult = ADC12MEM0;
		/*tempADCresult = counterd;
		counterd = counterd+25;
		if(counterd>1000)
		{
			counterd = 0;

		}*/
		bufferX[writingHeadX++] = (0x7u & (tempADCresult>>7));
		bufferX[writingHeadX++] = (0x7Fu & tempADCresult);
		if(writingHeadX>=MEGA_DATA_LENGTH)
		{
			writingHeadY = 0;
			weUseBufferX = 0;
			weHaveDataToSend = 1;
		}

		if(numberOfChannels>1)
		{
			tempADCresult = ADC12MEM1;
			bufferX[writingHeadX++] = (0x7u & (tempADCresult>>7));
			bufferX[writingHeadX++] = (0x7Fu & tempADCresult);
			if(writingHeadX>=MEGA_DATA_LENGTH)
			{
				writingHeadY = 0;
				weUseBufferX = 0;
				weHaveDataToSend = 1;
			}

		}

		bufferX[tempIndex] |= BIT7;//put flag for begining of frame
	}
	else
	{
		//P4OUT ^= BIT2;
		tempIndex = writingHeadY;//remember position of begining of frame to put flag bit

		tempADCresult = ADC12MEM0;
		/*tempADCresult = counterd;
		counterd = counterd+25;
		if(counterd>1000)
		{
			counterd = 0;
		}*/
		bufferY[writingHeadY++] = (0x7u & (tempADCresult>>7));
		bufferY[writingHeadY++] = (0x7Fu & tempADCresult);
		if(writingHeadY>=MEGA_DATA_LENGTH)
		{
			writingHeadX = 0;
			weUseBufferX = 1;
			weHaveDataToSend = 1;

		}

		if(numberOfChannels>1)
		{
			tempADCresult = ADC12MEM1;
			bufferY[writingHeadY++] = (0x7u & (tempADCresult>>7));
			bufferY[writingHeadY++] = (0x7Fu & tempADCresult);
			if(writingHeadY>=MEGA_DATA_LENGTH)
			{
				writingHeadX = 0;
				weUseBufferX = 1;
				weHaveDataToSend = 1;

			}

		}

		bufferY[tempIndex] |= BIT7;
	}
//Uncomment this when not using repeat of sequence
	ADC12CTL0 &= ~ADC12SC;
}
