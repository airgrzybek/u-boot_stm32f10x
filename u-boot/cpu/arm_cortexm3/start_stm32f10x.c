/*
 * (C) Copyright 2010,2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
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
#include <config.h>
#include <string.h>
#include "wdt.h"

#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
#include <asm/arch/lpc18xx_gpio.h>
#endif

#if defined(CONFIG_STM32F10X)
#include "asm/arch/system_stm32f10x.h"
#endif

#define WEAK __attribute__ ((weak))

/* Includes ----------------------------------------------------------------------*/

void WEAK Reset_Handler(void);
void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK MemManage_Handler(void);
void WEAK BusFault_Handler(void);
void WEAK UsageFault_Handler(void);
void WEAK MemManage_Handler(void);
void WEAK vPortSVCHandler(void);
void WEAK DebugMon_Handler(void);
void WEAK xPortPendSVHandler(void);
void WEAK xPortSysTickHandler(void);

/* External Interrupts */
void WEAK WWDG_IRQHandler(void);
void WEAK PVD_IRQHandler(void);
void WEAK TAMPER_IRQHandler(void);
void WEAK RTC_IRQHandler(void);
void WEAK FLASH_IRQHandler(void);
void WEAK RCC_IRQHandler(void);
void WEAK EXTI0_IRQHandler(void);
void WEAK EXTI1_IRQHandler(void);
void WEAK EXTI2_IRQHandler(void);
void WEAK EXTI3_IRQHandler(void);
void WEAK EXTI4_IRQHandler(void);
void WEAK DMA1_Channel1_IRQHandler(void);
void WEAK DMA1_Channel2_IRQHandler(void);
void WEAK DMA1_Channel3_IRQHandler(void);
void WEAK DMA1_Channel4_IRQHandler(void);
void WEAK DMA1_Channel5_IRQHandler(void);
void WEAK DMA1_Channel6_IRQHandler(void);
void WEAK DMA1_Channel7_IRQHandler(void);
void WEAK ADC1_2_IRQHandler(void);
void WEAK USB_HP_CAN1_TX_IRQHandler(void);
void WEAK USB_LP_CAN1_RX0_IRQHandler(void);
void WEAK CAN1_RX1_IRQHandler(void);
void WEAK CAN1_SCE_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void WEAK TIM1_BRK_IRQHandler(void);
void WEAK TIM1_UP_IRQHandler(void);
void WEAK TIM1_TRG_COM_IRQHandler(void);
void WEAK TIM1_CC_IRQHandler(void);
void WEAK TIM2_IRQHandler(void);
void WEAK TIM3_IRQHandler(void);
void WEAK TIM4_IRQHandler(void);
void WEAK I2C1_EV_IRQHandler(void);
void WEAK I2C1_ER_IRQHandler(void);
void WEAK I2C2_EV_IRQHandler(void);
void WEAK I2C2_ER_IRQHandler(void);
void WEAK SPI1_IRQHandler(void);
void WEAK SPI2_IRQHandler(void);
void WEAK USART1_IRQHandler(void);
void WEAK USART2_IRQHandler(void);
void WEAK USART3_IRQHandler(void);
void WEAK EXTI15_10_IRQHandler(void);
void WEAK RTCAlarm_IRQHandler(void);
void WEAK USBWakeUp_IRQHandler(void);
void WEAK TIM8_BRK_IRQHandler(void);
void WEAK TIM8_UP_IRQHandler(void);
void WEAK TIM8_TRG_COM_IRQHandler(void);
void WEAK TIM8_CC_IRQHandler(void);
void WEAK ADC3_IRQHandler(void);
void WEAK FSMC_IRQHandler(void);
void SDIO_IRQHandler(void);
void WEAK TIM5_IRQHandler(void);
void WEAK SPI3_IRQHandler(void);
void WEAK UART4_IRQHandler(void);
void WEAK UART5_IRQHandler(void);
void WEAK TIM6_IRQHandler(void);
void WEAK TIM7_IRQHandler(void);
void WEAK DMA2_Channel1_IRQHandler(void);
void WEAK DMA2_Channel2_IRQHandler(void);
void WEAK DMA2_Channel3_IRQHandler(void);
void DMA2_Channel4_5_IRQHandler(void);
void WEAK SystemInit_ExtMemCtl(void);

 /*
 * FIXME: move to the appropriate header
 */
