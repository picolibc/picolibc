/* or1k_uart.c -- UART implementation for OpenRISC 1000.
 *
 *Copyright (c) 2014 Authors
 *
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 * Contributor Olof Kindgren <olof.kindgren@gmail.com>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include "include/or1k-support.h"
#include "or1k_uart.h"

#include <stdint.h>

// Register interface
#define RB  _or1k_board_uart_base + 0 // Receiver Buffer (R)
#define THR _or1k_board_uart_base + 0 // Transmitter Holding Register (W)
#define IER _or1k_board_uart_base + 1 // Interrupt Enable Register (RW)
#define IIR _or1k_board_uart_base + 2 // Interrupt Identification Register (R)
#define FCR _or1k_board_uart_base + 2 // FIFO Control Register (W)
#define LCR _or1k_board_uart_base + 3 // Line Control Register (RW)
#define MCR _or1k_board_uart_base + 4 // Modem Control Register (W)
#define LSR _or1k_board_uart_base + 5 // Line Status Register (R)
#define MSR _or1k_board_uart_base + 6 // Modem Status Register (R)

// Divisor Register (Accessed when DLAB bit in LCR is set)
#define DLB1 _or1k_board_uart_base + 0 // Divisor Latch LSB (RW)
#define DLB2 _or1k_board_uart_base + 1 // Divisor Latch MSB (RW)

// Interrupt Enable Register bits
#define IER_RDAI 0 // Receiver Data Available Interrupt
#define IER_TEI  1 // Transmitter Holding Register Empty Interrupt
#define IER_RLSI 2 // Receiver Line Status Interrupt
#define IER_MSI  3 // Modem Status Interrupt

// Interrupt Identification Register Values
#define IIR_RLS  0xC6 // Receiver Line Status
#define IIR_RDA  0xC4 // Receiver Data Available
#define IIR_TO   0xCC // Timeout
#define IIR_THRE 0xC2 // Transmitter Holding Register Empty
#define IIT_MS   0xC0 // Modem Status

// FIFO Control Register bits
#define FCR_CLRRECV 0x1  // Clear receiver FIFO
#define FCR_CLRTMIT 0x2  // Clear transmitter FIFO

// FIFO Control Register bit 7-6 values
#define FCR_TRIG_1  0x0  // Trigger level 1 byte
#define FCR_TRIG_4  0x40 // Trigger level 4 bytes
#define FCR_TRIG_8  0x80 // Trigger level 8 bytes
#define FCR_TRIG_14 0xC0 // Trigger level 14 bytes

// Line Control Reigster values and bits
#define LCR_BPC_5 0x0  // 5 bits per character
#define LCR_BPC_6 0x1  // 6 bits per character
#define LCR_BPC_7 0x2  // 7 bits per character
#define LCR_BPC_8 0x3  // 8 bits per character
#define LCR_SB_1  0x0  // 1 stop bit
#define LCR_SB_2  0x4  // 1.5 stop bits (LCR_BPC_5) or 2 stop bits (else)
#define LCR_PE    0x8  // Parity Enabled
#define LCR_EPS   0x10 // Even Parity Select
#define LCR_SP    0x20 // Stick Parity
#define LCR_BC    0x40 // Break Control
#define LCR_DLA   0x80 // Divisor Latch Access

// Line Status Register
#define LSR_DR  0x0  // Data Ready
#define LSR_OE  0x2  // Overrun Error
#define LSR_PE  0x4  // Parity Error
#define LSR_FE  0x8  // Framing Error
#define LSR_BI  0x10 // Break Interrupt
#define LSR_TFE 0x20 // Transmitter FIFO Empty
#define LSR_TEI 0x40 // Transmitter Empty Indicator

/**
 * The registered callback function
 */
void (*_or1k_uart_read_cb)(char c);

/**
 * This is the interrupt handler that is registered for the callback
 * function.
 */
void _or1k_uart_interrupt_handler(uint32_t data)
{
	uint8_t iir = REG8(IIR);

	// Check if this is a read fifo or timeout interrupt, bit 0
	// indicates pending interrupt and the other bits are IIR_RDA
	// or IIR_TO
	if (!(iir & 0x1) || ((iir & 0xfe) != IIR_RDA) ||
	    ((iir & 0xfe) != IIR_TO)) {
		return;
	}

	// Read character and call callback function
	_or1k_uart_read_cb(REG8(RB));
}

int _or1k_uart_init(void)
{
	uint16_t divisor;

	// Is uart present?
	if (!_or1k_board_uart_base) {
		return -1;
	}

	// Reset the callback function
	_or1k_uart_read_cb = 0;

	// Calculate and set divisor
	divisor = _or1k_board_clk_freq / (_or1k_board_uart_baud * 16);
	REG8(LCR) = LCR_DLA;
	REG8(DLB1) = divisor & 0xff;
	REG8(DLB2) = divisor >> 8;

	// Set line control register:
	//  - 8 bits per character
	//  - 1 stop bit
	//  - No parity
	//  - Break disabled
	//  - Disallow access to divisor latch
	REG8(LCR) = LCR_BPC_8;

	// Reset FIFOs and set trigger level to 14 bytes
	REG8(FCR) = FCR_CLRRECV | FCR_CLRTMIT | FCR_TRIG_14;

	// Disable all interrupts
	REG8(IER) = 0;

	return 0;
}

void _or1k_uart_write(char c)
{
	// Wait until FIFO is empty
	while (!(REG8(LSR) & LSR_TFE)) {}

	// Write character to device
	REG8(THR) = c;
}

void or1k_uart_set_read_cb(void (*cb)(char c))
{
	// Set callback function
	_or1k_uart_read_cb = cb;

	// Enable interrupt
	REG8(IER) = 1 << IER_RDAI;

	// Add the interrupt handler that calls the callback function
	or1k_interrupt_handler_add(_or1k_board_uart_IRQ,
			_or1k_uart_interrupt_handler, 0);

	// Enable UART interrupt
	or1k_interrupt_enable(_or1k_board_uart_IRQ);
}
