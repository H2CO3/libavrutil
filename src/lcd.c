/* 
 * lcd.c
 * libavrutil
 *
 * Created by Árpád Goretity on 30/03/2012.
 * Licensed under a CreativeCommons Attribution-ShareAlike 3.0 Unported License
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include "lcd.h"

#define __AVR_LCD_DDR  DDRC
#define __AVR_LCD_PORT PORTC

#define __AVR_LCD_DB4  PC0
#define __AVR_LCD_DB5  PC1
#define __AVR_LCD_DB6  PC2
#define __AVR_LCD_DB7  PC3

#define __AVR_LCD_RS   PC4
#define __AVR_LCD_CLK  PC5

#define LOW_NIBBLE(b)  (((uint8_t)(b) >> 0) & B00001111)
#define HIGH_NIBBLE(b) (((uint8_t)(b) >> 4) & B00001111)

static uint8_t __avr_lcd_cursor_pos = 0;

uint8_t __avr_lcd_transcribe_byte(uint8_t byte);
void __avr_lcd_clock();
void __avr_lcd_send_command(uint8_t byte);

uint8_t __avr_lcd_transcribe_byte(uint8_t byte)
{
	uint8_t b = 0;
	b |= ((byte >> 0) & B00000001) << __AVR_LCD_DB4;
	b |= ((byte >> 1) & B00000001) << __AVR_LCD_DB5;
	b |= ((byte >> 2) & B00000001) << __AVR_LCD_DB6;
	b |= ((byte >> 3) & B00000001) << __AVR_LCD_DB7;

	return b;
}

void __avr_lcd_clock()
{
	__AVR_LCD_PORT |= _BV(__AVR_LCD_CLK);
	_delay_us(10); /* wait more than 500 ns */
	__AVR_LCD_PORT &= ~_BV(__AVR_LCD_CLK);
}

void __avr_lcd_send_command(uint8_t byte)
{
	uint8_t high_nibble = HIGH_NIBBLE(byte);
	uint8_t low_nibble  = LOW_NIBBLE(byte);
	
	/* Send high nibble first */
	__AVR_LCD_PORT = 0;
	__AVR_LCD_PORT |= high_nibble;
	__avr_lcd_clock();
	_delay_ms(10);
	
	/* Send low nibble second */
	__AVR_LCD_PORT = 0;
	__AVR_LCD_PORT |= low_nibble;
	__avr_lcd_clock();
	_delay_ms(10);
}

void avr_lcd_set_cursor_pos(uint8_t addr)
{
	__avr_lcd_cursor_pos = addr;
	__avr_lcd_send_command(_BV(7) | __avr_lcd_cursor_pos);
}

uint8_t avr_lcd_get_cursor_pos()
{
	return __avr_lcd_cursor_pos;
}

void avr_lcd_init()
{
	/* Wait for LCD to boot up */
	__AVR_LCD_DDR = 0xff;
	__AVR_LCD_PORT = 0;
	_delay_ms(50);
	
	/* Set 8-bit mode 3 times (!) */
	__AVR_LCD_PORT |= _BV(__AVR_LCD_DB5) | _BV(__AVR_LCD_DB4);
	__avr_lcd_clock();
	_delay_ms(5);
	__avr_lcd_clock();
	_delay_ms(1);
	__avr_lcd_clock();
	_delay_ms(1);

	/* Enable 4-bit mode */
	__avr_lcd_send_command(_BV(1));	
	
	/* Initially setup the LCD */
	__avr_lcd_send_command(_BV(5) | _BV(3)); /* 4-bit mode; 2-line display; 5x8 px font */
	__avr_lcd_send_command(_BV(3)); /* Display, cursor, blink: OFF */
	__avr_lcd_send_command(_BV(0)); /* Clear screen */
	__avr_lcd_send_command(_BV(2) | _BV(1)); /* Shift cursor to the right after writing a char */
	__avr_lcd_send_command(_BV(3) | _BV(2)); /* Display: ON, cursor, blink: OFF */
}


void avr_lcd_config(uint8_t flags)
{
	__avr_lcd_send_command(_BV(3) | flags);
}

void avr_lcd_clear()
{
	__avr_lcd_send_command(_BV(0));
}

void avr_lcd_home()
{
	__avr_lcd_send_command(_BV(1));
}

void avr_lcd_putc(char c)
{
	__AVR_LCD_PORT = 0;
	__AVR_LCD_PORT |= __avr_lcd_transcribe_byte(HIGH_NIBBLE(c));
	__AVR_LCD_PORT |= _BV(__AVR_LCD_RS);
	__avr_lcd_clock();
	_delay_ms(5);

	__AVR_LCD_PORT = 0;
	__AVR_LCD_PORT |= __avr_lcd_transcribe_byte(LOW_NIBBLE(c));
	__AVR_LCD_PORT |= _BV(__AVR_LCD_RS);
	__avr_lcd_clock();
	_delay_ms(5);
}

void avr_lcd_puts(const char *s)
{
	while (*s)
	{
		avr_lcd_putc(*s++);
	}
}

