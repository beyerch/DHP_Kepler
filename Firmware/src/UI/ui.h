/*
 * ui.h
 *
 * Created: 2/24/2017 11:14:15 PM
 *  Author: adeck
 */ 


#ifndef UI_H_
#define UI_H_
#include "compiler.h"
//! \brief Initializes the user interface
void ui_init(void);

//! \brief Enters the user interface in power down mode
void ui_powerdown(void);

//! \brief Exits the user interface of power down mode
void ui_wakeup(void);

//! \name Callback to show the MSC read and write access
//! @{
void ui_mmc_start_read(void);
void ui_mmc_stop_read(void);
void ui_mmc_start_write(void);
void ui_mmc_stop_write(void);
//! @}

/*! \brief Called when communication port is opened
 */
void ui_com_open(uint8_t port);

/*! \brief Called when communication port is closed
 */
void ui_com_close(uint8_t port);

/*! \brief Called when a data is received on CDC
 */
void ui_com_rx_start(void);

/*! \brief Called when a data is received on port com
 */
void ui_com_tx_start(void);

/*! \brief Called when all data pending are sent on port com
 */
void ui_com_rx_stop(void);

/*! \brief Called when all data pending are sent on CDC
 */
void ui_com_tx_stop(void);

/*! \brief Called when a communication error occur
 */
void ui_com_error(void);

/*! \brief Called when a overflow occur
 */
void ui_com_overflow(void);

/*! \brief This process is called each 1ms
 * It is called only if the USB interface is enabled.
 *
 * \param framenumber  Current frame number
 */
void ui_com_process(uint16_t framenumber);

void ui_power_good(void);

void ui_vehicle_enable_vpw(void);

void ui_vehicle_disable_vpw(void);
void ui_vehicle_vpw_tx_notify(void);
void ui_vehicle_vpw_rx_notify(void);
void ui_vehicle_vpw_tx_notify_off(void);
void ui_vehicle_vpw_rx_notify_off(void);
void ui_vehicle_enable_can(void);
void ui_vehicle_disable_can(void);




#endif /* UI_H_ */