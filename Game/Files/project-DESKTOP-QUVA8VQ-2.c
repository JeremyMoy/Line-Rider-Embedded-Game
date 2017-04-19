#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "GLCD.h"
#include "uart.h"

#define DEBUG TRUE

//Function Declarations
void Setup(void);
void TimerInit(void);
void GameInit(void);
void AudioInit(void);
void Delay(int);
void getKey(void);
void clearBuffer(void);
void Character(void);
void CharacterMove(void);
void Left(void);
void Right(void);
void Lines(void);
void LineMove(void);
void Sound(int, int);
void Collision(void);
void GameOver(void);
void GameWin(void);

uint8_t * sendData(uint8_t *, int, char);
	
//Variable Declarations and Initializations
extern volatile uint32_t UART0_Count;
extern volatile uint8_t UART0_Buffer[BUFSIZE];
extern volatile uint32_t UART2_Count;
extern volatile uint8_t UART2_Buffer[BUFSIZE];

char GameScore[8];
char timer = FALSE;
char start = FALSE;
char game = TRUE;
char alive = TRUE;
char move = FALSE;

int direction = 0;
int randCount = 0;
int charX = 0;
int charY = 230;
int lineSize = 24;
int lineX, lineY;
int lineCount;
int linePlacement;
int j;
int score = 2;
int newLineCounter = 0;
int hitCount = 0;

//Line Structure = A way to define a new line
struct line
{
	int x;
	int y;
	char alive;
};

struct line line[100];

int main(void)
{
	//Set up system
	Setup();
	//Randomizer to make random truly random
	while(start == FALSE)
	{
		sprintf(GameScore, "Score = %d", score);
		LCD_PutText(90, 50, "Line Rider: The Game", Yellow, Black);
		LCD_PutText(110, 80, "By: Jeremy Moy", Yellow, Black);
		LCD_PutText(90, 150, "Press Enter to Start", Green, Black);
		getKey();
		clearBuffer();
		randCount++;
		if(randCount == 10000) randCount = 0;
	}
	srand(randCount);
	//Game initialization
	GameInit();
	LCD_DrawHeader(0, 0, Blue);
	while(game == TRUE)
	{
		//Print score at top of screen
		sprintf(GameScore, "Score = %d", score);
		LCD_PutText(0, 0,(uint8_t*)GameScore, Yellow, Black);
		//Check to see which command is sent, or which key is pressed
		getKey();
		clearBuffer();
		//checks to see if a movement is made on the character
		if(move == TRUE)
		{
			//Character moves
			CharacterMove();
			//Reset movement
			direction = 0; 
		}
		if(timer == TRUE)
		{
						//Line moves
			LineMove();			
			if(lineCount == 100)
				{
					lineCount = 0;
				}
			if(newLineCounter == 2)
			{
				//Change index of line structure
				lineCount++;
				//Create new line with new index
				Lines();
				newLineCounter = 0;
			}	
			newLineCounter++;
		}
		//If game end is reached
		if(score == 0 || score > 10)
		{
			game = FALSE;
		}
	}
	//Game over
	if(score <= 0)
	{
		GameOver();
	}
	//Win game
	if(score >= 10)
	{
		GameWin();
	}
}

//Set up function
void Setup(void)
{
	SystemInit();
	UARTInit(0, 9600);
	UARTInit(2, 9600);
	TimerInit();
	LCD_Initialization();
	LCD_Clear(Black);
	sendData((uint8_t *)"AT+VERSION\r\n", 2000, DEBUG);

}

//Game initialization function
void GameInit(void)
{
	LCD_Clear(Black);
	Character();
	Lines();
	lineCount = 0;
}
void getKey(void)
{
		LPC_UART2->IER = IER_THRE | IER_RLS; // Disable RBR 
		UARTSend( 0, (uint8_t *)(UART0_Buffer), UART0_Count );
		if(UART0_Buffer[0] == '\r' || UART2_Buffer[0] == '0')start = TRUE;
		if(UART0_Buffer[2] == 'D' || UART2_Buffer[0] == '1') 
		{
			direction = 1;
			move = TRUE;
		}
		if(UART0_Buffer[2] == 'C' || UART2_Buffer[0] == '2')
		{			
			direction = 2;
			move = TRUE;
		}
		UART2_Count = 0;
		LPC_UART2->IER = IER_THRE | IER_RLS | IER_RBR; // Re-enable RBR
}

//clears the UART's buffer
void clearBuffer(void)
{
	LPC_UART0->IER = IER_THRE | IER_RLS; // Disable RBR 
	UART0_Count = 0;
	LPC_UART0->IER = IER_THRE | IER_RLS | IER_RBR; // Re-enable RBR
	
	LPC_UART2->IER = IER_THRE | IER_RLS; // Disable RBR 
	UART2_Count = 0;
	LPC_UART2->IER = IER_THRE | IER_RLS | IER_RBR; // Re-enable RBR 
}


//Character function
void Character(void)
{
	LCD_DrawObstacle(charX, charY, Green);
}

//Character move function
void CharacterMove(void)
{
	//If value changes because of left key
	if(direction == 1)
	{
		Left();
	}
	//If value changes because of right key
	if(direction == 2)
	{
		Right();
	}
	UART2_Buffer[0]= '3';
	move = FALSE;
}

//Left function: Moves character left
void Left(void)
{
	LCD_DrawObstacle(charX, charY, Black);
	charX -= 80;
	if(charX <= 0) charX = 0;
	LCD_DrawObstacle(charX, charY, Green);
}