unsigned char cortex_m3_irq_vec_get(void);

/*
 * Declare symbols used in the start-up code
 */
extern void printf(const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));

extern char _data_lma_start;
extern char _data_start;
extern char _data_end;

extern char _mem_stack_base, _mem_stack_end;
unsigned long _armboot_start;

extern char _bss_start;
extern char _bss_end;

void _start(void);
void default_isr(void);

extern void start_armboot(void);

#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
extern void lpc18xx_norflash_bootstrap_iomux_init(void);
#endif

/*
 * Control IRQs
 */
static inline void __attribute__((used)) __enable_irq(void)
{
	asm volatile ("cpsie i");
}

static inline void __attribute__((used)) __disable_irq(void)
{
	asm volatile ("cpsid i");
}

/*
 * Exception-processing vectors:
 */
unsigned int vectors[] __attribute__((section(".vectors"))) =
{

(unsigned long) &_mem_stack_end, // The initial stack pointer
(unsigned int)&_start, /* Reset Handler */
        NMI_Handler, /* NMI Handler */
        HardFault_Handler, /* Hard Fault Handler */
        MemManage_Handler, /* MPU Fault Handler */
        BusFault_Handler, /* Bus Fault Handler */
        UsageFault_Handler, /* Usage Fault Handler */
        0, /* Reserved */
        0, /* Reserved */
        0, /* Reserved */
        0, /* Reserved */
        vPortSVCHandler, /* SVCall Handler */
        DebugMon_Handler, /* Debug Monitor Handler */
        0, /* Reserved */
        xPortPendSVHandler, /* PendSV Handler */
        xPortSysTickHandler, /* SysTick Handler */
        /* External Interrupts */
        WWDG_IRQHandler, /* Window Watchdog */
        PVD_IRQHandler, /* PVD through EXTI Line detect */
        TAMPER_IRQHandler, /* Tamper */
        RTC_IRQHandler, /* RTC */
        FLASH_IRQHandler, /* Flash */
        RCC_IRQHandler, /* RCC */
        EXTI0_IRQHandler, /* EXTI Line 0 */
        EXTI1_IRQHandler, /* EXTI Line 1 */
        EXTI2_IRQHandler, /* EXTI Line 2 */
        EXTI3_IRQHandler, /* EXTI Line 3 */
        EXTI4_IRQHandler, /* EXTI Line 4 */
        DMA1_Channel1_IRQHandler, /* DMA1 Channel 1 */
        DMA1_Channel2_IRQHandler, /* DMA1 Channel 2 */
        DMA1_Channel3_IRQHandler, /* DMA1 Channel 3 */
        DMA1_Channel4_IRQHandler, /* DMA1 Channel 4 */
        DMA1_Channel5_IRQHandler, /* DMA1 Channel 5 */
        DMA1_Channel6_IRQHandler, /* DMA1 Channel 6 */
        DMA1_Channel7_IRQHandler, /* DMA1 Channel 7 */
        ADC1_2_IRQHandler, /* ADC1 & ADC2 */
        USB_HP_CAN1_TX_IRQHandler, /* USB High Priority or CAN1 TX */
        USB_LP_CAN1_RX0_IRQHandler, /* USB Low  Priority or CAN1 RX0 */
        CAN1_RX1_IRQHandler, /* CAN1 RX1 */
        CAN1_SCE_IRQHandler, /* CAN1 SCE */
        EXTI9_5_IRQHandler, /* EXTI Line 9..5 */
        TIM1_BRK_IRQHandler, /* TIM1 Break */
        TIM1_UP_IRQHandler, /* TIM1 Update */
        TIM1_TRG_COM_IRQHandler, /* TIM1 Trigger and Commutation */
        TIM1_CC_IRQHandler, /* TIM1 Capture Compare */
        TIM2_IRQHandler, /* TIM2 */
        TIM3_IRQHandler, /* TIM3 */
        TIM4_IRQHandler, /* TIM4 */
        I2C1_EV_IRQHandler, /* I2C1 Event */
        I2C1_ER_IRQHandler, /* I2C1 Error */
        I2C2_EV_IRQHandler, /* I2C2 Event */
        I2C2_ER_IRQHandler, /* I2C2 Error */
        SPI1_IRQHandler, /* SPI1 */
        SPI2_IRQHandler, /* SPI2 */
        USART1_IRQHandler, /* USART1 */
        USART2_IRQHandler, /* USART2 */
        USART3_IRQHandler, /* USART3 */
        EXTI15_10_IRQHandler, /* EXTI Line 15..10 */
        RTCAlarm_IRQHandler, /* RTC Alarm through EXTI Line */
        USBWakeUp_IRQHandler, /* USB Wakeup from suspend */
        TIM8_BRK_IRQHandler, /* TIM8 Break */
        TIM8_UP_IRQHandler, /* TIM8 Update */
        TIM8_TRG_COM_IRQHandler, /* TIM8 Trigger and Commutation */
        TIM8_CC_IRQHandler, /* TIM8 Capture Compare */
        ADC3_IRQHandler, /* ADC3 */
        FSMC_IRQHandler, /* FSMC */
        SDIO_IRQHandler, /* SDIO */
        TIM5_IRQHandler, /* TIM5 */
        SPI3_IRQHandler, /* SPI3 */
        UART4_IRQHandler, /* UART4 */
        UART5_IRQHandler, /* UART5 */
        TIM6_IRQHandler, /* TIM6 */
        TIM7_IRQHandler, /* TIM7 */
        DMA2_Channel1_IRQHandler, /* DMA2 Channel 1 */
        DMA2_Channel2_IRQHandler, /* DMA2 Channel 2 */
        DMA2_Channel3_IRQHandler, /* DMA2 Channel 3 */
        DMA2_Channel4_5_IRQHandler, /* DMA2 Channel 4 and Channel 5 */
        0, 0, 0, 0, 0, 0, 0, 0, /* @0x130 */
        0, 0, 0, 0, 0, 0, 0, 0, /* @0x150 */
        0, 0, 0, 0, 0, 0, 0, 0, /* @0x170 */
        0, 0, 0, 0, 0, 0, 0, 0, /* @0x190 */
        0, 0, 0, 0, 0, 0, 0, 0, /* @0x1B0 */
        0, 0, 0, 0, /* @0x1D0 */
//(void *)0xF1E0F85F          /* @0x1E0. This is for boot in RAM mode for
        };

