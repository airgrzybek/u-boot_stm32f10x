/*
 * flash.c
 *
 *  Created on: 08-12-2013
 *      Author: grzybek
 */


#include <common.h>
#include <asm/arch/stm32f10x.h>

#define SST39VF1601 0x234b

/* Private define ------------------------------------------------------------*/
#define NOR_FLASH_START_ADDR       ((uint32_t)0x64000000)
#define NOR_FLASH_END_ADDR         ((uint32_t)0x64FFFFFF)

/* Delay definition */
#define BlockErase_Timeout         ((uint32_t)0x00A00000)
#define ChipErase_Timeout          ((uint32_t)0x30000000)
#define Program_Timeout            ((uint32_t)0x00001400)

#define ADDR_SHIFT(A)              ((2 * (A)))
#define NOR_WRITE(Address, Data)   (*(vu16 *)(Address) = (Data))

#define BUFFER_SIZE                0x400
#define WRITE_READ_ADDR            0x8000


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

/* NOR Status */
typedef enum
{
  NOR_SUCCESS = 0,
  NOR_ONGOING,
  NOR_ERROR,
  NOR_TIMEOUT
}
NOR_Status;

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (FPW * addr, flash_info_t *info);
static int write_data (flash_info_t *info, ulong dest, FPW data);
static void flash_get_offsets (ulong base, flash_info_t *info);
static NOR_Status FSMC_NOR_GetStatus(uint32_t Timeout);

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
	int i = 0;

	if (info->flash_id == FLASH_UNKNOWN)
	{
		return;
	}

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST)
	{
		for (i = 0; i < info->sector_count; i++)
		{
			info->start[i] = base + (i * 0x1000); // 4kB sector
			info->protect[i] = 0;
		}
	}
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
	FPW manuf = 0, device = 0;
	uint32_t size = 0, sectors = 0;

	addr[0x5555] = (FPW) 0x00AA;
	addr[0x2AAA] = (FPW) 0x0055;
	addr[0x5555] = (FPW) 0x0090;

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
		addr[0] = BS(0xF0); /* restore read mode, (yes, BS is a NOP) */
		return 0; /* no or unknown flash	*/
	}

	device = addr[0x1]; /* device ID		*/
	DEBUGF("Device ID @ 0x%08lx: 0x%08x\n", (ulong )(&addr[0x1]), device);

	addr[0] = (FPW) 0x00F0;	/* restore read mode */

	addr[0x5555] = (FPW) 0x00AA;
	addr[0x2AAA] = (FPW) 0x0055;
	addr[0x5555] = (FPW) 0x0098;

	size = 1 << addr[0x27];
	DEBUGF("size=%#x\n",size);
	sectors = (addr[0x2e] << 8) | addr[0x2d];
	DEBUGF("sectors=%#x\n",sectors);

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
		info->sector_count = sectors + 1;
		info->size = size; // 2 MB
		break;


	default:
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = BS(0xF0); /* restore read mode (yes, a NOP) */
		return 0; /* => no or unknown flash */

	}

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT)
	{
		printf("** ERROR: sector count %d > max (%d) **\n", info->sector_count,
		        CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	//DEBUGF("Device sector size %#x, %#x\n", addr[0xfe], addr[0xfd]);

	addr[0] = (FPW) 0x00F0;	/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last))
	{
		if (info->flash_id == FLASH_UNKNOWN)
		{
			printf("- missing\n");
		}
		else
		{
			printf("- no sectors to erase\n");
		}
		return 1;
	}

	// protected sectors
	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect)
	{
		if (info->protect[sect])
		{
			prot++;
		}
	}

	if (prot)
	{
		printf("- Warning: %d protected sectors will not be erased!\n", prot);
	}
	else
	{
		printf("\n");
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++)
	{
		if (info->protect[sect] == 0)
		{ /* not protected */
			FPWV *base = (FPWV *) (info->start[0]);
			FPWV *addr = (FPWV *) (info->start[sect]);
			FPW status;

			printf("Erasing sector %2d ... ", sect);

			//flash_unprotect_sectors(addr);

			base[0x5555] = 0x00AA;/* clear status register */
			base[0x2AAA] = 0x0055;/* erase setup */
			base[0x5555] = 0x0080;/* erase confirm */
			base[0x5555] = 0x00AA;
			base[0x2AAA] = 0x0055;
			*addr = 0x0030;

			if (NOR_SUCCESS != FSMC_NOR_GetStatus(BlockErase_Timeout ))
			{
				addr[0] = (FPW) 0x00F0;	/* restore read mode */
				printf("Timeout.");
				return 1;
			}

			addr[0] = (FPW) 0x00F0;	/* restore read mode */
			printf(" done\n");
		}
	}
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
	FPW *wp = (FPW *) addr;
	int rc = 0;
	FPW data = 0;

	if (info->flash_id == FLASH_UNKNOWN)
	{
		return 4;
	}

	while (cnt > 0)
	{
		data = 0;

		data = *src++;
		data |= *src++ << 8;


		if ((rc = write_data(info, wp, data)) != 0)
		{
			return rc;
		}
		wp++;
		//src++;
		cnt-=2;
	}

	return cnt;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, ulong dest, FPW data)
{
	FPWV *addr = (FPWV *) dest;
	FPWV * base = info->start[0];
	ulong status;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data)
	{
		printf ("not erased at %08lx (%x)\n", (ulong) addr, *addr);
		return 2;
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[0x5555] = 0x00AA;
	base[0x2AAA] = 0x0055;
	base[0x5555] = 0x00A0;
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag)
	{
		enable_interrupts();
	}

	if (NOR_SUCCESS != FSMC_NOR_GetStatus(Program_Timeout ))
	{
		addr[0] = (FPW) 0x00F0;	/* restore read mode */
		return 1;
	}

	*addr = (FPW) 0x00F0; /* restore read mode */

	return 0;
}