//Right function: Moves character right
void Right(void)
{
	LCD_DrawObstacle(charX, charY, Black);
	charX += 80;
	if(charX - 80 >= 319) charX = 320 - 80;		
	LCD_DrawObstacle(charX, charY, Green);
}

//Draws line at random x value
void Lines(void)
{
	linePlacement = rand() % 4;
	line[lineCount].alive = TRUE;
	if(linePlacement == 0)
	{
		line[lineCount].x = 0;
		line[lineCount].y = 20 + lineSize;
	}
	else if(linePlacement == 1)
	{
		line[lineCount].x = 80;
		line[lineCount].y = 20 + lineSize;
	}
	else if(linePlacement == 2)
	{
		line[lineCount].x = 160;
		line[lineCount].y = 20 + lineSize;
	}
	else
	{
		line[lineCount].x = 240;
		line[lineCount].y = 20 + lineSize;
	}
	LCD_DrawObstacle(line[lineCount].x, line[lineCount].y, Red);
}

//Moves line toward user
void LineMove(void)
{
	for(j = 0; j <= lineCount; j++)
	{
		if(line[j].alive == TRUE)
		{
			LCD_DrawObstacle(line[j].x, line[j].y, Black);
			line[j].y += 10;
			if(line[j].y + 24 >= 239)
			{
				score--;
				hitCount--;
				if(hitCount < 0) hitCount = 0;
				line[j].x = -100;
				line[j].y = -100;
				LCD_DrawObstacle(line[j].x, line[j].y, Black);
			}
			else
			LCD_DrawObstacle(line[j].x, line[j].y, Red);
		}
	}
	Collision();
}

//Checks for a collision
void Collision(void)
{
	for(j = 0; j <= 100; j++)
	{
		//Buggy collision math
		if(((abs((line[j].x) - charX)*2) < 160) && ((abs((line[j].y) - charY)*2) < 48))
		{
			LCD_DrawObstacle(line[j].x, line[j].y, Black);
			line[j].x = 700;
			line[j].y = 700;
			LCD_DrawObstacle(line[j].x, line[j].y, Black);
			hitCount++;
			if(hitCount == 2)
			{
				score++;
				hitCount = 0;
			}
			line[j].alive = FALSE;
		}
	}
}

//Game over function
void GameOver(void)
{
	LCD_Clear(Black);
	LCD_PutText(150, 120, (uint8_t *)"You Lose", Green, Black);
}

//Win game function
void GameWin(void)
{
	LCD_Clear(Black);
	LCD_PutText(150, 120, (uint8_t *)"You Win", Green, Black);
	LCD_draw_smiley_face(30, 30, 80, Red);
	LCD_draw_smiley_face(290, 30, 80, Green);
}

//Timer Initialization
void TimerInit(void)
{
	LPC_SC->PCONP |= 1 << 1; // Power up Timer 0 (see page 64 of user manual) 
  LPC_SC->PCLKSEL0 |= 1 << 2; // Clock for timer = CCLK, i.e., CPU Clock
  LPC_TIM0->MR0 = 1 << 27; // Give a value suitable for the LED blinking 
                           // frequency based on the clock frequency 
  LPC_TIM0->MCR |= 1 << 0; // Interrupt on Match 0 compare 
  LPC_TIM0->MCR |= 1 << 1; // Reset timer on Match 0 
  LPC_TIM0->TCR |= 1 << 1; // Manually Reset Timer 0 (forced); 
  LPC_TIM0->TCR &= ~(1 << 1); // Stop resetting the timer 
  NVIC_EnableIRQ(TIMER0_IRQn); // see core_cm3.h header file
  LPC_TIM0->TCR |= 1 << 0; // Start timer (see page 501 and 505 of user manual)	
}

//Timer Handler
void TIMER0_IRQHandler(void)
{	
	timer = TRUE;
	LPC_TIM0->IR |= 1 << 0; // Clear MR0 interrupt flag
	 
}

uint8_t * sendData(uint8_t * command, int timeout, char debug){
	uint8_t * constructedResponse;
	int time;
	int dataSize = strlen((char *)command);
	
	UARTSend(2, command, dataSize);
	if(debug){
		UARTSend(0, command, strlen((char *)command));
	}
	time = LPC_TIM0->TC;
	while((time+timeout) > LPC_TIM0->TC){
		while (UART2_Count != 0)
		{
			LPC_UART2->IER = IER_THRE | IER_RLS; // Disable RBR 
			UARTSend(0, (uint8_t *)(UART2_Buffer), UART2_Count);
			UART2_Count = 0;
			LPC_UART2->IER = IER_THRE | IER_RLS | IER_RBR; // Re-enable RBR 
		}
	}
	Delay(3000000);
	return (uint8_t *)constructedResponse;
}


//Initialization of the speakers
void AudioInit(void)
{
	LPC_PINCON->PINSEL1 &= ~(3<<20);   /* P0.26 21:20*/
  LPC_PINCON->PINSEL1 |=  (2<<20);   /* PINSEL1 21:20 = 10 */
}

//Plays the speaker sound
void Sound(int d, int t)
{
	int v;
	timer = FALSE;
	while(t > 0){
		v++;
		LPC_DAC->DACR = v << 6;//sets the tone of the speaker
		Delay(d);	//this delay changes the frequency depending on its length
		LPC_DAC->DACR = 0x8000;	//mutes the speaker
		if(timer){
			t--;
			timer = FALSE;
		}
	}	
}

//delay for the speaker
void Delay(int d)
{
	while(d > 0){
		d--;
	}
}
