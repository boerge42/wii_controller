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

#include "wii_controller.h"

// *******************************************************
uint16_t wii_controller_init(void) 
{
	uint8_t buf[WII_CONTROLLER_DATA_LEN];
	// wii-Controller initialisieren
	i2c_start_wait(WII_CONTROLLER_TWIADDR + I2C_WRITE);
	i2c_write(0xF0);
	i2c_write(0x55);
	i2c_stop();
	_delay_ms(1);
	i2c_start_wait(WII_CONTROLLER_TWIADDR + I2C_WRITE);
	i2c_write(0xFB);
	i2c_write(0x00);
	i2c_stop();
	_delay_ms(1);
	// Controller-Typ auslesen
	wii_controller_get_raw_data(0xFA, buf);
	// Controller-Typ steht im 5. und 6. empfangenen Byte
	return (buf[5] << 8) + buf[5];
}


// *******************************************************
void wii_controller_get_raw_data(uint8_t reg, uint8_t *buf)
{
	uint8_t i;
	// Datenuebertragung initiieren
	i2c_start_wait(WII_CONTROLLER_TWIADDR + I2C_WRITE);
	i2c_write(reg);
	i2c_stop();
	_delay_ms(1);
	// 6 Byte sollten ankommen...
	i2c_start_wait(WII_CONTROLLER_TWIADDR + I2C_READ);
	for(i=0; i < WII_CONTROLLER_DATA_LEN; i++) {
		if(i == WII_CONTROLLER_DATA_LEN-1)
			buf[i] = i2c_readNak();
		else
			buf[i] = i2c_readAck();
	}
	i2c_stop();	
}


// *******************************************************
nunchuck_controls_t wii_nunchuck_get_data(void)
{
	uint8_t buf[WII_CONTROLLER_DATA_LEN]; 
	nunchuck_controls_t ctrl;
	static uint8_t old_button_c =0;
	static uint8_t old_button_z =0;
	static uint8_t old_button_xl=0;
	static uint8_t old_button_xr=0;
	static uint8_t old_button_yu=0;
	static uint8_t old_button_yd=0;
	// Daten holen	
	wii_controller_get_raw_data(0x00, buf);
	// empfangene Werte in Nunchuck-Struktur uebernehmen
	ctrl.joy_x = buf[0];
	ctrl.joy_y = buf[1];
	// ...Button-Bits sind low-aktiv!
	ctrl.button_c = !((buf[5] >> 1) & 0b00000001);
	ctrl.button_z = !(buf[5]        & 0b00000001);
	// ...deren Repeat-Werte setzen
	ctrl.button_c_repeat = (ctrl.button_c && old_button_c);
	old_button_c = ctrl.button_c;
	ctrl.button_z_repeat = (ctrl.button_z && old_button_z);
	old_button_z = ctrl.button_z;
	// ..."Joystick-Button"
	ctrl.button_xl = (buf[0] < BUTTON_XL_LIMIT);
	ctrl.button_xr = (buf[0] > BUTTON_XR_LIMIT);
	ctrl.button_yu = (buf[1] > BUTTON_YU_LIMIT);
	ctrl.button_yd = (buf[1] < BUTTON_YD_LIMIT);
	// ...deren Repeat-Werte setzen
	ctrl.button_xl_repeat = (ctrl.button_xl && old_button_xl);
	old_button_xl = ctrl.button_xl;
	ctrl.button_xr_repeat = (ctrl.button_xr && old_button_xr);
	old_button_xr = ctrl.button_xr;
	ctrl.button_yu_repeat = (ctrl.button_yu && old_button_yu);
	old_button_yu = ctrl.button_yu;
	ctrl.button_yd_repeat = (ctrl.button_yd && old_button_yd);
	old_button_yd = ctrl.button_yd;
	// ...10-Bit-Beschleunigungswerte auf 2 Byte verteilt
	ctrl.accel_x = (buf[2] << 2) + ((buf[5] >> 2) & 0b00000011);
	ctrl.accel_y = (buf[3] << 2) + ((buf[5] >> 4) & 0b00000011);
	ctrl.accel_z = (buf[4] << 2) + ((buf[5] >> 6) & 0b00000011);
	return ctrl;
}