/******************************************************************************
* Function Name  : FSMC_NOR_GetStatus
* Description    : Returns the NOR operation status.
* Input          : - Timeout: NOR progamming Timeout
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
* Attention		 : None
*******************************************************************************/
NOR_Status FSMC_NOR_GetStatus(uint32_t Timeout)
{
  uint16_t val1 = 0x00, val2 = 0x00;
  NOR_Status status = NOR_ONGOING;
  uint32_t timeout = Timeout;

  /* Poll on NOR memory Ready/Busy signal ------------------------------------*/
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) != RESET) && (timeout > 0))
  {
    timeout--;
  }

  timeout = Timeout;

  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == RESET) && (timeout > 0))
  {
    timeout--;
  }

  /* Get the NOR memory operation status -------------------------------------*/
  while((Timeout != 0x00) && (status != NOR_SUCCESS))
  {
    Timeout--;

	  /* Read DQ6 and DQ5 */
    val1 = *(FPWV *)(NOR_FLASH_START_ADDR);
    val2 = *(FPWV *)(NOR_FLASH_START_ADDR);

    /* If DQ6 did not toggle between the two reads then return NOR_Success */
    if((val1 & 0x0040) == (val2 & 0x0040))
    {
      return NOR_SUCCESS;
    }

    if((val1 & 0x0020) != 0x0020)
    {
      status = NOR_ONGOING;
    }

    val1 = *(FPWV *)(NOR_FLASH_START_ADDR);
    val2 = *(FPWV *)(NOR_FLASH_START_ADDR);

    if((val1 & 0x0040) == (val2 & 0x0040))
    {
      return NOR_SUCCESS;
    }
    else if((val1 & 0x0020) == 0x0020)
    {
      return NOR_ERROR;
    }
  }

  if(Timeout == 0x00)
  {
    status = NOR_TIMEOUT;
  }

  /* Return the operation status */
  return (status);
}
