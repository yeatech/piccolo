/**********************************************************************
* File: Adc.c -- File for Lab 3 and 4 (not used in Lab 2)
* Devices: TMS320F2806x
* Author: Technical Training Organization (TTO), Texas Instruments
* 				Juan Herrero Mac�as, Universidad Polit�cnica de Madrid
* History:
*   10/14/11 - original
*   13/09/2012 - modifications for piccolo project
**********************************************************************/

#include "Lab.h"				// Main include file


/**********************************************************************
* Function: InitAdc()
*
* Description: Initializes the ADC on the F2806x
**********************************************************************/
void InitAdc(void)
{
	asm(" EALLOW");						// Enable EALLOW protected register access
	
//--- Power-up and configure the ADC
	AdcRegs.ADCCTL1.all = 0x00E4;		// Power-up reference and main ADC
// bit 15        0:      RESET, ADC software reset, 0=no effect, 1=resets the ADC
// bit 14        0:      ADCENABLE, ADC enable, 0=disabled, 1=enabled
// bit 13        0:      ADCBSY, ADC busy, read-only
// bit 12-8      0's:    ADCBSYCHN, ADC busy channel, read-only
// bit 7         1:      ADCPWDN, ADC power down, 0=powered down, 1=powered up
// bit 6         1:      ADCBGPWD, ADC bandgap power down, 0=powered down, 1=powered up 
// bit 5         1:      ADCREFPWD, ADC reference power down, 0=powered down, 1=powered up 
// bit 4         0:      reserved
// bit 3         0:      ADCREFSEL, ADC reference select, 0=internal, 1=external
// bit 2         1:      INTPULSEPOS, INT pulse generation, 0=start of conversion, 1=end of conversion
// bit 1         0:      VREFLOCONV, VREFLO convert, 0=VREFLO not connected, 1=VREFLO connected to B5
// bit 0         0:      TEMPCONV, Temperature sensor convert.  0=ADCINA5 is pin, 1=ADCINA5 is temp sensor

	AdcRegs.ADCCTL2.all = 0x0001;		// ADC clock configuration
// bit 15-3      0's:    reserved
// bit 2         0:      CLKDIV4EN, ADC clock divider.  0=no effect, 1=CPUCLK/4 if CLKDIV2EN=1 (else no effect)
// bit 1         0:      ADCNONOVERLAP, 0=overlap sample and conversion, 1=no overlap 
// bit 0         1:      CLKDIV2EN, ADC clock divider.  0=CPUCLK, 1=CPUCLK/2

	DelayUs(1000);						// Wait 1 ms after power-up before using the ADC

	//--- Configure SOC0 to proc when its software forced in TZINT1

		AdcRegs.ADCSAMPLEMODE.bit.SIMULEN0 = 0;		// SOC0/1 in single sample mode (vs. simultaneous mode)

		AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 0;			// Trigger using software only
		AdcRegs.ADCSOC0CTL.bit.CHSEL = 0;			// Convert channel ADCINA0 (ch0)
		AdcRegs.ADCSOC0CTL.bit.ACQPS = 15;			// Acquisition window set to (49+1)=50 cycles
				// Each ADC clock cycle lasts 25ns --> 50 * 25 = 1250 ns time to read the input voltage

		AdcRegs.ADCINTSOCSEL1.bit.SOC0 = 0;			// No ADCINT triggers SOC0.  TRIGSEL field determines trigger.

		AdcRegs.SOCPRICTL.bit.SOCPRIORITY = 0;		// All SOCs handled in round-robin mode

	// Configure SOC1, SOC2 and SOC3 as SOC0 to oversample x4 the measure

//		AdcRegs.ADCSAMPLEMODE.bit.SIMULEN2 = 0;		// SOC2/3 in single sample mode (vs. simultaneous mode)
//
//		AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 0;			// Trigger using software only
//		AdcRegs.ADCSOC2CTL.bit.TRIGSEL = 0;			// Trigger using software only
//		AdcRegs.ADCSOC3CTL.bit.TRIGSEL = 0;			// Trigger using software only
//
//		AdcRegs.ADCSOC1CTL.bit.CHSEL = 0;			// Convert channel ADCINA0 (ch0)
//		AdcRegs.ADCSOC2CTL.bit.CHSEL = 0;			// Convert channel ADCINA0 (ch0)
//		AdcRegs.ADCSOC3CTL.bit.CHSEL = 0;			// Convert channel ADCINA0 (ch0)
//
//		AdcRegs.ADCSOC1CTL.bit.ACQPS = 49;			// Acquisition window set to (49+1)=50 cycles
//		AdcRegs.ADCSOC2CTL.bit.ACQPS = 49;			// Acquisition window set to (49+1)=50 cycles
//		AdcRegs.ADCSOC3CTL.bit.ACQPS = 49;			// Acquisition window set to (49+1)=50 cycles
//
//		AdcRegs.ADCINTSOCSEL1.bit.SOC1 = 0;			// No ADCINT triggers SOC0.  TRIGSEL field determines trigger.
//		AdcRegs.ADCINTSOCSEL1.bit.SOC2 = 0;			// No ADCINT triggers SOC0.  TRIGSEL field determines trigger.
//		AdcRegs.ADCINTSOCSEL1.bit.SOC3 = 0;			// No ADCINT triggers SOC0.  TRIGSEL field determines trigger.


	//--- ADCINT1 configuration
		AdcRegs.INTSEL1N2.bit.INT1CONT = 1;			// ADCINT1 pulses regardless of ADCINT1 flag state
		AdcRegs.INTSEL1N2.bit.INT1E = 1;			// Enable ADCINT1
		AdcRegs.INTSEL1N2.bit.INT1SEL = 0;			// EOC0 triggers ADCINT1

	//--- Enable the ADC interrupt
		PieCtrlRegs.PIEIER1.bit.INTx1 = 1;			// Enable ADCINT1 in PIE group 1
		IER |= 0x0001;								// Enable INT1 in IER to enable PIE group

	//--- Comparator 1 Configuration --> If (input + > input -) --> CompOut = 1
														// If (input + < input -) --> CompOut = 0
		Comp1Regs.COMPCTL.bit.COMPDACEN = 1; // Enable Comparator 1
		Comp1Regs.COMPCTL.bit.CMPINV = 0; // If you want to invert the output
		Comp1Regs.COMPCTL.bit.COMPSOURCE = 0;	// 1  Both comparator inputs connected to external pin
																			// 0 Input - is generated internally via DAC and compared to an input  + which is an external pin
		Comp1Regs.COMPCTL.bit.SYNCSEL = 0; //Asynchronous Comp1 Output
		Comp1Regs.COMPCTL.bit.QUALSEL =  0; // Don't Care

		Comp1Regs.DACVAL.bit.DACVAL = 300; // 10 bits [0-1023] Not used this time. Generates V = DACVAL * 3.3 / 1023 on DAC signal

		 AdcRegs.ADCCTL1.bit.ADCBGPWD = 1; // It's already been enabled, but it's necessary to enable it for the comparator to work

//--- Finish up
	AdcRegs.ADCCTL1.bit.ADCENABLE = 1;	// Enable the ADC
	asm(" EDIS");						// Disable EALLOW protected register access

} // end InitAdc()


//--- end of file -----------------------------------------------------
