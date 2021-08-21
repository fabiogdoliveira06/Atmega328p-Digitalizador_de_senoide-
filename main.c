#define FOSC 1000000
#define BAUD 9600
#define MYUBR FOSC/16/BAUD-1
#define TIMER_TOP (125-1)
#include <xc.h>
#include <avr/interrupt.h>

void enviaLeituras();
ISR(USART_RX_vect);

int myubr;
unsigned int valor_adc = 0;
unsigned int leituras[50];
int posicao = 0;
int FlagTX = 0;
int FlagVetor = 0;

int main(void)
{
	DDRB |= 0x01;
	// Valor a ser comparado no contador do Timer 0
	OCR0A = TIMER_TOP;
	
	//Inicializa com o timer desligado
	TCCR0B = 0x00; 
	
	// Modo CTC (Clear Time on Compare Match) para zerar o contador do timer ao atingir o valor de comparação          
	//TCCR0A &= (0 << WGM00) ; 
	TCCR0A |= (1 << WGM01) ;  
	 
 	//Garante que o comparador do timer 0 está ativado
 	TIMSK0 |= (1 << OCIE0A);
	
 	// Liga o timer com prescaler de 64
 	TCCR0B |= 0x03;	

	//ADC Prescaler de 128
	ADCSRA = 0b00000111;
	
	//Seleciona o ADC1
	ADMUX = 0;
	
	//Ativa o ADC
	ADCSRA |= 1 << ADEN;
	
	// set baudrate in UBRR
	myubr = 2*MYUBR;
	UBRR0H = (unsigned char)((myubr)>>8);
	UBRR0L = (unsigned char)((myubr));

	//Double the USART Transmission Speed
	UCSR0A = 0b00000010;
	UCSR0B = 0B10011000;
	UCSR0C = 0B00000110;
	
	//Ativa as interrupções
	sei();
	
	while(1)
    {

		if ( (FlagTX == 1) && (FlagVetor == 1) )
		{
			enviaLeituras();
			FlagVetor = 0;
		}			
    }
}

//Interrupção do comparador do Timer 0 irá disparar uma interrupção a cada 1 ms
ISR(TIMER0_COMPA_vect)
{
		
	//começa a Conversão
	ADCSRA |= 1 << ADSC;
	
	//aguarda o fim da conversão
	while( !(ADCSRA & (1<<ADIF)) );
	
	//limpa o ADIF	
	ADCSRA |= (1<<ADIF);   
	
    //Faz a Conversão 
	valor_adc = ADCL + (ADCH << 8);
	
	//armazena o valor convertido no vetor de valores
	leituras[posicao] = valor_adc;
	//incremnta a posição
	posicao++;
	
	PORTB ^= 0x01;

	if (posicao == 50)
	{
		FlagVetor = 1;
		posicao = 0;
	}

}

ISR(USART_RX_vect)
{
	// variaval que armazena o valor da USART
	unsigned char data;
	
	//realiza leitura da USART
	data = UDR0;
	
	if(data == '0')
	{
		FlagTX = 1;
	}
	else
	{
		FlagTX = 0;
	}
}


void enviaLeituras()
{
	int aux;
	int valor;
	
	for (aux = 0; aux < 50; aux++)
	{
		valor = leituras[aux];
		
		while( ! (UCSR0A & (1<<UDRE0)) );
		
		//envia os bits mais significativos
		UDR0 = (unsigned char)((valor));
		
		while( ! (UCSR0A & (1<<UDRE0)) );
		//envia os 8 bits menos significativos
		
		UDR0 = (unsigned char)((valor)>>8);
		//aguarda a transmissão acabar

	}
}