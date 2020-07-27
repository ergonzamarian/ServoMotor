//https://exploreembedded.com/wiki/8051_Family_C_Library
//https://exploreembedded.com/wiki/8051_serial_communication
//http://www.electronicwings.com/8051/servo-motor-interfacing-with-8051


#include<reg52x2.h>
#include<stdio.h>
#include <intrins.h>
#define adc_databus P2 // porta para leitura dos dados convertidos
#define PWM_Period 0xB7FE // Define value to be loaded in timer for PWM period of 20 milli second 


//PINOS DE CONTROLE DO ADC 0808
sbit adc_A    = P1^0;
sbit adc_B    = P1^1;

//SBIT ADC_C    = P3^2; SOMENTE 4 CANAIS ECONOMIZA 1 BIT
sbit adc_ALE  = P1^2; // escolha do canal
sbit adc_Start= P1^3; // inicio da conversão
sbit adc_EOC  = P1^4; // indica final da conversão 0 -->1
sbit adc_OE   = P1^5; // habilitar saidas do conversor

//PINOS DE PORTA
sbit Servo_1 = P3^7;
sbit Servo_2 = P3^6;
sbit Servo_3 = P3^5;
sbit Servo_4 = P3^4;

//VARIÁVEIS PARA CONTROLE DO PWM
unsigned int ON_Period, OFF_Period, DutyCycle;
unsigned char	ch,he;

float PWM_IN0,PWM_IN1,PWM_IN2,PWM_IN3, x;
int motor = 0;
void Delay(unsigned int);

//FUNÇÕES DE DELAY
//PARA 1us
void delay_us(unsigned int us_count)
{
    while(us_count!=0)
    {
        us_count--;
    }
}
//FUNÇÕES QUE FARÃO A CONVERSÃO DO ADC

void ADC_Init()
{
    adc_Start=0;                //Initialize all the control lines to zero.
    adc_ALE=0;
    adc_OE=0;
    adc_EOC=1;                  //Configure the EOC pin as INPUT
    adc_databus=0xff;             //configure adc_databus as input

}

unsigned char ADC_StartConversion(char channel)//É ENVIADO COMO CHAR PARA QUE POSSA REALIZAR A CONVERSÃO
{
    unsigned char adc_result;
    //SELECIONA O CANAL
    adc_A=((channel>>0) & 0x01);     //desloca para direita 0 bits
    adc_B=((channel>>1) & 0x01);     //desloca para direita 1 bits


    adc_ALE=1;         // Latch the address by making the ALE high / Define o canal a ser convertido.
    delay_us(500);
    adc_Start=1;       //Start the conversion after latching the channel address / Inicie a conversão depois de bloquear o endereço do canal
    delay_us(250);

    adc_ALE=0;         //Pull ALE line to zero after starting the conversion.
    delay_us(500);
    adc_Start=0;       //Pull Start line to zero after starting the conversion.

    while(adc_EOC==0);   
	//ESPERANDO ROTINA PARA CONVERSÃO
	// Wait till the ADC conversion is completed,
    // EOC will be pulled to HIGH by the hardware(ADC0809)
    // once conversion is completed.

    adc_OE=1;         //Make the Output Enable high
    //to bring the ADC data to port pins
    delay_us(250);
    adc_result=adc_databus;  //Read the ADC data from ADC bus
    adc_OE=0;           //After reading the data, disable th ADC output line.

	

    return(adc_result) ;

}
void UART_TxChar(char ch)
{
    SBUF = ch;      // Load the data to be transmitted
    while(TI==0);   // Wait till the data is trasmitted
    TI = 0;         //Clear the Tx flag for next cycle.
}

//Funções para geração do PWM
void Timer_init()
{
    TMOD = 0x21;		/* Timer0 mode1 */
    TH0 = (PWM_Period >> 8);/* 20ms timer value */
    TL0 = PWM_Period;
    TR0 = 1;		/* Start timer0 */
}

