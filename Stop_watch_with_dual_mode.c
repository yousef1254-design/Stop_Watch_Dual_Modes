/*
 * Stop_watch_with_dual_mode.c
 *
 *  Created on: Sep 14, 2026
 *      Author: YOUSEF
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char min=0,sec=0,hours=0,mode=0,pause=0,resume=1 ;	//mode=0 count up , mode=1 count down


void INT1_Init(void)
{
	MCUCR|=(1<<ISC11)|(1<<ISC10);			//interrupt with raising edge
	GICR|=(1<<INT1);						//external interrupt 1 enable
}

ISR(INT1_vect)				//pause interrupt
{
	pause=1;
	resume=0;
}

void INT2_Init(void)
{
	GICR|=(1<<INT2);				//external interrupt 2 enable
	MCUCSR&=~(1<<ISC2);				//interrupt with falling edge
}

ISR(INT2_vect)			//Resume interrupt
{
	resume=1;
	pause=0;
}

void INT0_Init(void)
{
	MCUCR|=(1<<ISC01);				//interrupt with falling edge
	GICR|=(1<<INT0);				//external interrupt 0 enable
}

ISR(INT0_vect)				//Reset interrupt
{
	sec=0;	min=0;	hours=0;
}

void TIMER1_Init_CTC_Mode(void)
{
	TCCR1B=(1<<WGM12)|(1<<CS12)|(1<<CS10);
	TCCR1A=(1<<FOC1A);
	OCR1A=15624;
	TCNT1=0;
	TIMSK=(1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect){
	if(pause==0 && resume==1)
	{
		if(mode==0)
		{
			if(sec<60)
				sec++;
			else
			{
				sec=0;
				if( min<60)
					min++;
				else if(min==60)
				{
					min=0;
					if(hours<24)
						hours++;
					else
						hours=0;
				}
			}
		}
		else if(mode==1)
		{
			if(sec>0)
				sec--;
			else
			{
				if(min!=0 || hours!=0)
					sec=59;
				if(min>0)
					min--;
				else
				{
					if(hours!=0)
						min=59;
					if(hours>0)
						hours--;
				}
			}
		}
	}
}

void Display_Seven_Segment(void)
{
    // Display tens of hours
    PORTA = (PORTA & 0xC0) | (1 << PA0);
    PORTC = (PORTC & 0xF0) | (hours / 10);
    _delay_ms(2);

    // Display ones of hours
    PORTA = (PORTA & 0xC0) | (1 << PA1);
    PORTC = (PORTC & 0xF0) | (hours % 10);
    _delay_ms(2);

    // Display tens of minutes
    PORTA = (PORTA & 0xC0) | (1 << PA2);
    PORTC = (PORTC & 0xF0) | (min / 10);
    _delay_ms(2);

    // Display ones of minutes
    PORTA = (PORTA & 0xC0) | (1 << PA3);
    PORTC = (PORTC & 0xF0) | (min % 10);
    _delay_ms(2);

    // Display tens of seconds
    PORTA = (PORTA & 0xC0) | (1 << PA4);
    PORTC = (PORTC & 0xF0) | (sec / 10);
    _delay_ms(2);

    // Display ones of seconds
    PORTA = (PORTA & 0xC0) | (1 << PA5);
    PORTC = (PORTC & 0xF0) | (sec % 10);
    _delay_ms(2);


}

int main(){
	DDRA|=0x3F;         //make PA0 -> PA5  output
	PORTA&=0xc0;		//make PA0 -> PA5  at beginning is 0
	DDRD|=0x31;			//make PD4,PD5,PD0 output
	DDRD&=0xf3;			//make PD2,PD3 input
	PORTD&=0xCE;		//make PD5,PD4,PD0  at beginning is 0
	PORTD|=0x04;		//make PD2 internal pull up resistor
	DDRB&=0x00;			//make all PORTB input
	PORTB|=0xFF;		//make PB0 -> PB7 internal pull up resistor
	DDRC|=0x0F;			//make PC0 -> PC3 output
	PORTC&=0xf0;		//make PC0 -> PC3  at beginning is 0

	SREG|=(1<<7);		//enable global interrupt

	TIMER1_Init_CTC_Mode();
	INT1_Init();
	INT0_Init();
	INT2_Init();

	while(1){


		if((!(PINB & (1<<PB7))) && pause==1)		//Toggle Mode
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB7)))
			{
				mode^=1;
			}
			while(!(PINB & (1<<PB7)));
		}

		if(mode==0)					//Count Up
		{
			Display_Seven_Segment();
			PORTD|=(1<<PD4);
			PORTD&=~(1<<PD5);
			PORTD&=~(1<<PD0);
		}

		if(mode==1 )					//Count Down
		{
			PORTD|=(1<<PD5);
			PORTD&=~(1<<PD4);
			if(sec==0 && min==0 && hours==0)
			{
				PORTD|=(1<<PD0);
			}
			 Display_Seven_Segment();
		}

		if((!(PINB & (1<<PB1))) && pause==1)		//hours increment
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB1)))
			{
				if(hours<24)
					hours++;
			}
			while(!(PINB & (1<<PB1)));
		}

		if((!(PINB & (1<<PB0))) && pause==1)		//hours decrement
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB0)))
			{
				if(hours>0)
					hours--;
			}
			while(!(PINB & (1<<PB0)));
		}

		if((!(PINB & (1<<PB4))) && pause==1)		//minutes increment
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB4)))
			{
				if(min<59)
					min++;
			}
			while(!(PINB & (1<<PB4)));
		}

		if((!(PINB & (1<<PB3))) && pause==1)		//minutes decrement
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB3)))
			{
				if(min>0)
					min--;
			}
			while(!(PINB & (1<<PB3)));
		}

		if((!(PINB & (1<<PB6))) && pause==1)		//second increment
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB6)))
			{
				if(sec<59)
					sec++;
			}
			while(!(PINB & (1<<PB6)));
		}

		if((!(PINB & (1<<PB5))) && pause==1)		//second decrement
		{
			_delay_ms(30);
			if(!(PINB & (1<<PB5)))
			{
				if(sec>0)
					sec--;
			}
			while(!(PINB & (1<<PB5)));
		}

	}
}













