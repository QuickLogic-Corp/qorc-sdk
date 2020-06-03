/*==========================================================
 * Copyright 2020 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

#include "Fw_global_config.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "eoss3_dev.h"
#include "eoss3_hal_spi.h"
#include "spi_flash.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* FreeRTOS+FAT headers. */
#include "ff_headers.h"
#include "ff_stdio.h"

#include "mmc_s3.h"
#include "media_drv_spi_sd.h"
#include "ql_fs.h"


/* The number of bytes read/written to the example files at a time. */
#define fsRAM_BUFFER_SIZE 				4096

/* The number of bytes written to the file that uses f_putc() and f_getc(). */
#define fsPUTC_FILE_SIZE				100


/* The SD card is mounted in the root of the file system. */
#define mainHAS_SDCARD					1
#define mainSD_CARD_DISK_NAME			       "/"

static uint8_t pcRAMBuffer[fsRAM_BUFFER_SIZE];

const static BaseType_t xMaxFiles = 2;
//

/*
 * Create a set of example files in the root directory of the volume using
 * ff_fwrite().
 */
static void prvCreateDemoFilesUsing_ff_fwrite( const char *pcMountPath );




/*
 * Use ff_fread() to read back and verify the files that were created by
 * prvCreateDemoFilesUsing_ff_fwrite().
 */
static void prvVerifyDemoFileUsing_ff_fread( void );

static void vCreateAndVerifyExampleFiles( const char *pcMountPath );

void prvCreateExampleFiles(FF_Disk_t *pxDisk) 
{
    FF_PRINTF(( "Mount SD-card\n" ));

    if (pxDisk != NULL) 
    {
		/* Remove the base directory again, ready for another boot. */
		//ff_deltree( mainSD_CARD_TESTING_DIRECTORY);

		/* Make sure that the testing directory exists. */
		//ff_mkdir( mainSD_CARD_TESTING_DIRECTORY);

		/* Create a few example files on the disk.  These are not deleted again. */
		vCreateAndVerifyExampleFiles( "/SPISD");
    }
}

void vCreateAndVerifyExampleFiles( const char *pcMountPath )
{
	/* Create and verify a few example files using both line based and character
	based reads and writes. */
	prvCreateDemoFilesUsing_ff_fwrite( pcMountPath );
	prvVerifyDemoFileUsing_ff_fread();
}
/*-----------------------------------------------------------*/

/*!
 *\fn static int file_IsPresent(const char * file_path)
 *\param file_path -- path to the file
 *\brief Returns 1 if file is present, 0 if not 
*/
static int file_IsPresent(const char * file_path)
{
    FF_FILE *fp;
    int ret =0;
    fp = ff_fopen(file_path, "r");
    
    if(fp == NULL)
    {
        //if(pdFREERTOS_ERRNO_NONE == ff_errno())
        printf("-------Error:: Opening file %s", file_path);
        ret = 0;
    }
    else
        ret = 1;
    
    ff_fclose(fp);
    
    return ret;
}

static void prvCreateDemoFilesUsing_ff_fwrite( const char *pcMountPath )
{
    BaseType_t xFileNumber;
    int32_t lItemsWritten;
    FF_FILE *pxFile = NULL;
    char *pcRAMBufferT, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBufferT = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBuffer );
	configASSERT( pcFileName );

	/* Ensure in the root of the mount being used. */
	//lResult = ff_chdir( pcMountPath );
	//configASSERT( lResult >= 0 );
        
    char file_path[40] = "/SPISD/";
    
    /* Generate a file name. */
    snprintf( pcFileName, ffconfigMAX_FILENAME, "root%03d.txt", 1 );
#if 1   
    //strcat(SPISD_DISK_NAME, pcFileName);
    strcat(file_path, pcFileName);
    
    // Just for testing added
    
    // just for testing ends
    
    if(!file_IsPresent(file_path))
    {
        /* Open the file, creating the file if it does not already exist. */
        pxFile = ff_fopen(file_path, "w");
	    configASSERT( pxFile );
        ff_fclose(pxFile);
    }
#else
    if(!file_IsPresent(pcFileName))
    {
        pxFile = ff_fopen(pcFileName, "w");
	    configASSERT( pxFile );
        ff_fclose(pxFile);
    }