/* Timer0 interrupt service routine (ISR) */
void Timer0_ISR() interrupt 1
{
	switch (motor)
	{
		case 1:
			Servo_1 = !Servo_1;
			if(Servo_1)
			{
				TH0 = (ON_Period >> 8);
				TL0 = ON_Period;
			}
			else
			{
				TH0 = (OFF_Period >> 8);
				TL0 = OFF_Period;
			}
		break;
		case 2:
			Servo_2 =!Servo_2;
			if(Servo_2)
			{
				TH0 = (ON_Period >> 8);
				TL0 = ON_Period;
			}
			else
			{
				TH0 = (OFF_Period >> 8);
				TL0 = OFF_Period;
			}
		break;
		case 3:
			Servo_3 =!Servo_3;
			if(Servo_3)
			{
				TH0 = (ON_Period >> 8);
				TL0 = ON_Period;
			}
			else
			{
				TH0 = (OFF_Period >> 8);
				TL0 = OFF_Period;
			}
		break;
		case 4:
			Servo_4 =!Servo_4;
			if(Servo_4)
			{
				TH0 = (ON_Period >> 8);
				TL0 = ON_Period;
			}
			else
			{
				TH0 = (OFF_Period >> 8);
				TL0 = OFF_Period;
			}
		break;
	}
   
}

/* Calculate ON & OFF period from duty cycle */
void Set_DutyCycle_To(float duty_cycle)
{
    float period = 65535 - PWM_Period;
    ON_Period = ((period/100.0) * duty_cycle);
    OFF_Period = (period - ON_Period);
    ON_Period = 65535 - ON_Period;
    OFF_Period = 65535 - OFF_Period;
}

void Timer2_ISR() interrupt 5
{
    P1_6=~P1_6;
}
void Delay(unsigned int ms)
{
	//RECEBERÁ 5ms E MULTIPLICARÁ POR 100 FICANDO EM 0,5s
    unsigned long int us = ms*100; 
    while(us--)//ROTINA DELAY REFAZ DELAY, ESPERA 500 PELO VALOR LOGO ABAIXO 
    {
        _nop_();
    }
}

