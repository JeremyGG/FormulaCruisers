#include <avr/io.h>
#include <stdio.h>
#include "Defines.h"
#include "lcd.h"
#include "UI.h"
#include "Error.h"

void lcd_refresh()
{
	get_screen(Linebuffer, ui_current_screen);

	if(!shutdownon)
	{
		Linebuffer[2][16] = '!';
		Linebuffer[2][17] = 'S';
		Linebuffer[2][18] = 'D';
		Linebuffer[2][19] = '!';
	}
	if(ams_shutdown)
	{
		Linebuffer[2][11] = '!';
		Linebuffer[2][12] = 'A';
		Linebuffer[2][13] = 'M';
		Linebuffer[2][14] = 'S';
		Linebuffer[2][15] = '!';
	}
	if(imd_shutdown)
	{
		Linebuffer[2][6] = '!';
		Linebuffer[2][7] = 'I';
		Linebuffer[2][8] = 'M';
		Linebuffer[2][9] = 'D';
		Linebuffer[2][10] = '!';
	}

	lcd_quickrefresh();
}

void lcd_quickrefresh()
{
	//Write the current line buffer into their respective LCD lines
	for(uint8_t y = 0; y < 4; y++)
	{
		lcd_gotoxy(0, y);
		lcd_puts(Linebuffer[y]);
	}
}

void change_screen(enum uiscreen s)
{
	ui_current_screen = s;
	lcd_refresh();	
}

void get_screen(char buffer[4][21], enum uiscreen s)
{
	switch(s)
	{
		case SCREEN_WELCOME:
			snprintf(buffer[0], sizeof buffer[0], "        HU-2        ");
			snprintf(buffer[1], sizeof buffer[1], SOFTWARE_VERSION);
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], " Press blue button  ");
			break;

		case SCREEN_ERROR:
			snprintf(buffer[0], sizeof buffer[0], "     !!!ERROR!!!    ");
			snprintf(buffer[1], sizeof buffer[1], get_error(_errorcode));
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], "                    ");
			break;

		case SCREEN_PREDISCHARGING:
			snprintf(buffer[0], sizeof buffer[0], " Precharging...     ");
			snprintf(buffer[1], sizeof buffer[1], " %5d              ", predistimer);
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], "Flow: L%5d  R%5d", flowleft, flowright);
			break;

		case SCREEN_START:
			snprintf(buffer[0], sizeof buffer[0], "Gas1:%4d Gas2:%4d", gas1, gas2);
			snprintf(buffer[1], sizeof buffer[1], "Brake:%4d          ", brake);
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], "Press green to begin");
			break;

		case SCREEN_DRIVING:
			snprintf(buffer[0], sizeof buffer[0], "Gas1:%3d%%, Gas2:%3d%%", gas1perc, gas2perc);
			snprintf(buffer[1], sizeof buffer[1], "Brake:%3d%%          ", brakeperc);
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], "                    ");
			break;

		case SCREEN_STATUS:
			snprintf(buffer[0], sizeof buffer[0], "Flow: L%5d  R%5d", flowleft, flowright);
			snprintf(buffer[1], sizeof buffer[1], "Rpm: FL%5d FR%5d", rpm_fl, rpm_fr);
			snprintf(buffer[2], sizeof buffer[2], "Gas1:%3d%%, Gas2:%3d%%", gas1perc, gas2perc);
			snprintf(buffer[3], sizeof buffer[3], "Steerpos: %5d     ", steerpos);
			break;

		default:
			snprintf(buffer[0], sizeof buffer[0], " Unknown screen ID  ");
			snprintf(buffer[1], sizeof buffer[1], "                    ");
			snprintf(buffer[2], sizeof buffer[2], " Tell Jeremy to fix ");
			snprintf(buffer[3], sizeof buffer[3], " his stuff          ");
			break;
			
		/*
		case SCREEN_:
			snprintf(buffer[0], sizeof buffer[0], "                    ");
			snprintf(buffer[1], sizeof buffer[1], "                    ");
			snprintf(buffer[2], sizeof buffer[2], "                    ");
			snprintf(buffer[3], sizeof buffer[3], "                    ");
			break;
		*/
	}
}
