/*
 * stm32f10x_nand.c
 *
 *  Created on: 05-01-2014
 *      Author: grzybek
 */


#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/stm32f10x.h>

static void stm32f10x_nand_hwcontrol(struct mtd_info *mtd,
					 int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
		IO_ADDR_W &= ~(CONFIG_SYS_NAND_MASK_ALE
			     | CONFIG_SYS_NAND_MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_ALE;


		this->IO_ADDR_W = (void *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SYS_NAND_READY_PIN
static int stm32f10x_nand_ready(struct mtd_info *mtd)
{
	return GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6);
}
#endif

int board_nand_init(struct nand_chip *nand)
{
	nand->ecc.mode = NAND_ECC_SOFT;
#ifdef CONFIG_SYS_NAND_DBW_16
	nand->options = NAND_BUSWIDTH_16;
#endif
	nand->cmd_ctrl = stm32f10x_nand_hwcontrol;
#ifdef CONFIG_SYS_NAND_READY_PIN
	nand->dev_ready = stm32f10x_nand_ready;
#endif
	nand->chip_delay = 20;

	return 0;
}
