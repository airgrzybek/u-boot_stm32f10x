/*
 * flash.c
 *
 *  Created on: 08-12-2013
 *      Author: grzybek
 */


#include <common.h>
#include <asm/arch/stm32f10x.h>

#define SST39VF1601 0x4b

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR  (CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE  CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

#undef FLASH_PORT_WIDTH32
#define FLASH_PORT_WIDTH16

#ifdef FLASH_PORT_WIDTH16
#define FLASH_PORT_WIDTH		ushort
#define FLASH_PORT_WIDTHV		vu_short
#define SWAP(x)			__swab16(x)
#else
#define FLASH_PORT_WIDTH		ulong
#define FLASH_PORT_WIDTHV		vu_long
#define SWAP(x)			__swab32(x)
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

/*---------------------------------------------------------------------*/
#define DEBUG_FLASH

#ifdef DEBUG_FLASH
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (FPW * addr, flash_info_t *info);
static int write_data (flash_info_t *info, uchar *dest, uchar data);
static void flash_get_offsets (ulong base, flash_info_t *info);

#define BS(b)     (b)
#define BYTEME(x) ((x) & 0xFF)

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long flash_banks[CONFIG_SYS_MAX_FLASH_BANKS] =
			CONFIG_SYS_FLASH_BANKS_LIST;
	unsigned long size, size_b[CONFIG_SYS_MAX_FLASH_BANKS];

	int i = 0;

	printf("stm32f10x NOR flash init\n\r");

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
	{
		flash_info[i].flash_id = FLASH_UNKNOWN;

		DEBUGF("Get flash bank %d @ 0x%08lx\n", i, flash_banks[i]);
		/*
		 size_b[i] = flash_get_size((vu_char *)flash_banks[i], &flash_info[i]);
		 */
		size_b[i] = flash_get_size((FPW *) 0x64000000, &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN)
		{
			printf("## Unknown FLASH on Bank %d: "
					"ID 0x%lx, Size = 0x%08lx = %ld MB\n", i,
			        flash_info[i].flash_id, size_b[i], size_b[i] << 20);
		}
		else
		{
			DEBUGF("## Flash bank %d at 0x%08lx sizes: 0x%08lx \n", i,
			        flash_banks[i], size_b[i]);

			flash_get_offsets(flash_banks[i], &flash_info[i]);
			flash_info[i].size = size_b[i];
		}
	}

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	DEBUGF("protect environtment %x @ %x\n", CONFIG_ENV_ADDR, CONFIG_ENV_SECT_SIZE);
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR,
			CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
			&flash_info[0]);
#endif

	size = 0;
	DEBUGF("## Final Flash bank sizes: ");
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
	{
		DEBUGF("%08lx ", size_b[i]);
		size += size_b[i];
	}
	DEBUGF("\n");
	return (size);
	return 0;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	return;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK)
	{
	case FLASH_MAN_SST:
		printf("SST ");
		break;

	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK)
	{
	case SST39VF1601:
		printf("SST39VF1601 (16MBit x 16)");
		break;
	default:
			printf ("Unknown Chip Type\n");
			break;
	}

#if 1
	if (info->size >= (1 << 20)) {
		i = 20;
	} else {
		i = 10;
	}
	printf ("  Size: %ld %cB in %d Sectors\n",
		info->size >> i,
		(i == 20) ? 'M' : 'k',
		info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
#endif
	return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (FPW * addr, flash_info_t *info)
{
	vu_char manuf = 0, device;

	addr[0x5555] = (FPW) 0x00AA00AA;
	addr[0x2AAA] = (FPW) 0x00550055;
	addr[0x5555] = (FPW) 0x00900090;

	manuf = addr[0];
	DEBUGF("Manuf. ID @ 0x%08lx: 0x%08x\n", (ulong )addr, manuf);

	switch (manuf)
	{
	case BYTEME(AMD_MANUFACT):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case BYTEME(FUJ_MANUFACT):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case BYTEME(SST_MANUFACT):
		info->flash_id = FLASH_MAN_SST;
		break;
	case BYTEME(STM_MANUFACT):
		info->flash_id = FLASH_MAN_STM;
		break;
	case BYTEME(INTEL_MANUFACT):
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = BS(0xFF); /* restore read mode, (yes, BS is a NOP) */
		return 0; /* no or unknown flash	*/
	}

	device = addr[1]; /* device ID		*/

	DEBUGF("Device ID @ 0x%08lx: 0x%08x\n", (ulong )(&addr[1]), device);

	switch (device)
	{
	case BYTEME(INTEL_ID_28F320J3A):
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000;
		break; /* =>  4 MB		*/

	case SST39VF1601:
		DEBUGF("Device SST39VF1601\n");
		info->flash_id += SST39VF1601;
		info->sector_count = 512;
		info->size = 0x00200000; // 2 MB
		break;


	default:
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = BS(0xFF); /* restore read mode (yes, a NOP) */
		return 0; /* => no or unknown flash */

	}

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT)
	{
		printf("** ERROR: sector count %d > max (%d) **\n", info->sector_count,
		        CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	//DEBUGF("Device sector size %#x, %#x\n", addr[0xfe], addr[0xfd]);

	addr[0] = (FPW) 0x00FF00FF;	/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

#define	FLASH_WIDTH	1	/* flash bus width in bytes */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	return cnt;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, uchar *dest, uchar data)
{

	return 0;
}
