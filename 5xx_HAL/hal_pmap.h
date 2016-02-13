//****************************************************************************//
// Function Library for setting the Port Mapper
//    File: hal_pmap.h
//
//    Texas Instruments
//
//    Version 1.0
//    10/17/09
//
//    V1.0  Initial Version
//****************************************************************************//

#ifndef __HAL_PMAP
#define __HAL_PMAP

#include <stdint.h>

/* ***************************************************************************
 * configure_ports
 *  Configures the MSP430 Port Mapper
 *
 * \param *port_mapping:     Pointer to init Data
 * \param PxMAPy:            Pointer start of first Port Mapper to initialize
 * \param num_of_ports:      Number of Ports to initialize
 * \param port_map_reconfig: Flag to enable/disable reconfiguration
 *
*/
extern void configure_ports(const uint8_t * port_mapping, uint8_t * PxMAPy, 
                            uint8_t num_of_ports, uint8_t port_map_reconfig);

#endif // __HAL_PMAP