//******* PRINCIPAL *********//
void main()
{
    unsigned char IN0,IN1,IN2,IN3;
    char ch;


	//config adc
	adc_Start=0; //Initialize all the control lines to zero.
	adc_ALE=0;
	adc_OE=0;
	adc_EOC=1; //Configure the EOC pin as INPUT
	adc_databus=0xff; //configure adc_databus as input

	//config timer
	TMOD = 0x21; //Timer0(PWM) mode1 e Timer1(serial) in Mode2

	//config pwm
	TH0 = (PWM_Period >> 8); //20ms timer value
	TL0 = PWM_Period;
	TR0 = 1; //Start timer0

	//config serial
	SCON = 0x50; //Asynchronous mode, 8-bit data and 1-stop bit
	TH1 = 0xFD; //9600 baudrate
	TR1 = 1; //Turn ON the timer for Baud rate generation

	//configurando interrupcoes
	ES = 1; //interrupcao da serial
	EA  = 1; //Atendendo interrupcao
	EX0 = 1; //Habilitando externa 0
	EX1 = 1; //Habilitano externa 1
	ET0 = 1; //Enable timer0 interrupt
	IT1 = 1; // sensivel por borda
	IT0 = 1; // sensivel por borda

	//utilizando o timer 2
	T2MOD=0X00;
	T2CON=0X00;
	RCAP2L=0X00;
	RCAP2H=0X4D;
	TL2=0X00;
	TH2=0X9B;
	ET2=1;
	TR2 = 1;
	Delay(51);
    while(1)
    {	
		//REALIZARÁ A CHAMADA DA FUNÇÃO DO PWM PARA DEFINIR NÍVEIS ALTOS E BAIXOS
        IN0 = ADC_StartConversion(0);
        motor = 1;
        PWM_IN0 = ((IN0*9.3)/255)+2.7;
        Set_DutyCycle_To(PWM_IN0);
        Delay(5);

        IN1 = ADC_StartConversion(1);
        motor = 2;
        PWM_IN1 = ((IN1*9.3)/255)+2.7;
        Set_DutyCycle_To(PWM_IN1);
        Delay(5);

        IN2 = ADC_StartConversion(2);
        motor = 3;
        PWM_IN2 = ((IN2*9.3)/255)+2.7;
        Set_DutyCycle_To(PWM_IN2);
        Delay(5);

        IN3 = ADC_StartConversion(3);
        motor = 4;
        PWM_IN3 = ((IN3*9.3)/255)+2.7;
        Set_DutyCycle_To(PWM_IN3);
        Delay(5);
		//CONFERE SE ENTROU DADO NA SERIAL
        if(RI==1)
        {
            ch=SBUF;
            RI=0;

            if(ch == 'q')
            {
				//ENQUANTO NÃO RECEBER O CARCATERE X SERÁ FEITO A RECEPÇÃO DOS DADOS PELA SERIAL
                while(ch!='x')
                {	
					//CONFERE SE ENTROU DADO NA SERIAL
                    if(RI==1)
                    {
                        ch=SBUF;
                        RI=0;
                        if(ch =='a')//ABAIXAR
                        {
                            motor = 1;
                            if(PWM_IN0>=2.7&&PWM_IN0<=12)
							{
                                PWM_IN0=PWM_IN0+0.5;
								
							}
							Set_DutyCycle_To(PWM_IN0);
                        }
                        if(ch=='l')//LEVANTAR
                        {
                            motor = 1;
                            if(PWM_IN0>=2.7&&PWM_IN0<=12)
							{
                                PWM_IN0=PWM_IN0-0.5;
								
							}
							Set_DutyCycle_To(PWM_IN0);
                        }
                    }
                    if(RI==1)
					{
                        ch=SBUF;
                        RI=0;
                        if(ch=='d')//DIREITA
						{
                            motor = 2;
                            if(PWM_IN1>=2.7&&PWM_IN1<=12)
							{
                                PWM_IN1=PWM_IN1+0.5;
								
							}
							Set_DutyCycle_To(PWM_IN1);
                        }
                        if(ch=='e')//ESQUERDA
						{
                            motor = 2;
                            if(PWM_IN1>=2.7&&PWM_IN1<=12)
							{
                                PWM_IN1=PWM_IN1-0.5;
								
							}
							Set_DutyCycle_To(PWM_IN1);
                        }
                    }
                    if(RI==1)
					{
                        ch=SBUF;
                        RI=0;
                        if(ch=='f')//FRENTE
						{
                            motor = 3;
                            if(PWM_IN2>=2.7&&PWM_IN2<=12)
							{
                                PWM_IN2=PWM_IN2+0.5;
								
							}
							Set_DutyCycle_To(PWM_IN2);
                        }
                        if(ch=='t')//TRAS
						{
                            motor = 3;
                            if(PWM_IN2>=2.7&&PWM_IN2<=12)
							{
                                PWM_IN2=PWM_IN2-0.5;
								
							}
							Set_DutyCycle_To(PWM_IN2);
                        }
                    }
                    if(RI==1) 
					{
                        ch=SBUF;
                        RI=0;
                        if(ch=='p')//PEGAR
						{
                            motor = 4;
                            if(PWM_IN3>=2.7&&PWM_IN3<=12)
							{
                                PWM_IN3=PWM_IN3+0.5;
								
							}
							Set_DutyCycle_To(PWM_IN3);
                        }
                        if(ch=='s')//SOLTAR
						{
                            motor = 4;
                            if(PWM_IN3>=2.7&&PWM_IN3<=12)
							{
                                PWM_IN3=PWM_IN3-0.5;
								
							}
							Set_DutyCycle_To(PWM_IN3);
                        }
                    }
                }
            }
        }
    }
}