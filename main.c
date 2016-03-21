/* **************************************************************
 * 
 *      Wii - Controller
 *     ==================
 *      Uwe Berger; 2016 
 * 
 * Ein Beispielprogramm, in dem die Zustaende der Sensoren
 * diverser Wii-Controller (via I2C) abgefragt und visualieriert
 * werden.
 * 
 * ---------
 * Have fun! 
 * 
 ***************************************************************
 */

#include <stdio.h>
#include <util/delay.h>

#include <limits.h>

#include "i2cmaster.h"
#include "mcurses.h"
#include "wii_controller.h"

nunchuck_controls_t nc_ctrl;
classic_controls_t  cl_ctrl;


// ***************************************************
void output_nunchuck_screen(nunchuck_controls_t *c)
{
	char buf[30];
	attrset(A_NORMAL);
	sprintf(buf, " C: %1u      Z: %1u", c->button_c, c->button_z);
	mvaddstr(3, 1, buf);	
	sprintf(buf, "XL: %1u     XR: %1u", c->button_xl, c->button_xr);
	mvaddstr(4, 1, buf);	
	sprintf(buf, "YU: %1u     YD: %1u", c->button_yu, c->button_yd);
	mvaddstr(5, 1, buf);	
	sprintf(buf, " x: %3u    y: %3u", c->joy_x, c->joy_y);
	mvaddstr(8, 1, buf);	
	sprintf(buf, " x: %4u   y: %4u  z: %4u", 
	             c->accel_x, c->accel_y, c->accel_z);
	mvaddstr(11, 1, buf);	
}

// ***************************************************
void output_classic_screen(classic_controls_t *c)
{
	char buf[40];
	attrset(A_NORMAL);
	sprintf(buf, " BDL: %1u  BDR: %1u  BDU: %1u  BDD: %1u", 
	             c->button_bdl, c->button_bdr, c->button_bdu, c->button_bdd);
	mvaddstr(3, 1, buf);	
	sprintf(buf, "  BY: %1u   BX: %1u   BB: %1u   BA: %1u", 
	             c->button_by, c->button_bx, c->button_bb, c->button_ba);
	mvaddstr(4, 1, buf);	
	sprintf(buf, "  B-: %1u   BH: %1u   B+: %1u", 
	             c->button_bminus, c->button_bh, c->button_bplus);
	mvaddstr(5, 1, buf);	
	sprintf(buf, " BZL: %1u  BZR: %1u  BLT: %1u  BRT: %1u", 
	             c->button_bzl, c->button_bzr, c->button_blt, c->button_brt);
	mvaddstr(6, 1, buf);	
	sprintf(buf, "  lx: %3u   ly: %3u", c->joy_lx, c->joy_ly);
	mvaddstr(9, 1, buf);	
	sprintf(buf, "  rx: %3u   ry: %3u", c->joy_rx, c->joy_ry);
	mvaddstr(10, 1, buf);	
	sprintf(buf, "  lt: %3u   rt: %3u", c->trigger_lt, c->trigger_rt);
	mvaddstr(13, 1, buf);	
}

// ***************************************************
char *int2bin(uint8_t n, char *buf)
{
    #define BITS (sizeof(n) * CHAR_BIT)
    static char static_buf[BITS + 1];
    int i;
    if (buf == NULL)
        buf = static_buf;
    for (i = BITS - 1; i >= 0; --i) {
        buf[i] = (n & 1) ? '1' : '0';
        n >>= 1;
    }
    buf[BITS] = '\0';
    return buf;
    #undef BITS
}

// ***************************************************
// ***************************************************
// ***************************************************
int main (void)
{	
	uint8_t buf[WII_CONTROLLER_DATA_LEN]; 
	uint16_t controller_typ;
	char sbuf[30];

	i2c_init();	
    initscr ();
   	curs_set(0);
   	// wii-Kontroller initialisieren und Typ bestimmen
   	controller_typ = wii_controller_init();
	// je nach Controllertyp Bildschirm aufbauen etc.
	switch (controller_typ) {
		case WII_CONTROLLER_NUNCHUCK:
			attrset(A_REVERSE);
			mvaddstr(0,  0, "        Wii - Nunchuck        ");
			attrset(A_BOLD);
    		mvaddstr(2, 1,  "Tasten");
    		mvaddstr(7, 1,  "Joystick");
    		mvaddstr(10, 1, "Beschleunigung");
   			attrset(A_NORMAL);
			nc_ctrl = wii_nunchuck_get_data();
			output_nunchuck_screen(&nc_ctrl);
    		while (1)
    		{
				_delay_ms(100);
				nc_ctrl = wii_nunchuck_get_data();
				output_nunchuck_screen(&nc_ctrl);
    		}
			break;
		case WII_CONTROLLER_CLASSIC:
			attrset(A_REVERSE);
			mvaddstr(0,  0, "    Wii - Classic - Controller   ");
			attrset(A_BOLD);
    		mvaddstr(2, 1,  "Tasten");
    		mvaddstr(8, 1,  "Joysticks");
    		mvaddstr(12, 1, "Trigger");
   			attrset(A_NORMAL);
			cl_ctrl = wii_classic_get_data();
			output_classic_screen(&cl_ctrl);
    		while (1)
    		{
				_delay_ms(100);
				cl_ctrl = wii_classic_get_data();
				output_classic_screen(&cl_ctrl);
    		}
			break;
		default:
			attrset(A_REVERSE);
			mvaddstr(0,  0, " unbekannter Wii-Controller ");
   			attrset(A_NORMAL);
			sprintf(sbuf, "Controller-ID: 0x%04x", controller_typ); 
			mvaddstr(2, 1, sbuf);		
		   	while(1)
   			{
				wii_controller_get_raw_data(0x00, buf);
				sprintf(sbuf, "0.: %s", int2bin(buf[0], NULL));
				mvaddstr(4, 1, sbuf);		
				sprintf(sbuf, "1.: %s", int2bin(buf[1], NULL));
				mvaddstr(5, 1, sbuf);		
				sprintf(sbuf, "2.: %s", int2bin(buf[2], NULL));
				mvaddstr(6, 1, sbuf);		
				sprintf(sbuf, "3.: %s", int2bin(buf[3], NULL));
				mvaddstr(7, 1, sbuf);		
				sprintf(sbuf, "4.: %s", int2bin(buf[4], NULL));
				mvaddstr(8, 1, sbuf);		
				sprintf(sbuf, "5.: %s", int2bin(buf[5], NULL));
				mvaddstr(9, 1, sbuf);		
				_delay_ms(10);	
			}
			break;	
	}
    endwin ();
    return 0;
}
