/*
 * adc.h
 *
 * Created: 2/27/2017 2:28:45 PM
 *  Author: Aaron
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <asf.h>


#define CHANNEL_A AFEC_CHANNEL_3
#define CHANNEL_B AFEC_CHANNEL_4
#define CHANNEL_C AFEC_CHANNEL_5

void adcInit(void);
uint32_t ReadADCValues (void);
afec_callback_t ADCDataReadyCallback(void);
void configure_adc_tc_trigger(double PeriodMilliseconds);
void disable_adc_tc_trigger();

#endif /* ADC_H_ */