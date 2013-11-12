#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)stm32f10x_driver_lib.a

COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_adc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_bkp.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_can.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_crc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_dac.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_dbgmcu.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_dma.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_exti.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_flash.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_fsmc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_gpio.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_i2c.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_iwdg.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_pwr.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_rcc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_rtc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_sdio.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_spi.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_tim.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_usart.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += stm32f10x_wwdg.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += misc.o
COBJS-$(CONFIG_STM32_DRIVER_LIB) += core_cm3.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