#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
/*
 * This function will be called very early on U-Boot initialization to reload
 * the whole U-Boot image from NOR flash if we use bootloading from NOR flash.
 */
void __attribute__((section(".lpc18xx_image_top_text")))
	lpc18xx_bootstrap_from_norflash(void);
#endif /* CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND */

 /*
  * Reset entry point
  */
void
#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
	__attribute__((section(".lpc18xx_image_top_text")))
#endif
	_start(void)
{

#if defined(CONFIG_SYS_STM32F10X)
	SystemInit();
#endif
	/*
	 * Depending on the config parameter, enable or disable the WDT.
	 */
#if !defined(CONFIG_HW_WATCHDOG)
#if !defined(CONFIG_SYS_M2S)
	wdt_disable();
#endif
#else
	wdt_enable();
#endif

	/*
	 * Make sure interrupts are disabled.
	 */
	__disable_irq();

#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
	/*
	 * Reload the whole U-Boot image from NOR flash.
	 * The Boot ROM on LPC4350 parts cannot load more than 32KBytes
	 * from NOR flash when booting.
	 */
	lpc18xx_bootstrap_from_norflash();
#endif /* CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND */

	/*
	 * Copy data and initialize BSS
	 * This is in lieu of the U-boot "conventional" relocation
	 * of code & data from Flash to RAM.
	 * With Cortex-M3, we execute from NVRAM (internal Flash),
	 * having relocated data to internal RAM (and having cleared the BSS
	 * area in internal RAM as well)
	 * Stack grows downwards; the stack base is set-up by the first
	 * value in the first word in the vectors.
	 */
	memcpy(&_data_start, &_data_lma_start, &_data_end - &_data_start);
	memset(&_bss_start, 0, &_bss_end - &_bss_start);

	/*
	 * In U-boot (armboot) lingvo, "go to the C code" -
	 * in fact, with M3, we are at the C code from the very beginning.
	 * In actuality, this is the jump to the ARM generic start code.
	 * ...
	 * Note initialization of _armboot_start below. The ARM generic
	 * code expects that this variable is set to the upper boundary of
	 * the malloc pool area.
	 * For Cortex-M3, where we do not relocate the code to RAM, I set
	 * the malloc pool right behind the stack. See how armboot_start
	 * is defined in the CPU specific .lds file.
	 */
	_armboot_start = (unsigned long)&_mem_stack_base;
	start_armboot();
}


