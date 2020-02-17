#include "Std_Types.h"
#include "Macros.h"
#include "DIO_interface.h"
#include "TIMER0_interface.h"
#include "TIMER1_interface.h"
#include "INTERRUPTS_interface.h"
#include "LCD_interface.h"
#include <util/delay.h>

void vidCountOVF(void);
void vidGetPulse(void);
void vidTrigger(void);
void vidBuildPulse(void);

volatile u32 u32OVFCount = 0;
volatile u8 u8Edge = TIMER1_ICP_EDGE_FALLING;
volatile u64 u64PulseTime = 0;
volatile u16 u16FirstEdgeTime = 0;
volatile u16 u16Timer0OVF = 0;
volatile u8 u8Display = 0;

void vidCountOVF(void)
{
	u32OVFCount++;
}

void vidGetPulse(void)
{

	if (u8Edge == TIMER1_ICP_EDGE_FALLING)
	{

		u64PulseTime = TIMER1_u16GetInputCaptTime() + (65536*u32OVFCount);		
		TIMER1_vidSelectInputCaptEdge(TIMER1_ICP_EDGE_RISING);
		u8Edge = TIMER1_ICP_EDGE_RISING;
			}
	else
	{
		TIMER1_vidSelectInputCaptEdge(TIMER1_ICP_EDGE_FALLING);
		u8Edge = TIMER1_ICP_EDGE_FALLING;
		TIMER1_vidSetTCNT(0);
		u32OVFCount = 0;
	}

}

void vidTrigger(void)
{
	DIO_vidSetPinValue(DIO_PORTD,DIO_PIN1,STD_HIGH);
	_delay_us(10);
	DIO_vidSetPinValue(DIO_PORTD,DIO_PIN1,STD_LOW);
}

void vidBuildPulse(void)
{
	u16Timer0OVF++;
	if (u16Timer0OVF == 3150)
	{
		u16Timer0OVF = 0;
		if (u8Display == 1)
		{
			LCD_vidSendCommand(LCD_CLEAR_SCREEN);
			LCD_vidSendCommand(LCD_RETURN_HOME);
			LCD_vidWriteString("Distance: ");
			LCD_vidGoToXY(LCD_XPOS10,LCD_YPOS0);
			LCD_vidWriteNumber(u64PulseTime/466);
			u8Display = 0;
		}
		else
		{
			vidTrigger();
			u8Display = 1;
		}
	}
}

void main(void)
{

	LCD_vidInit();

	TIMER1_vidInit(TIMER1_WGM_NORMAL,TIMER1_COM1A_NORMAL,TIMER1_COM1B_NORMAL,TIMER1_CLK_1);

	TIMER0_vidInit(TIMER0_WGM_NORMAL,TIMER0_COM_NORMAL,TIMER0_CLK_1);

	/*Test*/
	INTERRUPTS_vidEnableInterrupt(INTERRUPTS_TIMER0_OVF);
	INTERRUPTS_vidPutISRFunction(INTERRUPTS_TIMER0_OVF,vidBuildPulse);

	/*Input capture initialization*/
	TIMER1_vidSelectInputCaptEdge(TIMER1_ICP_EDGE_FALLING);

	INTERRUPTS_vidPutISRFunction(INTERRUPTS_TIMER1_ICP,vidGetPulse);
	INTERRUPTS_vidPutISRFunction(INTERRUPTS_TIMER1_OVF,vidCountOVF);
	
	INTERRUPTS_vidEnableInterrupt(INTERRUPTS_TIMER1_OVF);
	INTERRUPTS_vidEnableInterrupt(INTERRUPTS_TIMER1_ICP);


	/*Trigger pin configuration*/
	DIO_vidSetPinDirection(DIO_PORTD,DIO_PIN1,DIO_OUTPUT);

	/*Echo pin configuration*/
	DIO_vidSetPinDirection(DIO_PORTD,DIO_PIN6,DIO_INPUT);
	DIO_vidSetPinValue(DIO_PORTD,DIO_PIN6,STD_HIGH);

	INTERRUPTS_vidSetGlobalInterruptFlag();

	while(1)
	{

			}
}
