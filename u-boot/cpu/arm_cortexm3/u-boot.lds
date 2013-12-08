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

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

#define NVM_BASE	(CONFIG_MEM_NVM_BASE + CONFIG_MEM_NVM_UBOOT_OFF)
#define NVM_LEN		(CONFIG_MEM_NVM_LEN - CONFIG_MEM_NVM_UBOOT_OFF)

#define RAM_BUF_BASE    (CONFIG_MEM_RAM_BASE + CONFIG_MEM_RAM_LEN)
#define MALLOC_BASE     (RAM_BUF_BASE + CONFIG_MEM_RAM_BUF_LEN)
#define STACK_BASE      (MALLOC_BASE + CONFIG_MEM_MALLOC_LEN)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
    RAM_BUF (r): ORIGIN = 0x20008000, LENGTH = (22 * 1024)
    MALLOC (r): ORIGIN =  0x2000D800, LENGTH = (8 * 1024)
    STACK (r): ORIGIN = 0x2000F800, LENGTH = (2 * 1024)
}

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))

        *(.text .text.* .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)

        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)

        . = ALIGN(4);
        KEEP(*(.init))

        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        . = ALIGN(0x4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;

        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))

        . = ALIGN(8);
        _etext = .;

    } >FLASH

    /* .ARM.exidx is sorted, so has to go in its own output section. */
    __exidx_start = .;

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >FLASH
    __exidx_end = .;

    _flash_data = .;
    .data :
    {
        _data_start = .;
        _data_lma_start = LOADADDR(.data);
        *(vtable)
        *(.data*)
        . = ALIGN(4);
        *(.ramcode)
        _data_end = .;
    } > SRAM AT > FLASH

    .u_boot_cmd :
    {
        . = ALIGN(4);
        __u_boot_cmd_start = .;
        *(.u_boot_cmd)
        __u_boot_cmd_end = .;
    } >FLASH

    .bss :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
        _end = .;
        _bss_end = .;
    } > SRAM
    
    .stack :
    {
        *(.stack)
    } >STACK
}

_mem_nvm_base       = ORIGIN(FLASH);
_mem_nvm_size       = LENGTH(FLASH);
_mem_ram_base       = ORIGIN(SRAM);
_mem_ram_size       = LENGTH(SRAM);
_mem_ram_buf_base   = ORIGIN(RAM_BUF);
_mem_ram_buf_size   = LENGTH(RAM_BUF);
_mem_malloc_base    = ORIGIN(MALLOC);
_mem_malloc_size    = LENGTH(MALLOC);
_mem_stack_base     = ORIGIN(STACK);
_mem_stack_size     = LENGTH(STACK);
_mem_stack_end      = ORIGIN(STACK) + LENGTH(STACK);