// *******************************************************
classic_controls_t  wii_classic_get_data(void)
{
	uint8_t buf[WII_CONTROLLER_DATA_LEN]; 
	classic_controls_t ctrl;
	static uint8_t old_button_bdr    =0;
	static uint8_t old_button_bdl    =0;
	static uint8_t old_button_bdd    =0;
	static uint8_t old_button_bdu    =0;
	static uint8_t old_button_bh     =0;
	static uint8_t old_button_bminus =0;
	static uint8_t old_button_bplus  =0;
	static uint8_t old_button_bx     =0;
	static uint8_t old_button_by     =0;
	static uint8_t old_button_ba     =0;
	static uint8_t old_button_bb     =0;
	static uint8_t old_button_blt    =0;
	static uint8_t old_button_brt    =0;
	static uint8_t old_button_bzl    =0;
	static uint8_t old_button_bzr    =0;
	// Daten holen	
	wii_controller_get_raw_data(0x00, buf);	
	// empfangene Werte in Classic-Struktur uebernehmen
	ctrl.button_bdr        = !((buf[4] >> 7) & 0b00000001);
	ctrl.button_bdr_repeat = (ctrl.button_bdr && old_button_bdr);
	old_button_bdr         = ctrl.button_bdr;
	ctrl.button_bdl        = !((buf[5] >> 1) & 0b00000001);
	ctrl.button_bdl_repeat = (ctrl.button_bdl && old_button_bdl);
	old_button_bdl         = ctrl.button_bdl;
	ctrl.button_bdd        = !((buf[4] >> 6) & 0b00000001);
	ctrl.button_bdd_repeat = (ctrl.button_bdd && old_button_bdd);
	old_button_bdd         = ctrl.button_bdd;
	ctrl.button_bdu        = !(buf[5]        & 0b00000001);
	ctrl.button_bdu_repeat = (ctrl.button_bdu && old_button_bdu);
	old_button_bdu         = ctrl.button_bdu;
	ctrl.button_bh         = !((buf[4] >> 3) & 0b00000001);
	ctrl.button_bh_repeat  = (ctrl.button_bh && old_button_bh);
	old_button_bh          = ctrl.button_bh;
	ctrl.button_bminus     = !((buf[4] >> 4) & 0b00000001);
	ctrl.button_bminus_repeat = (ctrl.button_bminus && old_button_bminus);
	old_button_bminus         = ctrl.button_bminus;
	ctrl.button_bplus      = !((buf[4] >> 2) & 0b00000001);
	ctrl.button_bplus_repeat = (ctrl.button_bplus && old_button_bplus);
	old_button_bplus         = ctrl.button_bplus;
	ctrl.button_bx         = !((buf[5] >> 3) & 0b00000001);
	ctrl.button_bx_repeat  = (ctrl.button_bx && old_button_bx);
	old_button_bx          = ctrl.button_bx;
	ctrl.button_by         = !((buf[5] >> 5) & 0b00000001);
	ctrl.button_by_repeat  = (ctrl.button_by && old_button_by);
	old_button_by          = ctrl.button_by;
	ctrl.button_ba         = !((buf[5] >> 4) & 0b00000001);
	ctrl.button_ba_repeat  = (ctrl.button_ba && old_button_ba);
	old_button_ba          = ctrl.button_ba;
	ctrl.button_bb         = !((buf[5] >> 6) & 0b00000001);
	ctrl.button_bb_repeat  = (ctrl.button_bb && old_button_bb);
	old_button_bb          = ctrl.button_bb;
	ctrl.button_blt        = !((buf[4] >> 5) & 0b00000001);
	ctrl.button_blt_repeat = (ctrl.button_blt && old_button_blt);
	old_button_blt         = ctrl.button_blt;
	ctrl.button_brt        = !((buf[4] >> 1) & 0b00000001);
	ctrl.button_brt_repeat = (ctrl.button_brt && old_button_brt);
	old_button_brt         = ctrl.button_brt;
	ctrl.button_bzl        = !((buf[5] >> 7) & 0b00000001);
	ctrl.button_bzl_repeat = (ctrl.button_bzl && old_button_bzl);
	old_button_bzl         = ctrl.button_bzl;
	ctrl.button_bzr        = !((buf[5] >> 2) & 0b00000001);
	ctrl.button_bzr_repeat = (ctrl.button_bzr && old_button_bzr);
	old_button_bzr         = ctrl.button_bzr;
	ctrl.joy_lx     = buf[0] & 0b00111111;
	ctrl.joy_ly     = buf[1] & 0b00111111;
	ctrl.joy_rx     = ((buf[0] >> 3) & 0b00011000) +
	                  ((buf[1] >> 5) & 0b00000110) +
	                  ((buf[2] >> 7) & 0b00000001);
	ctrl.joy_ry     = buf[2] & 0b00011111;
	ctrl.trigger_lt = ((buf[2] >> 2) & 0b00011000) + ((buf[3] >> 5) & 0b00000111);
	ctrl.trigger_rt = buf[3] & 0b00011111;
	return ctrl;	
}
