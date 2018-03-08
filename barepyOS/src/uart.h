#ifndef __HEADER_UART
#define __HEADER_UART

//
// Registres UART
#define UART_BASE 0x3F201000
#define UART_DR		(UART_BASE + 0x00)	// Data register
#define UART_RSRECR	(UART_BASE + 0x04)
#define UART_FR		(UART_BASE + 0x18)	// Flag register
#define UART_ILPR (UART_BASE + 0x20)
#define UART_IBRD	(UART_BASE + 0x24)	// Integer Daud rate divisor
#define UART_FBRD	(UART_BASE + 0x28)	// Fractional Baud rate divisor
#define UART_LCRH	(UART_BASE + 0x2C)	// Line control register
#define UART_CR 	(UART_BASE + 0x30)	// Control register
#define UART_IFLS	(UART_BASE + 0x34)	// Interupt FIFO level select register
#define UART_IMSC	(UART_BASE + 0x38)	// Interupt mask set clear register
#define UART_RIS	(UART_BASE + 0x3C)	// Raw Interupt Status register
#define UART_MIS	(UART_BASE + 0x40)	// Masked Interupt Status Register
#define UART_ICR	(UART_BASE + 0x44) // Interupt clear register
#define UART_DMACR (UART_BASE + 0x48)
#define UART_ITCR	(UART_BASE + 0x80)	// Test Control register
#define UART_ITIP	(UART_BASE + 0x84)	// Integration test input reg
#define UART_ITOP	(UART_BASE + 0x88)	// Integration test output reg
#define UART_TDR	(UART_BASE + 0x8C)	// Test data reg
// Constantes GPIO utiles aux UART
#define UART_TXD0_PIN	14u	// UART TDX0 pin sur ALT0
#define UART_RDX0_PIN	15u // UART RDX0 pin sur ALT0
#define GPIO_ALT0		4u	// GPIO fonction ALT0
#define GPIO_TDX0_OFF	((14u % 10u) * 3u)
#define GPIO_RDX0_OFF	((15u % 10u) * 3u)

//
// Variable d'erreur
extern int uart_error;

// Initialise l'UART
void uart_init(void);

// Envoie un caractère
void uart_send_char(const char c);

// Permet d'envoyer des chaine de caractère. Celle-ci doit se terminer
//	par le caractère nul.
void uart_send_str(const char *data);

// Permet d'envoyer un entier signé
void uart_send_int(int n);

// Permet de savoir si la receive fifo est vide
int uart_is_receive_fifo_empty(void);

// Permet de recevoir une chaine de caractère.
// Bloque jusqu'a la reception de n-1 caractères, ou la reception
//	du caractère nul.
// NOTE : Un caractère nul est placé automatiquement en fin de
//	chaine contenu dans le buffer.
int uart_receive_str(char *buffer, unsigned int n);

// Permet de lire un entier signé.
// Renvoie la valeur lue
// NOTE : en cas d'erreur, uart_error est mis à -1
int uart_receive_int(void);

#endif
