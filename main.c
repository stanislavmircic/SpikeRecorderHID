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
#define FIRMWARE_VERSION "0.02"  // firmware version. Try to keep it to 4 characters
#define HARDWARE_TYPE "NEURONSB" // hardware type/product. Try not to go over 8 characters
#define HARDWARE_VERSION "0.5"  // hardware version. Try to keep it to 4 characters
#define COMMAND_RESPONSE_LENGTH 35  //16 is just the delimiters etc.
#define DEBOUNCE_TIME 2000
#define MAX_SAMPLE_RATE "10000" //this will be sent to Host when host asks for maximal ratings
#define MAX_NUMBER_OF_CHANNELS "2" //this will be sent to Host when host asks for maximal ratings

//operation modes
#define OPERATION_MODE_DEFAULT 0
#define OPERATION_MODE_BNC 1
#define OPERATION_MODE_REACTION_TIMER 2
#define OPERATION_MODE_DEV_BOARD 3

#define GREEN_LED BIT0
#define RED_LED BIT1
#define RELAY_OUTPUT BIT7

#define IO1 BIT2
#define IO2 BIT3
#define IO3 BIT4
#define IO4 BIT5
#define IO5 BIT6

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

//used for events in normal mode and reaction timer
unsigned int debounceTimer1 = 0;
unsigned int debounceTimer2 = 0;
unsigned int eventEnabled1 = 1;
unsigned int eventEnabled2 = 1;


//reaction timer mode
unsigned int reactionTimerMode = 0;
unsigned int mainStimulusRTCounter = 0;
unsigned int durationStimulusRTCounter = 0;
#define STIMULUS_DELAY 40000
#define STIMULUS_DURATION 2200
#define RT_FREQUENCY_FIRST 10
#define RT_FREQUENCY_SECOND 20
unsigned int RTSpeakerFrequency = 10;
unsigned int RTFrequencyGeneratorCounter = 0;
unsigned int stimulusChoosen =0;




//changes when we detect board
unsigned int operationMode = 0;

//used to detect changing in voltage on board detection
int lastEncoderVoltage = 0;
//used to detect changing in voltage on board detection
int currentEncoderVoltage = 0;
//measure time after voltage changes on board detection input
unsigned int debounceEncoderTimer = 0;
//flag that is active when board detection voltage stabilize
unsigned int encoderEnabled = 0;

unsigned int sampleData = 0;


#define BOARD_DETECTION_TIMER_MAX_VALUE 15000 //1.5 sec


