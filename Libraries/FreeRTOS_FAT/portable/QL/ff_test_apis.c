/*==========================================================
 * *
 *    File   : ff_app.c
 *    Purpose: This file is a defines for custom commands of FS and test APIs
 *
 *=========================================================*/
#include "Fw_global_config.h"
#include <stdio.h>
#include <string.h>
#include "ff_headers.h"
#include "ff_stdio.h"


#ifdef FS_TESTING

#define TEST_FILENAME	"/SPIFLASH/test.txt"
#define NSECTORS 		(50)
char testbuf[NSECTORS*SPIFLASH_DISK_SECTOR_SIZE];
FF_FILE *fr;

void write_test(void)
{
	fr = ff_fopen(TEST_FILENAME, "wr");
	configASSERT( fr);

	for(int i=0;i<NSECTORS;i++)
	{
		for(int j=0;j<SPIFLASH_DISK_SECTOR_SIZE;j++)
		{
			testbuf[(i*SPIFLASH_DISK_SECTOR_SIZE)+j]=(char)(i+1)&0x000000FF;
		}
	}

	ff_fwrite(testbuf,SPIFLASH_DISK_SECTOR_SIZE,NSECTORS,fr);

#if 1 //READBACK

	memset(testbuf,0,sizeof(testbuf));

	ff_rewind(fr);

	ff_fread(testbuf,SPIFLASH_DISK_SECTOR_SIZE,NSECTORS,fr);

	printf("\n");

	for(int i=0;i<NSECTORS;i++)
	{
		for(int j=0;j<4;j++)
		{
			printf(" 0x%x",testbuf[(i*4096)+j]);
		}
	}

	printf("\n");

#endif

	ff_fclose(fr);
}

void read_test()
{
	char c='A';
	fr = ff_fopen(TEST_FILENAME, "r");
	configASSERT( fr);

	while( ff_feof( fr ) == 0 )
	{
		c = ff_fgetc(fr); /* read from file*/
		printf("%c",c); /*  display on screen*/
	}

	ff_fclose(fr);
}

void FS_rw_test(void)
{

	printf("\n Starting RW test\n");
	write_test();
	//read_test();

	printf("\n RW test DONE\n");
	return ;
}
#endif



