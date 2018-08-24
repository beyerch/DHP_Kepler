/*
 * adc.c
 *
 * Created: 2/27/2017 2:28:37 PM
 *  Author: Aaron
 */ 
#include "adc.h"

int ul_mode = 0; //0 - sw trig ; 1 - tc trig
 
 void adcInit()
 {
	 sysclk_enable_peripheral_clock(ID_AFEC0);
	 struct afec_config afec_cfg;
	 afec_get_config_defaults(&afec_cfg);
	 afec_enable(AFEC0);
	 afec_init(AFEC0, &afec_cfg);
	 afec_set_trigger(AFEC0, AFEC_TRIG_SW);

	 afec_ch_set_config(AFEC0, CHANNEL_A, &afec_cfg);
	 afec_ch_set_config(AFEC0, CHANNEL_B, &afec_cfg);
	 afec_ch_set_config(AFEC0, CHANNEL_C, &afec_cfg);

	 afec_channel_set_analog_offset(AFEC0, CHANNEL_A, 0x800);
	 afec_channel_set_analog_offset(AFEC0, CHANNEL_B, 0x800);
	 afec_channel_set_analog_offset(AFEC0, CHANNEL_C, 0x800);

	 afec_channel_enable(AFEC0, CHANNEL_A);
	 afec_channel_enable(AFEC0, CHANNEL_B);
	 afec_channel_enable(AFEC0, CHANNEL_C);


	 afec_set_callback(AFEC0, AFEC_INTERRUPT_DATA_READY, ADCDataReadyCallback, 14);

	 afec_start_calibration(AFEC0);
	 while((afec_get_interrupt_status(AFEC0) & AFEC_ISR_EOCAL) != AFEC_ISR_EOCAL);

	 
 }

 afec_callback_t ADCDataReadyCallback(void)
 {
	 
	 int value = afec_get_latest_value(AFEC0);
	 
	 int chA = afec_channel_get_value(AFEC0, CHANNEL_A);
	 int chB = afec_channel_get_value(AFEC0, CHANNEL_B);
	 int chC = afec_channel_get_value(AFEC0, CHANNEL_C);

	 char ADCMessageBuffer[] = {START_BYTE, 0x00,0x07, READ_ADC_VALUE ,0x00,0x00,0x00,0x00,0x00,0x00};

	 ADCMessageBuffer[4] = ( (chA & 0xFF00) >> 8);
	 ADCMessageBuffer[5] =	(chA & 0x00FF);

	 ADCMessageBuffer[6] = ( (chB & 0xFF00) >> 8);
	 ADCMessageBuffer[7] =	(chB & 0x00FF);

	 ADCMessageBuffer[8] = ( (chC & 0xFF00) >> 8);
	 ADCMessageBuffer[9] =	(chC & 0x00FF);

	 //WriteBufferOut(ADCMessage, 10);
	 Message_t ADCMessage;
	 ADCMessage.buf = ADCMessage.buf = ADCMessageBuffer;
	 ADCMessage.Size = 10;
	 
	 WriteMessage(&ADCMessage);
 }

 uint32_t ReadADCValues (void)
 {
	 if(ul_mode == 0)
	 {
		 afec_start_software_conversion(AFEC0);
	 }else
	 {
		// uint8_t ADCError[] = {START_BYTE, 0x00,0x03,ERROR_RESPONSE, 0x00, ADC_MANUAL_TRIGGER_ERROR};
		 //WriteBufferOut(ADCError,6);
	 }

 }

 void configure_adc_tc_trigger(double PeriodMilliseconds)
 {


	 uint32_t ul_div = 0;
	 uint32_t ul_tc_clks = 0;
	 uint32_t ul_sysclk = sysclk_get_cpu_hz();
	 double ul_desiredFrequency = (1.0/ (PeriodMilliseconds/1000));
	 /* Enable peripheral clock. */
	 pmc_enable_periph_clk(ID_TC0);

	 /* Configure TC for a 1Hz frequency and trigger on RC compare. */
	 tc_find_mck_divisor(ul_desiredFrequency, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	 tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
	 TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);

	 TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / 2;
	 TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / 1;

	 /* Start the Timer. */
	 tc_start(TC0, 0);
	 ul_mode = 1;

	 uint8_t tmpRtn[] = {START_BYTE, 0x00,0x02,ENABLE_PERIODIC_ADC, 0x01};
	 //WriteBufferOut(tmpRtn,5);

	 afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);

 }

 void disable_adc_tc_trigger()
 {
	 ul_mode = 0;
	 tc_stop(TC0,0);
	 afec_set_trigger(AFEC0, AFEC_TRIG_SW);
	 uint8_t tmpRtn[] = {START_BYTE, 0x00,0x02,DISABLE_PERIODIC_ADC, 0x01};
	 //WriteBufferOut(tmpRtn,5);

 }