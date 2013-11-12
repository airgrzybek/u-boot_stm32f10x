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
 * Board specific code for the STMicroelectronic STM3220G-EVAL board
 */

#include <common.h>
#include <netdev.h>
#include <ili932x.h>

#include <asm/arch/stm32f10x.h>
#include <asm/arch/FSMC_config.h>

DECLARE_GLOBAL_DATA_PTR;


/*
 * Dump pertinent info to the console.
 */
int checkboard(void)
{
	printf("Board: DRAGON board + STM-MEM add-on,%s\n",
		CONFIG_SYS_BOARD_REV_STR);

	return 0;
}

/*
 * Early hardware init.
 */
int board_init(void)
{
	int rv = 0;

	return 0;
}



/*
 * Setup external RAM.
 */
int dram_init(void)
{
	int				rv = 0;

	FSMC_Init();

	/*
	 * Fill in global info with description of SRAM configuration
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SYS_RAM_SIZE;

	return rv;
}

