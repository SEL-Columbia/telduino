
/*
 * Copyright (c) 2006-2007 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */
#include "Select/select.h"

#ifndef SD_RAW_CONFIG_H
#define SD_RAW_CONFIG_H

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SD_RAW_WRITE_SUPPORT 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SD_RAW_WRITE_SUPPORT is 0.
 */
#define SD_RAW_WRITE_BUFFERING 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SD_RAW_WRITE_SUPPORT is 1, SD_RAW_SAVE_RAM will
 *       be reset to 0.
 */
#define SD_RAW_SAVE_RAM 1

/* defines for customisation of sd/mmc port access */
#if defined(__AVR_ATmega8__) || \
    defined(__AVR_ATmega48__) || \
    defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__)
    #define configure_pin_mosi() DDRB |= (1 << DDB3)
    #define configure_pin_sck() DDRB |= (1 << DDB5)
    #define configure_pin_ss() DDRB |= (1 << DDB2)
    #define configure_pin_miso() DDRB &= ~(1 << DDB4)

    #define select_card() PORTB &= ~(1 << PB2)
    #define unselect_card() PORTB |= (1 << PB2)
#elif defined(__AVR_ATmega16__) || \
      defined(__AVR_ATmega32__)
    #define configure_pin_mosi() DDRB |= (1 << DDB5)
    #define configure_pin_sck() DDRB |= (1 << DDB7)
    #define configure_pin_ss() DDRB |= (1 << DDB4)
    #define configure_pin_miso() DDRB &= ~(1 << DDB6)

    #define select_card() PORTB &= ~(1 << PB4)
    #define unselect_card() PORTB |= (1 << PB4)
#elif defined(__AVR_ATmega64__) || \
      defined(__AVR_ATmega128__) || \
      defined(__AVR_ATmega169__) || \
      defined(__AVR_ATmega1280__)
    #define configure_pin_mosi() DDRB |= (1 << DDB2)
    #define configure_pin_sck() DDRB |= (1 << DDB1)
    //#define configure_pin_ss() DDRB |= (1 << DDB0)   //Must change, sets PB0 to output 
    #define configure_pin_ss() initSelect()//DDRL |= (1 << DDL2)   //Must change, sets PL2 to output 
    #define configure_pin_miso() DDRB &= ~(1 << DDB3)

    #define select_card()   CSSelectDevice(SDCARD)//PORTL &= ~(1 << PL2)     //Changed, Sets PL2 to low
    #define unselect_card() CSSelectDevice(DEVDISABLE)//PORTL |= (1 << PL2)      //Changed, sets PL2 to high
#else
    #error "no sd/mmc pin mapping available!"
#endif


//The SD Card is always considered available, for now
#define configure_pin_available() //DDRC &= ~(1 << DDC4)
#define configure_pin_locked() //DDRC &= ~(1 << DDC5)

#define get_pin_available() 0//((PINC >> PC4) & 0x01) //Always available
#define get_pin_locked() 1//((PINC >> PC5) & 0x01) //Always unlocked

/**
 * @}
 */

/* configuration checks */
#if SD_RAW_WRITE_SUPPORT
#undef SD_RAW_SAVE_RAM
#define SD_RAW_SAVE_RAM 0
#else
#undef SD_RAW_WRITE_BUFFERING
#define SD_RAW_WRITE_BUFFERING 0
#endif

#endif