#endif
        
    // First fill up the buffer with known number
    for(int cnt = 0; cnt<fsRAM_BUFFER_SIZE; cnt++)
	{
		pcRAMBuffer[cnt] = cnt;
	}

	/* Create xMaxFiles files.  Each created file will be
	( xFileNumber * fsRAM_BUFFER_SIZE ) bytes in length, and filled
	with a different repeating character. */
	for( xFileNumber = 0; xFileNumber <= xMaxFiles; xFileNumber++ )
	{
		/* Generate a file name. */
		snprintf( pcFileName, ffconfigMAX_FILENAME, "root%03d.txt", ( int ) xFileNumber );

		/* Obtain the current working directory and print out the file name and
		the	directory into which the file is being written. */
		//ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
		FF_PRINTF( "Creating file %s in %s\n", pcFileName, pcRAMBuffer );
                
                memset(file_path, 0, sizeof(file_path));
                strcat(file_path,"/SPISD/");
                strcat(file_path, pcFileName);

		/* Open the file, creating the file if it does not already exist. */
		pxFile = ff_fopen( file_path, "w" );
		configASSERT( pxFile );

		/* Fill the RAM buffer with data that will be written to the file.  This
		is just a repeating ascii character that indicates the file number. */
//		memset( pcRAMBuffer, ( int ) ( '0' + xFileNumber ), fsRAM_BUFFER_SIZE );

		/* Write the RAM buffer to the opened file a number of times.  The
		number of times the RAM buffer is written to the file depends on the
		file number, so the length of each created file will be different. */
//		for( xWriteNumber = 0; xWriteNumber < 8192; xWriteNumber++ )
//		{
//			lItemsWritten = ff_fwrite( pcRAMBuffer, fsRAM_BUFFER_SIZE, 1, pxFile );
//			configASSERT( lItemsWritten == 1 );
//		}
        
        // Write buffer to the file
        lItemsWritten = ff_fwrite( pcRAMBuffer, 1, fsRAM_BUFFER_SIZE, pxFile );
		configASSERT( lItemsWritten == fsRAM_BUFFER_SIZE );

		/* Close the file so another file can be created. */
		ff_fclose( pxFile );
	}

	vPortFree( pcRAMBufferT );
	vPortFree( pcFileName );
}
/*-----------------------------------------------------------*/

static void prvVerifyDemoFileUsing_ff_fread( void )
{
BaseType_t xFileNumber, xReadNumber;
static size_t xItemsRead;
FF_FILE *pxFile;
char *pcRAMBufferR, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBufferR = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBufferR );
	configASSERT( pcFileName );
        
        char file_path[40] = "/SPISD/";

	/* Read back the files that were created by
	prvCreateDemoFilesUsing_ff_fwrite(). */
	for( xFileNumber = 0; xFileNumber <= xMaxFiles; xFileNumber++ )
	{
		/* Generate the file name. */
		snprintf( pcFileName, ffconfigMAX_FILENAME, "root%03d.txt", ( int ) xFileNumber );

		/* Obtain the current working directory and print out the file name and
		the	directory from which the file is being read. */
		//ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
		//FF_PRINTF( "Reading file %s from %s\n", pcFileName, pcRAMBuffer );
#if 1
         memset(file_path, 0, sizeof(file_path));
         strcat(file_path,"/SPISD/");
         strcat(file_path, pcFileName);

		 /* Open the file for reading. */
		 pxFile = ff_fopen( file_path, "r" );
#else
         		 /* Open the file for reading. */
		 pxFile = ff_fopen(pcFileName, "r");
#endif
		 configASSERT( pxFile );

		/* Read the file into the RAM buffer, checking the file contents are as
		expected.  The size of the file depends on the file number. */
		//for( xReadNumber = 0; xReadNumber < xFileNumber; xReadNumber++ )
        for( xReadNumber = 0; xReadNumber < 1; xReadNumber++ )
		{
			/* Start with the RAM buffer clear. */
			memset( pcRAMBuffer, 0x00, fsRAM_BUFFER_SIZE );

            //fsRAM_BUFFER_SIZE = 512;
			//xItemsRead = ff_fread( pcRAMBuffer, 1, fsRAM_BUFFER_SIZE, pxFile );
            xItemsRead = ff_fread( pcRAMBuffer, 1, 512, pxFile );
			configASSERT( xItemsRead == 512 );

			/* Check the RAM buffer is filled with the expected data.  Each
			file contains a different repeating ascii character that indicates
			the number of the file. */
            static int cnt = 0;
			//for( int cnt = 0; cnt < fsRAM_BUFFER_SIZE; cnt++ )
            for(; cnt < 512; cnt++ )
			{
				//configASSERT( pcRAMBuffer[ xChar ] == ( '0' + ( char ) xFileNumber ) );
                if(pcRAMBuffer[cnt] != (UINT8_t)cnt)
                {
                    configASSERT(0);
                }
			}
		}

		/* Close the file. */
		ff_fclose( pxFile );
	}

	vPortFree( pcRAMBufferR );
	vPortFree( pcFileName );

	/*_RB_ also test what happens when attempting to read using too large item
	sizes, etc. */
}