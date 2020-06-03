/*==========================================================
 * *
 *    File   : ff_app.c
 *    Purpose: This file used by BL1 only.
 *             It defines API to read files into SRAM and verify them.
 *
 *=========================================================*/

#include <stdio.h>
#include <string.h>
#include "ff_headers.h"
#include "ff_stdio.h"
#include "Bootconfig.h"
#include "crc32.h"

/*
	This data is added to the file at the its end when sent by the flash tool
*/
typedef struct file_trailer_t {
  unsigned int size; //0:3 bytes
  unsigned int crc32; //4:7 bytes
  unsigned int Rsrvd1; //8:11 bytes
  unsigned int signature; //12:15 bytes= 0x514C424C = "QLBL"  BootLoader signature
} FILE_TRAILER;

//Read a file from flash, calc crc of the contents(excluding the trailer) and verify with crc in the trailer */
uint8_t verify_file_crc(const char* filename,uint32_t* file_size_out)
{
	FF_FILE *fp;
	FILE_TRAILER file_trailer;

    printf("Verify: %s\n", filename);
	fp = ff_fopen(filename, "r");
	if(fp==NULL)
	{
		printf("\n Could not open file %s\n",filename);
		return 0;
	}

	size_t fsize=0;

	fsize=ff_filelength(fp);

	if(fsize==0)
	{
		printf("\n Error getting file size \n");
		return 0;
	}

	ff_rewind(fp);

	ff_fseek( fp, -(sizeof(file_trailer)), FF_SEEK_END );

	//Read the header
	ff_fread(&file_trailer,sizeof(file_trailer),1,fp);

	ff_rewind(fp);

	//Read file, calc and verify CRC

	unsigned int calc_crc32=0xFFFFFFFF;

	//printf("\nAvailable size %d B\n",FREE_SRAM_AREA_SIZE);

	if((fsize-sizeof(file_trailer))>FREE_SRAM_AREA_SIZE)
	{
		printf("\nNo space in SRAM to read %s, available space %d B",filename,FREE_SRAM_AREA_SIZE);
		return 0;
	}

	ff_fread((void*)FREE_SRAM_AREA,(fsize-sizeof(file_trailer)),1,fp);
	calc_crc32 = xcrc32 ((unsigned char const *)FREE_SRAM_AREA,fsize-sizeof(file_trailer), calc_crc32);

	ff_fclose(fp);

	if(calc_crc32!=file_trailer.crc32)
	{
		printf("\nFile: %s\n", filename);
        printf("CRC mismatch, calc: 0x%08x, expect: 0x%08x !!! \n", calc_crc32, file_trailer.crc32);
		*file_size_out=0;
		return 0;
	}

	//BL_PRINTF("\n CRC pass \n");

	*file_size_out=(fsize-sizeof(file_trailer));

	return 1;

}