/*
 * Default exception handler
 */
void __attribute__((naked, noreturn))
#ifdef CONFIG_LPC18XX_NORFLASH_BOOTSTRAP_WORKAROUND
	__attribute__((section(".lpc18xx_image_top_text")))
#endif
	default_isr(void);

void default_isr(void)
{
	/*
	 * Dump the registers
	 */
	asm("mov r0, sp; bl dump_ctx");

	/* And hang
	 */
	for (;;) ;
}

/*
 * Dump the registers on an exception we don't know how to process.
 */
static void __attribute__((used)) dump_ctx(unsigned int *ctx)
{
	static char *regs[] = {
		"R0", "R1", "R2", "R3", "R12", "LR", "PC", "PSR"
	};
	static char *exc[] = {
		0,
		0,
		"NMI",
		"HARD FAULT",
		"MEMORY MANAGEMENT",
		"BUS FAULT",
		"USAGE FAULT",
		"RESERVED",
		"RESERVED",
		"RESERVED",
		"RESERVED",
		"SVCALL",
		"DEBUG MONITOR",
		"RESERVED",
		"PENDSV",
		"SYSTICK",
};
	unsigned char vec = cortex_m3_irq_vec_get();
	int i;

	printf("UNHANDLED EXCEPTION: ");
	if (vec < 16) {
		printf("%s\n", exc[vec]);
	} else {
		printf("INTISR[%d]\n", vec - 16);
	}
	for (i = 0; i < 8; i++) {
		printf("  %s\t= %08x", regs[i], ctx[i]);
		if (((i + 1) % 2) == 0) {
			printf("\n");
		}
	}
}

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/
#pragma weak NMI_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak vPortSVCHandler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak xPortPendSVHandler = Default_Handler
#pragma weak xPortSysTickHandler = Default_Handler
#pragma weak WWDG_IRQHandler = Default_Handler
#pragma weak PVD_IRQHandler = Default_Handler
#pragma weak TAMPER_IRQHandler = Default_Handler
#pragma weak RTC_IRQHandler = Default_Handler
#pragma weak FLASH_IRQHandler = Default_Handler
#pragma weak RCC_IRQHandler = Default_Handler
#pragma weak EXTI0_IRQHandler = Default_Handler
#pragma weak EXTI1_IRQHandler = Default_Handler
#pragma weak EXTI2_IRQHandler = Default_Handler
#pragma weak EXTI3_IRQHandler = Default_Handler
#pragma weak EXTI4_IRQHandler = Default_Handler
#pragma weak DMA1_Channel1_IRQHandler = Default_Handler
#pragma weak DMA1_Channel2_IRQHandler = Default_Handler
#pragma weak DMA1_Channel3_IRQHandler = Default_Handler
#pragma weak DMA1_Channel4_IRQHandler = Default_Handler
#pragma weak DMA1_Channel5_IRQHandler = Default_Handler
#pragma weak DMA1_Channel6_IRQHandler = Default_Handler
#pragma weak DMA1_Channel7_IRQHandler = Default_Handler
#pragma weak ADC1_2_IRQHandler = Default_Handler
#pragma weak USB_HP_CAN1_TX_IRQHandler = Default_Handler
#pragma weak USB_LP_CAN1_RX0_IRQHandler = Default_Handler
#pragma weak CAN1_RX1_IRQHandler = Default_Handler
#pragma weak CAN1_SCE_IRQHandler = Default_Handler
//#pragma weak EXTI9_5_IRQHandler = Default_Handler
#pragma weak TIM1_BRK_IRQHandler = Default_Handler
#pragma weak TIM1_UP_IRQHandler = Default_Handler
#pragma weak TIM1_TRG_COM_IRQHandler = Default_Handler
#pragma weak TIM1_CC_IRQHandler = Default_Handler
#pragma weak TIM2_IRQHandler = Default_Handler
#pragma weak TIM3_IRQHandler = Default_Handler
#pragma weak TIM4_IRQHandler = Default_Handler
#pragma weak I2C1_EV_IRQHandler = Default_Handler
#pragma weak I2C1_ER_IRQHandler = Default_Handler
#pragma weak I2C2_EV_IRQHandler = Default_Handler
#pragma weak I2C2_ER_IRQHandler = Default_Handler
#pragma weak SPI1_IRQHandler = Default_Handler
#pragma weak SPI2_IRQHandler = Default_Handler
#pragma weak USART1_IRQHandler = Default_Handler
#pragma weak USART2_IRQHandler = Default_Handler
#pragma weak USART3_IRQHandler = Default_Handler
#pragma weak EXTI15_10_IRQHandler = Default_Handler
#pragma weak RTCAlarm_IRQHandler = Default_Handler
#pragma weak USBWakeUp_IRQHandler = Default_Handler
#pragma weak TIM8_BRK_IRQHandler = Default_Handler
#pragma weak TIM8_UP_IRQHandler = Default_Handler
#pragma weak TIM8_TRG_COM_IRQHandler = Default_Handler
#pragma weak TIM8_CC_IRQHandler = Default_Handler
#pragma weak ADC3_IRQHandler = Default_Handler
#pragma weak FSMC_IRQHandler = Default_Handler
//#pragma weak SDIO_IRQHandler = Default_Handler
#pragma weak TIM5_IRQHandler = Default_Handler
#pragma weak SPI3_IRQHandler = Default_Handler
#pragma weak UART4_IRQHandler = Default_Handler
#pragma weak UART5_IRQHandler = Default_Handler
#pragma weak TIM6_IRQHandler = Default_Handler
#pragma weak TIM7_IRQHandler = Default_Handler
#pragma weak DMA2_Channel1_IRQHandler = Default_Handler
#pragma weak DMA2_Channel2_IRQHandler = Default_Handler
#pragma weak DMA2_Channel3_IRQHandler = Default_Handler
//#pragma weak DMA2_Channel4_5_IRQHandler = Default_Handler
#pragma weak SystemInit_ExtMemCtl = SystemInit_ExtMemCtl_Dummy

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/

void Default_Handler(void)
{
	/*
	 * Dump the registers
	 */
	asm("mov r0, sp; bl dump_ctx");

	/* Go into an infinite loop. */
	while (1)
	{
	}
}

void HardFault_Handler(void)
{
	/*
	 * Dump the registers
	 */
	asm("mov r0, sp; bl dump_ctx");

	/* Go into an infinite loop. */
	while (1)
	{
	}
}

/**
 * @brief  Dummy SystemInit_ExtMemCtl function
 * @param  None
 * @retval : None
*/

void SystemInit_ExtMemCtl_Dummy(void)
{
}

