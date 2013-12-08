/*
 * (C) Copyright 2011
 *
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * STM32 USART driver; configured with the following options:
 * - CONFIG_STM32_USART_CONSOLE
 * - CONFIG_STM32_USART_PORT       (1..6)
 * - CONFIG_STM32_USART_TX_IO_PORT (0..8 <-> A..I)
 * - CONFIG_STM32_USART_RX_IO_PORT (0..8 <-> A..I)
 * - CONFIG_STM32_USART_TX_IO_PIN  (0..15)
 * - CONFIG_STM32_USART_RX_IO_PIN  (0..15)
 */

#include <common.h>
//#include <dragon.h>
#include <asm/arch/stm32f10x.h>
#include <asm/arch/stm32f10x_usart.h>
#include <asm/arch/stm32f10x_gpio.h>
#include <asm/arch/stm32f10x_rcc.h>


/*
 * Initialize the serial port.
 */
s32 serial_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_InitClockStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_TypeDef * USARTx = USART1;

	/* Enable USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA,
			ENABLE);

	/* Configure USART1 Rx (PA10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Tx (PA9) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitClockStructure.USART_Clock = USART_Clock_Disable;
	USART_InitClockStructure.USART_CPOL = USART_CPOL_Low;
	USART_InitClockStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_InitClockStructure.USART_LastBit = USART_LastBit_Disable;

	USART_Init(USARTx, &USART_InitStructure);
	USART_ClockInit(USARTx, &USART_InitClockStructure);

	USART_Cmd(USARTx, ENABLE);

	return 0;
}

/*
 * Set new baudrate.
 */
void serial_setbrg(void)
{

}

/*
 * Read a single character from the serial port.
 */
s32 serial_getc(void)
{
	while(USART_GetFlagStatus( USART1, USART_FLAG_RXNE ) == RESET);
	return USART_ReceiveData(USART1);
}

/*
 * Put a single character to the serial port.
 */
void serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while(USART_GetFlagStatus( USART1, USART_FLAG_TXE ) == RESET);
	USART_SendData(USART1,(uint16_t)c);
}

/*
 * Put a string ('\0'-terminated) to the serial port.
 */
void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

/*
 * Test whether a character in in the RX buffer.
 */
s32 serial_tstc(void)
{
	return (USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET ? 1:0);
	//return (usart_regs->sr & STM32_USART_SR_RXNE) ? 1 : 0;
}