void defaultSetupADC();
void setupPeriodicTimer();
void sendStringWithEscapeSequence(char * stringToSend);
void executeCommand(char * command);
void setupOperationMode(void);

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
    
       initPorts();            // Config GPIOS for low-power (output low)
       initClocks(16000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
       USB_setup(TRUE, TRUE);  // Init USB & events; if a host is present, connect
       operationMode = OPERATION_MODE_DEFAULT;
       setupOperationMode();   //setup GPIO
       defaultSetupADC();
       setupPeriodicTimer();   //setup periodic timer for sampling

       counterd = 0;

       //setup buffers variables for ADC sampling
       weUseBufferX = 1;
       writingHeadY = 0;
       writingHeadX = 0;

       //prepare state for USB
       bHIDDataSent_event = TRUE;

       // Pre-fill the buffers with visible ASCII characters (0x21 to 0x7E)
       y = 0x21;
       for (w = 0; w < MEGA_DATA_LENGTH; w++)
       {
           bufferX[w] = y;
           bufferY[w] = y;
       }

       //LED diode (BIT0, BIT1) and relay (BIT7)
       P4SEL = 0;//digital I/O
       P4DIR = GREEN_LED + RED_LED + RELAY_OUTPUT;
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

            	  // TA0CCTL0 &= ~CCIE;//stop timer -> this will stop sampling -> this will stop stream
            	   //prepare buffer for next connection
            	   sampleData = 0;
            	   P4OUT &= ~(RED_LED);
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
// Setup operation mode inputs outputs and state
//
void setupOperationMode(void)
{
	__disable_interrupt();
	switch(operationMode)
	{
		case OPERATION_MODE_BNC:
			P6SEL = BIT0+BIT1+BIT7;//analog inputs
			P6DIR = 0;//select all as inputs
			P6OUT = 0;//put output register to zero
			P4OUT |= RELAY_OUTPUT + GREEN_LED;
			//default setup of ADC, redefines part of Port 6 pins
			//defaultSetupADC();
		break;
		case OPERATION_MODE_REACTION_TIMER:
			P6SEL = BIT0+BIT1+BIT7;//select analog inputs
			//select two swithes to inputs (BIT6 and BIT5)
			//rest is output (BIT2 - speaker, BIT3 & BIT4 are LEDs)
			P6DIR = IO1+IO2+IO3;
			P6OUT = 0;//put output register to zero

			P4OUT &= ~(RELAY_OUTPUT + GREEN_LED);
			P4OUT |=  GREEN_LED;
			//default setup of ADC, redefines part of Port 6 pins
			//defaultSetupADC();
		break;
		case OPERATION_MODE_DEFAULT:
					P6SEL = BIT0+BIT1+BIT7;//select all pins as digital I/O
					P6DIR = 0;//select all as inputs
					P6OUT = 0;//put output register to zero

					P4OUT &= ~(RELAY_OUTPUT + GREEN_LED);
					//default setup of ADC, redefines part of Port 6 pins
					//defaultSetupADC();
				break;
		default:

			P6SEL = BIT0+BIT1+BIT7;//select all pins as digital I/O
			P6DIR = 0;//select all as inputs
			P6OUT = 0;//put output register to zero

			P4OUT &= ~(RELAY_OUTPUT + GREEN_LED);
			//default setup of ADC, redefines part of Port 6 pins
			//defaultSetupADC();

		break;
	}
	__enable_interrupt();
}


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
	 //  TA0CCTL0 &= ~CCIE;
	   sampleData = 0;
	   P4OUT &= ~(RED_LED);
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
	   //TA0CCTL0 = CCIE;
	   sampleData = 1;
	   P4OUT |= RED_LED;
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
	TA0CCTL0 = CCIE;
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

void defaultSetupADC()
{
   //select recording inputs and board detection input(A7)
   //as analog inputs
   P6SEL |= BIT0+BIT1+BIT7;

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
   ADC12MCTL0 = ADC12INCH_0;//recording channel
   ADC12MCTL1 = ADC12INCH_1;//recording channel
   ADC12MCTL2 = ADC12INCH_7+ADC12EOS;//board detection input
   //ADC12IE = 0x02;//enable interrupt on ADC12IFG2 bit

   ADC12IE = BIT0;//trigger interrupt after conversation of A2
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


	// ------------------- BOARD DETECTION -----------------------------

	currentEncoderVoltage = ADC12MEM2;

	if(debounceEncoderTimer>0)
	{
		debounceEncoderTimer = debounceEncoderTimer -1;
	}
	else
	{
		//if board detection voltage is changing
		if((currentEncoderVoltage - lastEncoderVoltage)>100 || (lastEncoderVoltage - currentEncoderVoltage)>100)
		{
			debounceEncoderTimer = BOARD_DETECTION_TIMER_MAX_VALUE;
		}
	}

	if(debounceEncoderTimer==0)
	{
		if(currentEncoderVoltage < 93 )
		{
			//default
			if(operationMode != OPERATION_MODE_DEFAULT)
			{
				operationMode = OPERATION_MODE_DEFAULT;
				setupOperationMode();

			}

		}
		else if((currentEncoderVoltage >= 93) && (currentEncoderVoltage < 232))
		{
			//first board BNC
			if(operationMode != OPERATION_MODE_BNC)
			{
				operationMode = OPERATION_MODE_BNC;
				setupOperationMode();

			}

		}
		else if((currentEncoderVoltage >= 232) && (currentEncoderVoltage < 384))
		{
			//second board - Reaction timer

			if(operationMode != OPERATION_MODE_REACTION_TIMER)
			{
				operationMode = OPERATION_MODE_REACTION_TIMER;
				setupOperationMode();

			}
		}
		else if((currentEncoderVoltage >= 384) && (currentEncoderVoltage < 543))
		{
			//third board

		}
		else if((currentEncoderVoltage >= 543) && (currentEncoderVoltage < 698))
		{
			//forth board

		}
		else if((currentEncoderVoltage >= 698) && (currentEncoderVoltage < 853))
		{
			//fifth board

		}
		else if((currentEncoderVoltage >= 853))
		{
			//sixth board

		}
	}

	lastEncoderVoltage = currentEncoderVoltage;


	//--------------------- BOARD EXECUTION -------------------------------


	switch(operationMode)
		{

			case OPERATION_MODE_REACTION_TIMER:
				//============== button 1 - mode selection RT =================

				if(debounceTimer1>0)
				{
					debounceTimer1 = debounceTimer1 -1;
				}
				else
				{
					if(eventEnabled1>0)
					{
							if(P6IN & IO5)
							{
									eventEnabled1 = 0;
									debounceTimer1 = DEBOUNCE_TIME;
									reactionTimerMode = 0;
									mainStimulusRTCounter = STIMULUS_DELAY - 150;
									P6OUT &= ~(IO3 + IO2 + IO1);//reset here because of speaker

									//sendStringWithEscapeSequence("EVNT:1;");
							}
					}
					else
					{
						if(!(P6IN & IO5))
						{
							eventEnabled1 = 1;
						}
					}
				}

				//================= button 2 RT mode selection ======================

				if(debounceTimer2>0)
					{
						debounceTimer2 = debounceTimer2 -1;
					}
					else
					{
						if(eventEnabled2>0)
						{
								if(P6IN & IO4)
								{
										eventEnabled2 = 0;
										debounceTimer2 = DEBOUNCE_TIME;
										reactionTimerMode = 1;
										mainStimulusRTCounter = STIMULUS_DELAY - 150;
										P6OUT &= ~(IO3 + IO2 + IO1);//reset here because of speaker
										//sendStringWithEscapeSequence("EVNT:2;");
								}
						}
						else
						{
							if(!(P6IN & IO4))
							{
								eventEnabled2 = 1;
							}

						}
					}

			//----------- Reaction Timer functionality based on mode------------------------
				mainStimulusRTCounter++;

				if(STIMULUS_DELAY == mainStimulusRTCounter)
				{
					mainStimulusRTCounter = 0;
					stimulusChoosen = currentEncoderVoltage & BIT0;
					if(reactionTimerMode)
					{
						//sound mode
						if(stimulusChoosen)
						{
							RTSpeakerFrequency = RT_FREQUENCY_FIRST;
							sendStringWithEscapeSequence("EVNT:1;");
						}
						else
						{
							RTSpeakerFrequency = RT_FREQUENCY_SECOND;
							sendStringWithEscapeSequence("EVNT:2;");
						}
						RTFrequencyGeneratorCounter = 0;
						P6OUT ^= IO1;
					}
					else
					{
						//LED mode
						if(stimulusChoosen)
						{
							P6OUT |= IO2;
							sendStringWithEscapeSequence("EVNT:1;");
						}
						else
						{
							P6OUT |= IO3;
							sendStringWithEscapeSequence("EVNT:2;");
						}

					}

					//set stimulus duration count down timer
					durationStimulusRTCounter = STIMULUS_DURATION;
				}


				if(durationStimulusRTCounter==0)
				{
					//set LEDs and Speeker on zerro when stimulus duration runs out
					P6OUT &= ~(IO3 + IO2 + IO1);
				}
				else
				{
					//count down stimulus duration - decrement
					durationStimulusRTCounter--;
					if(reactionTimerMode)
					{
						RTFrequencyGeneratorCounter++;
						if(RTFrequencyGeneratorCounter == RTSpeakerFrequency)
						{
							RTFrequencyGeneratorCounter = 0;
							P6OUT ^= IO1;
						}
					}
				}


			break;
			case OPERATION_MODE_BNC:
			default:
				//============== event 1 =================

					if(debounceTimer1>0)
					{
						debounceTimer1 = debounceTimer1 -1;
					}
					else
					{
						if(eventEnabled1>0)
						{
								if(P6IN & IO1)
								{
										eventEnabled1 = 0;
										debounceTimer1 = DEBOUNCE_TIME;
										sendStringWithEscapeSequence("EVNT:1;");
								}
						}
						else
						{
							if(!(P6IN & IO1))
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
									if(P6IN & IO2)
									{
											eventEnabled2 = 0;
											debounceTimer2 = DEBOUNCE_TIME;
											sendStringWithEscapeSequence("EVNT:2;");
									}
							}
							else
							{
								if(!(P6IN & IO2))
								{
									eventEnabled2 = 1;
								}

							}
						}

		}

	//========== ADC code ======================

if(sampleData == 1)
{
	if(weUseBufferX==1)
	{
		tempIndex = writingHeadX;//remember position of begining of frame to put flag bit
		tempADCresult = ADC12MEM0;


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

		tempIndex = writingHeadY;//remember position of begining of frame to put flag bit

		tempADCresult = ADC12MEM0;


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
}
else
{
	tempADCresult = ADC12MEM0;
	tempADCresult = ADC12MEM1;
}
//Uncomment this when not using repeat of sequence
	ADC12CTL0 &= ~ADC12SC;
}
