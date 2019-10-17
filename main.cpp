
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>

// digital_inputs
#define DI_PORT				PORTB
#define DI_PORT_DDR			DDRB
#define DI_PIN				PINB
#define IGN_PIN				PINB0
#define IGN_INPUT			PB0
#define RPI_PIN				PINB2
#define RPI_INPUT			PB2

// digital_outputs
#define DO_PORT				PORTB
#define DO_PORT_DDR			DDRB
#define DO_PIN				PINB
#define PWR_ON_PIN			PINB1
#define PWR_ON_OUTPUT		PB1

// misc
uint8_t ignition_state = 0;
uint8_t sleep_flag = 0;


ISR(PCINT0_vect) {  // IGN_INPUT vector
	
	if (bit_is_clear(DI_PIN, IGN_INPUT)) {  // check if IGN_INPUT is ON (this input is reverse logic)
		ignition_state = 1;
		DO_PORT |= (1 << PWR_ON_OUTPUT);  // turn on PWR_ON_OUTPUT (enables power supply to RPi)
	}
	else {
		ignition_state = 0;
	}
	
}

ISR(PCINT2_vect) {  // RPI_INPUT vector
	
	if (bit_is_clear(DI_PIN, RPI_INPUT)) {  // check in RPI_INPUT is OFF		
		if (ignition_state == 0) {
			DO_PORT &= ~(1 << PWR_ON_OUTPUT);  // turn off PWR_ON_OUTPUT (disables power supply to RPi)
			sleep_flag = 1;
		}
	}
	
}

void initPCinterrupts(void) {
	
	GIMSK |= (1 << PCIE);   // pin change interrupt enable
	PCMSK |= (1 << PCINT0); // pin change interrupt enabled for IGN_INPUT (PCINT0)
	PCMSK |= (1 << PCINT2); // pin change interrupt enabled for RPI_INPUT (PCINT2)
	sei();                  // enable interrupts
	
}

int main(void)
{
	// -------- Inits --------- //
	DI_PORT_DDR &= ~(1 << IGN_PIN);  // set IGN_INPUT pin as input
	DI_PORT_DDR &= ~(1 << RPI_PIN);  // set RPI_INPUT pin as input
	
	DO_PORT_DDR |= (1 << PWR_ON_OUTPUT);  // set PWR_ON_OUTPUT pin as output
	
	initPCinterrupts();
	
    while (1) {
		
		if (sleep_flag) {
			
			sleep_flag = 0;
			cli(); // Disable interrupts
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sleep_bod_disable();
			sei();
			sleep_cpu();
			// the AVR now sleeps until IGN_INPUT turns on...
			sleep_disable();
			sei(); // enable interrupts
			
		}
		
    }
}

