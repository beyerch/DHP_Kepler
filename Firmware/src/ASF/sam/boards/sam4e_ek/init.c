/**
 * \file
 *
 * \brief SAM4E-EK board init.
 *
 * Copyright (c) 2012-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <asf.h>
#include "compiler.h"
#include "board.h"
#include "conf_board.h"
#include "ioport.h"
#include "ui.h"
#include "console.h"
#include "adc.h"
/**
 * \brief Set peripheral mode for IOPORT pins.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param port IOPORT port to configure
 * \param masks IOPORT pin masks to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_port_peripheral_mode(port, masks, mode) \
	do {\
		ioport_set_port_mode(port, masks, mode);\
		ioport_disable_port(port, masks);\
	} while (0)

/**
 * \brief Set peripheral mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_pin_peripheral_mode(pin, mode) \
	do {\
		ioport_set_pin_mode(pin, mode);\
		ioport_disable_pin(pin);\
	} while (0)

/**
 * \brief Set input mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 * \param sense Sense for interrupt detection (\ref ioport_sense)
 */
#define ioport_set_pin_input_mode(pin, mode, sense) \
	do {\
		ioport_set_pin_dir(pin, IOPORT_DIR_INPUT);\
		ioport_set_pin_mode(pin, mode);\
		ioport_set_pin_sense_mode(pin, sense);\
	} while (0)
	
void initalize_hsmci(void)
{
	/* Configure HSMCI pins */
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCDA_GPIO, PIN_HSMCI_MCCDA_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCK_GPIO, PIN_HSMCI_MCCK_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA0_GPIO, PIN_HSMCI_MCDA0_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA1_GPIO, PIN_HSMCI_MCDA1_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA2_GPIO, PIN_HSMCI_MCDA2_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA3_GPIO, PIN_HSMCI_MCDA3_FLAGS);

	/* Configure SD/MMC card detect pin */
	ioport_set_pin_dir(SD_MMC_0_CD_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(SD_MMC_0_CD_GPIO, SD_MMC_0_CD_FLAGS);
}

void initalize_can0(void)
{
  	
	ioport_set_pin_dir(MSC_NEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(MSC_NRM, IOPORT_DIR_OUTPUT);

	ioport_set_pin_dir(HSC_NEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(HSC_NRM, IOPORT_DIR_OUTPUT);

	ioport_set_pin_dir(SWC_MODE_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(SWC_MODE_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(SWC_RX_EN, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_peripheral_mode(PIN_CAN0_RX_IDX, PIN_CAN0_RX_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_CAN0_TX_IDX, PIN_CAN0_TX_FLAGS);
	
	
	can_disable_interrupt(SYSTEM_CAN, CAN_DISABLE_ALL_INTERRUPT_MASK);
	can_reset_all_mailbox(SYSTEM_CAN);
	NVIC_EnableIRQ(SYSTEM_CAN_IRQ);
	
	
	
}
void initalize_leds(void)
{
	ioport_set_pin_dir(POWER_LED_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(POWER_LED_GREEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(POWER_LED_BLUE, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_dir(VEHICLE_LED_GREEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(VEHICLE_LED_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(VEHICLE_LED_BLUE, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_dir(PC_LED_GREEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(PC_LED_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(PC_LED_BLUE, IOPORT_DIR_OUTPUT);
	
	ioport_set_pin_level(POWER_LED_RED, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(POWER_LED_GREEN, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(POWER_LED_BLUE, IOPORT_PIN_LEVEL_HIGH);
	 
	ioport_set_pin_level(VEHICLE_LED_GREEN, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(VEHICLE_LED_RED, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(VEHICLE_LED_BLUE, IOPORT_PIN_LEVEL_HIGH);
	 
	ioport_set_pin_level(PC_LED_GREEN, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(PC_LED_RED, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(PC_LED_BLUE, IOPORT_PIN_LEVEL_HIGH);
	
}

void initalize_j1850_vpw(void)
{
	ioport_set_pin_mode(J1850_P_TX, IOPORT_MODE_MUX_A);
	ioport_set_pin_dir(J1850_P_TX, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(J1850_P_TX, IOPORT_PIN_LEVEL_LOW);
	
}

void board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
#endif

	/* Initialize IOPORTs */
	ioport_init();
	
	ioport_set_port_peripheral_mode(PINS_UART0_PORT, PINS_UART0, PINS_UART0_FLAGS);
	InitalizeDebugConsole();
	initalize_leds();
	ui_init();
	sleepmgr_init();
	initalize_hsmci();
	
	initalize_j1850_vpw();
	initalize_can0();
	sysclk_enable_peripheral_clock(ID_EFC);
	adcInit();
	udc_start();
	
	ui_power_good();
}
