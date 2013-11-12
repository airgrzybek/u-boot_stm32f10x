/*
 * STM32 Embedded flash programming support.
 *
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com
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

#include <common.h>
#include <errno.h>
#include "envm.h"

/*
 * Flash data area definitions
 */
#define STM32_FLASH_BASE		0x08000000
#define STM32_FLASH_SIZE	        (512 * 1024)
/*
 * This array defines the layout of the Embedded Flash on the STM32F2x chips
 */
static u32 flash_bsize[] = {
	[0 ... 3]	=  16 * 1024,
	[4]		=  64 * 1024,
	[5 ... 11]	= 128 * 1024
	};

/*
 * Number of flash blocks for STM32F2x chips
 */
#define STM32_FLASH_BLOCKS		ARRAY_SIZE(flash_bsize)

/*
 * Unlock the Flash Command Register
 */
static void __attribute__((section(".ramcode")))
	     __attribute__ ((long_call))
stm32_flash_cr_unlock(void)
{
	FLASH_Unlock();
}

/*
 * Lock the Flash Command Register
 */
static void __attribute__((section(".ramcode")))
	     __attribute__ ((long_call))
stm32_flash_cr_lock(void)
{
	FLASH_Lock();
}

/*
 * Given the flash address, return the block number.
 * Return error if the address is not a start block address.
 */
/*
 *
 * IMPLEMNTACJA!
 */
static s32 stm32_flash_get_block(u8 *addr)
{
	s32 i = 0;
	u8 *base = (u8 *)STM32_FLASH_BASE;

	while (i < STM32_FLASH_BLOCKS) {
		if (addr == base)
			break;
		base += flash_bsize[i];
		i++;
	}

	if (i == STM32_FLASH_BLOCKS)
		i = -EINVAL;

	return i;
}

/*
 * Erase the embedded flash of the STM32. Start block is calculated from the
 * given offset, end block - from size.
 */
static s32 __attribute__((section(".ramcode")))
	     __attribute__ ((long_call))
stm32_flash_erase(u32 offset, u32 size)
{
	s32 ret;
	s32 n, num;
	u32 erasesize;

	if ((n = stm32_flash_get_block((u8 *)offset)) < 0) {
		printf("%s: Address %#x is not block-aligned\n", __func__,
			offset);
		ret = n;
		goto xit;
	}

	/* Calculate the number of blocks to erase */
	erasesize = 0;
	num = n;
	while (erasesize < size) {
		erasesize += flash_bsize[num];
		num++;
	}

	FLASH_Unlock();

	while (n < num) {
		FLASH_ErasePage(offset+n*2048);
		n++;
	}

	FLASH_Lock();

	ret = 0;
xit:
	return ret;
}

static s32 __attribute__((section(".ramcode")))
	     __attribute__ ((long_call))
stm32_flash_program(u32 offset, void *buf, u32 size)
{
	u32 *src = (u32 *)buf;
	u32 *dst = (u32 *)offset;
	/* I can read 1-3 bytes beyond the input buffer, but this is OK */
	u32 words = (size + sizeof(u32) - 1) / sizeof(u32);
	s32 ret;

	/* No sanity check on flash address here, proceed to program */

	/* Check there is no pending operations */
	if ((FLASH_GetFlagStatus(FLASH_FLAG_BSY)) == SET) {
		ret = -EBUSY;
		goto xit;
	}
	FLASH_Unlock();

	/* Since we are using 32x parallelism (set in CR), copy by 4 bytes */
	while (words--) {
		*dst++ = *src++;

		FLASH_ProgramWord(offset++,*src++);
	}

	FLASH_Lock();

	ret = 0;
xit:
	return ret;
}

/*
 * Enable instruction cache, prefetch and set the Flash wait latency
 * according to the clock configuration used (HCLK value).
 * We _must_ do this before changing System clock source (or will crash on
 * fetching instructions of while() wait cycle).
 * In case of HSI clock - no Sys clock change happens, but, for consistency,
 * we configure Flash this way as well.
 */
/*void envm_config(u32 wait_states)
{
	if (wait_states > STM32_FLASH_ACR_LAT_MSK)
		wait_states = STM32_FLASH_ACR_LAT_MSK;

	STM32_FLASH_REGS->acr = STM32_FLASH_ACR_PRFTEN |
			  STM32_FLASH_ACR_ICEN |
			  (wait_states << STM32_FLASH_ACR_LAT_BIT);
}*/

/*
 * Initialize internal Flash interface
 */
void envm_init(void)
{

}

/*
 * Write a data buffer to internal Flash.
 * Note that we need for this function to reside in RAM since it
 * will be used to self-upgrade U-boot in internal Flash.
 */
u32 __attribute__((section(".ramcode")))
	     __attribute__ ((long_call))
envm_write(u32 offset, void * buf, u32 size)
{
	s32 ret = 0;

	/* Basic sanity check. More checking in the "get_block" routine */
	if ((offset < STM32_FLASH_BASE) ||
		((offset + size) > (STM32_FLASH_BASE + STM32_FLASH_SIZE))) {
		printf("%s: Address %#x is not in flash or size %d is too big\n",
			__func__, offset, size);
		goto xit;
	}

	if (stm32_flash_erase(offset, size) < 0 ||
		(stm32_flash_program(offset, buf, size) < 0))
		goto xit;

	ret = size;
xit:
	return ret;
}
