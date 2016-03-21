/* **************************************************************
 * 
 *      Wii - Controller
 *     ==================
 *      Uwe Berger; 2016 
 * 
 * Bibliothek zum Abfragen von Wii-Controller mit einem AVR.
 * 
 * Voraussetzung:
 * --------------
 * Eine I2C-Bibliothek, hier wird die von Peter Fleury verwendet.
 * http://homepage.hispeed.ch/peterfleury/doxygen/avr-gcc-libraries/group__pfleury__ic2master.html
 * 
 * 
 * Inspiration:
 * ------------
 * https://todbot.com/blog/2008/02/18/wiichuck-wii-nunchuck-adapter-available/
 * http://davidegironi.blogspot.de/2012/11/avr-atmega-wii-nunchuck-library-01.html
 * http://wiibrew.org/wiki/Wiimote/Extension_Controllers
 * 
 * ======================
 * Wii-Controller Interna
 * ======================
 * 
 * TWI-Adresse:
 * ------------
 * 0x52
 * 
 * TWI-Geschwindigkeit:
 * --------------------
 * 400kHz
 * 
 * Init-Sequenz senden:
 * --------------------
 * 0xF0
 * 0X55
 * Stop I2C-Bus
 * 0xFB
 * 0x00
 * Stop I2C-Bus
 * 
 * ...oder (geht nur bei Original-Nunchuck)
 * 0x40
 * 0x00
 * Stop I2C-Bus
 * --> dann muessen die empfangenen Datenbytes (siehe unten) vor der
 * weiteren Verarbeitung (byteweise) dekodiert werden:
 * x = (x ^ 0x17) + 0x17;
 * 
 * Controller-Typ ermitteln
 * ------------------------
 * 0xFA
 * Stop I2C-Bus
 * (ID steht im 5. und 6.Byte, der empfangenen 6 Byte)
 * 
 * Init-RAM-Pointer senden
 * -----------------------
 * 0x00
 * Stop I2C-Bus
 * 
 * Nutzdaten bestehen aus 6 Byte:
 * ------------------------------
 * ...je nach Controller-Typ, siehe auch:
 * http://wiibrew.org/wiki/Wiimote/Extension_Controllers
 * 
 * 
 * Nunchuck:
 * 0.Byte: Joy_X   [0...255]
 * 1.Byte: Joy_Y   [0...255]
 * 2.Byte: Accel_X (Bit 9...2)
 * 3.Byte: Accel_X (Bit 9...2)
 * 4.Byte: Accel_Z (Bit 9...2)
 * 5.Byte: D7      D6      D5      D4      D3      D2      D1      D0
 *         Accel_Z Accel_Z Accel_Y Accel_Y Accel_X Accel_X ButtonC ButtonZ
 *         Bit 1   Bit 0   Bit 1   Bit 0   Bit 1   Bit 0      low-aktiv
 * 
 * Classic:
 *         D7      D6      D5      D4      D3      D2      D1      D0
 * 0.Byte  <--RX[4:3]----> <--------------LX[5:0]------------------------>
 * 1.Byte  <--RX[2:1]----> <--------------LY[5:0]------------------------>
 * 2.Byte  RX[0]   <--LX[4:3]----> <--------------RY[4:0]---------------->
 * 3.Byte  <------LT[2:0]--------> <--------------RT[4:0]---------------->
 * 4.Byte  BDR     BDD     BLT     B-      BH      B+      BRT     1
 * 5.Byte  BZL     BB      BY      BA      BX      BZR     BDL     BDU        
 *
 *
 * ---------
 * Have fun! 
 * 
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 ***************************************************************
 */
#ifndef _WII_CONTROLLER_H
#define _WII_CONTROLLER_H

#include <stdint.h>
#include <util/delay.h>
#include "i2cmaster.h"

// TWI-Zeugs
#define WII_CONTROLLER_TWIADDR 	(0x52<<1)
#define WII_CONTROLLER_DATA_LEN 	6

// Controller-Typ-IDs
#define WII_CONTROLLER_NUNCHUCK		0x0000
#define WII_CONTROLLER_CLASSIC		0x0101

// Min.-/Max.-Grenzen fuer "Joystick_Button" (Nunchuck)
#define BUTTON_XL_LIMIT			60
#define BUTTON_XR_LIMIT			200
#define BUTTON_YU_LIMIT			190
#define BUTTON_YD_LIMIT			60

// Struktur Classic-Controller
typedef struct
{
	uint8_t  button_bdr;
	uint8_t  button_bdr_repeat;
	uint8_t  button_bdl;
	uint8_t  button_bdl_repeat;
	uint8_t  button_bdd;
	uint8_t  button_bdd_repeat;
	uint8_t  button_bdu;
	uint8_t  button_bdu_repeat;
	uint8_t  button_bh;
	uint8_t  button_bh_repeat;
	uint8_t  button_bminus;
	uint8_t  button_bminus_repeat;
	uint8_t  button_bplus;
	uint8_t  button_bplus_repeat;
	uint8_t  button_bx;
	uint8_t  button_bx_repeat;
	uint8_t  button_by;
	uint8_t  button_by_repeat;
	uint8_t  button_ba;
	uint8_t  button_ba_repeat;
	uint8_t  button_bb;
	uint8_t  button_bb_repeat;
	uint8_t  button_blt;
	uint8_t  button_blt_repeat;
	uint8_t  button_brt;
	uint8_t  button_brt_repeat;
	uint8_t  button_bzl;
	uint8_t  button_bzl_repeat;
	uint8_t  button_bzr;
	uint8_t  button_bzr_repeat;
	uint8_t  joy_lx;
	uint8_t  joy_ly;
	uint8_t  joy_rx;
	uint8_t  joy_ry;
	uint8_t  trigger_lt;
	uint8_t  trigger_rt;	
} classic_controls_t;

// Struktur Nunchuck-Controller
typedef struct
{
	uint8_t  button_c;
	uint8_t  button_c_repeat;
	uint8_t  button_z;
	uint8_t  button_z_repeat;
	uint8_t  button_xl;
	uint8_t  button_xl_repeat;
	uint8_t  button_xr;
	uint8_t  button_xr_repeat;
	uint8_t  button_yu;
	uint8_t  button_yu_repeat;
	uint8_t  button_yd;	
	uint8_t  button_yd_repeat;
	uint8_t  joy_x;
	uint8_t  joy_y;
	uint16_t accel_x;
	uint16_t accel_y;
	uint16_t accel_z;
} nunchuck_controls_t;


// ***********************************************
uint16_t wii_controller_init(void);
void wii_controller_get_raw_data(uint8_t reg, uint8_t *buf);
nunchuck_controls_t wii_nunchuck_get_data(void);
classic_controls_t  wii_classic_get_data(void);

#endif
