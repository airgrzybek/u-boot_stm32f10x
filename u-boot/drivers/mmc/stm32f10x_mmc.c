#include <common.h>
#include <mmc.h>
#include "stm32f10x_sdcard.h"
#include <part.h>


#ifdef DEBUG
#define debug(fmt, args...) printf(fmt, ##args)
#else
#define debug(...) do { } while(0)
#endif


static block_dev_desc_t mmc_blkdev;

static unsigned long mmc_read_block(int dev, unsigned long start,
        lbaint_t blkcnt, void *buffer);

static unsigned long mmc_write_block(int dev, unsigned long start,
        lbaint_t blkcnt, const void *buffer);

block_dev_desc_t *mmc_get_dev(int dev)
{
	return &mmc_blkdev;
}

int mmc_legacy_init(int verbose)
{
	SD_CardInfo cardInfo;
	SD_Error status = SD_Init();

	debug("mmc_legacy_init status = %d\n",status);

	if (status == SD_OK)
	{
		/*----------------- Read CSD/CID MSD registers ------------------*/
		status = SD_GetCardInfo(&cardInfo);
	}

	if (status == SD_OK)
	{
		/*----------------- Select Card --------------------------------*/
		status = SD_SelectDeselect((u32)(cardInfo.RCA << 16));
	}

	if (status == SD_OK)
	{
		status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
	}

	/* Set Device Transfer Mode to DMA [���ô���ģʽΪDMA��ʽ]*/
	if (status == SD_OK)
	{
		status = SD_SetDeviceMode(SD_DMA_MODE );
	}


	if(SD_OK != status)
	{
		debug("SD_Init() error!\n");
	}
	else
	{
		debug("Device info:\n"
				"block_size = %u\n"
				"capacity = %u\n", cardInfo.CardBlockSize, cardInfo.CardCapacity);

		//mmc_blkdev.lba = (cardInfo.SD_csd.DeviceSize + 1) ;
		//mmc_blkdev.lba *= (1 << (cardInfo.SD_csd.DeviceSizeMul + 2));
		mmc_blkdev.lba = cardInfo.CardCapacity / cardInfo.CardBlockSize;
		mmc_blkdev.if_type = IF_TYPE_MMC;
		mmc_blkdev.part_type = PART_TYPE_DOS;
		mmc_blkdev.blksz = cardInfo.CardBlockSize;
		mmc_blkdev.type = cardInfo.CardType;
		mmc_blkdev.dev = 0;
		mmc_blkdev.block_read = &mmc_read_block;
		mmc_blkdev.block_write = &mmc_write_block;

		//*(u8*) &mmc_blkdev.vendor = cardInfo.SD_cid.ManufacturerID;
		//*(u8*) &mmc_blkdev.product = cardInfo.SD_cid.ProdName1;
		//*(u8*) &mmc_blkdev.revision = cardInfo.SD_cid.ProdRev;

		sprintf((char *) mmc_blkdev.vendor, "Man %02x%04x Snr %08lx",
		        cardInfo.SD_cid.ManufacturerID, cardInfo.SD_cid.OEM_AppliID,
		        cardInfo.SD_cid.ProdSN);
		sprintf((char *) mmc_blkdev.product, "%c%c", cardInfo.SD_cid.ProdName1,
				cardInfo.SD_cid.ProdName2);
		sprintf((char *) mmc_blkdev.revision, "%x %x",
		        cardInfo.SD_cid.ProdRev >> 4, cardInfo.SD_cid.ProdRev & 0x0f);

		init_part(&mmc_blkdev);

	}

	return 0;
}

unsigned long mmc_read_block(int dev, unsigned long start,
        lbaint_t blkcnt, void *buffer)
{
	SD_Error status;
	unsigned long retVal = 0;

	debug("mmc_read_block dev = %d start = %d size = %d\n",dev,start,blkcnt);

	if(1 == blkcnt)
	{
		status = SD_ReadBlock(start << 9 ,(u32 *)(&buffer[0]),mmc_blkdev.blksz);
	}
	else
	{
		status = SD_ReadMultiBlocks(start << 9,(u32 *)(&buffer[0]),mmc_blkdev.blksz,blkcnt);
	}

	debug("SD_ReadMultiBlocks returns status = %d\n",status);

	if(SD_OK != status)
	{
		debug("mmc_read_block read error!\n");
	}
	else
	{
		retVal = blkcnt;
	}

	return retVal;
}

static unsigned long mmc_write_block(int dev, unsigned long start,
        lbaint_t blkcnt, const void *buffer)
{
	SD_Error status;
	unsigned long retVal = 0;

	debug("mmc_write_block dev = %d start = %d size = %d\n",dev,start,blkcnt);

	status = SD_WriteMultiBlocks(start,buffer,mmc_blkdev.blksz,blkcnt);

	debug("SD_ReadMultiBlocks returns status = %d\n",status);

	if(SD_OK != status)
	{
		debug("mmc_write_block read error!\n");
	}
	else
	{
		retVal = blkcnt;
	}

	return retVal;
}
