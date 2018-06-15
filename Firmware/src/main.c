/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "KeplerConfiguration.h"
#include "runtimer.h"
#include "console.h"


int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

    //Initialize interrupt vectors
	irq_initialize_vectors();
	//Enable interrupt controller
	cpu_irq_enable();
	//Setup and switch to PLL controlled clock
	sysclk_init();
	//Initalize all board systems
	board_init();
	//Debug console
	WriteLine("Welcome - Kepler Debug Console - V1.0");
	WriteLine("Board Initialization...OK");

	//Initialize Kepler Configuration
	SystemConfiguration.pc_com_mode = 0;
	SystemConfiguration.vehicle_com_mode = 0xFF;
	SystemConfiguration.bluetooth_unlocked = false;
	SystemConfiguration.in_secure_mode = false;
	SystemConfiguration.isotp_mode = 0;
	
	WriteLine("System Configuration...OK");
	WriteLine("System...RUNNING");
	
	//Loop and handle commands forever
	while(1)
	{
	 	if(MessageAvailable == 1)
		{
			//Got a message, handle it
			HandleMessage(&IncommingMessage);
			MessageAvailable = 0;
		}
	}
}
