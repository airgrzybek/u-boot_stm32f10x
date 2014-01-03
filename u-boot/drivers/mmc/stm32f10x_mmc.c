#include <common.h>
#include <mmc.h>
#include "stm32f10x_sdcard.h"
#include <part.h>

/*
#define printf
#ifdef printf
#define printf(fmt, args...) printf(fmt, ##args)
#else
#define printf(...) do { } while(0)
#endif
*/

static block_dev_desc_t mmc_blkdev;

static unsigned long mmc_read_block(int dev, unsigned long start,
        lbaint_t blkcnt, void *buffer);

static unsigned long mmc_write_block(int dev, unsigned long start,
        lbaint_t blkcnt, const void *buffer);

static void InterruptsSetup(void);

block_dev_desc_t *mmc_get_dev(int dev)
{
	return &mmc_blkdev;
}

int mmc_legacy_init(int verbose)
{
	SD_CardInfo cardInfo;
	SD_Error status = SD_Init();

	printf("mmc_legacy_init status = %d\n",status);

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
		printf("SD_Init() error!\n");
	}
	else
	{
		printf("Device info:\n"
				"block_size = %u\n"
				"capacity = %u\n", cardInfo.CardBlockSize, cardInfo.CardCapacity);

		mmc_blkdev.if_type = IF_TYPE_MMC;
		mmc_blkdev.part_type = PART_TYPE_DOS;
		mmc_blkdev.blksz = cardInfo.CardBlockSize;
		mmc_blkdev.type = cardInfo.CardType;
		mmc_blkdev.dev = 0;
		mmc_blkdev.block_read = &mmc_read_block;
		mmc_blkdev.block_write = &mmc_write_block;

		init_part(&mmc_blkdev);

		InterruptsSetup();
	}




	return 0;
}

/*void  init_part (block_dev_desc_t *dev_desc)
{

}*/

unsigned long mmc_read_block(int dev, unsigned long start,
        lbaint_t blkcnt, void *buffer)
{
	SD_Error status;
	unsigned long retVal = 0;

	printf("mmc_read_block dev = %d start = %d size = %d\n",dev,start,blkcnt);

	if(1 == blkcnt)
	{
		status = SD_ReadBlock(start << 9 ,(u32 *)(&buffer[0]),mmc_blkdev.blksz);
	}
	else
	{
		status = SD_ReadMultiBlocks(start << 9,(u32 *)(&buffer[0]),mmc_blkdev.blksz,blkcnt);
	}



	printf("SD_ReadMultiBlocks returns status = %d\n",status);

	if(SD_OK != status)
	{
		printf("mmc_read_block read error!\n");
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

	printf("mmc_write_block dev = %d start = %d size = %d\n",dev,start,blkcnt);

	status = SD_WriteMultiBlocks(start,buffer,mmc_blkdev.blksz,blkcnt);

	printf("SD_ReadMultiBlocks returns status = %d\n",status);

	if(SD_OK != status)
	{
		printf("mmc_write_block read error!\n");
	}
	else
	{
		retVal = blkcnt;
	}

	return retVal;
}

static void InterruptsSetup(void)
{
    EXTI_InitTypeDef  EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* Configure PA.00 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Connect PEN EXTI Line to Key Button GPIO Pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);

    /* Configure PEN EXTI Line to generate an interrupt on falling edge */
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Connect PEN EXTI Line to Key Button GPIO Pin
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource7 );

    // Configure PEN EXTI Line to generate an interrupt on falling edge
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // interrupt init
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* Enable the RTC Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Generate software interrupt: simulate a falling edge applied on PEN EXTI line */
    //EXTI_GenerateSWInterrupt(EXTI_Line8);
    //EXTI_GenerateSWInterrupt(EXTI_Line7);
}
