/*==========================================================
 * *
 *    File   :
 *    Purpose: This file is a defines for custom commands of FS
*    Author : QuickLogic
 *
 *=========================================================*/

#include "ff_headers.h"
#include "ff_stdio.h"

void DIRCommand( const char *pcDirectoryToScan )
{
	FF_FindData_t *pxFindStruct;
	const char  *pcAttrib,*pcWritableFile = "writable file",*pcReadOnlyFile = "read only file",*pcDirectory = "directory";

	printf("\nListing directory '%s' contents",pcDirectoryToScan);
	printf("\n=============================================================");

	/* FF_FindData_t can be large, so it is best to allocate the structure
	dynamically, rather than declare it as a stack variable. */
	pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );

	/* FF_FindData_t must be cleared to 0. */
	memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );

	/* The first parameter to ff_findfist() is the directory being searched.  Do
	not add wildcards to the end of the directory name. */
	if( ff_findfirst( pcDirectoryToScan, pxFindStruct ) == 0 )
	{
		do
		{
			/* Point pcAttrib to a string that describes the file. */
			if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
			{
				pcAttrib = pcDirectory;
			}
			else if( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
			{
				pcAttrib = pcReadOnlyFile;
			}
			else
			{
				pcAttrib = pcWritableFile;
			}

			/* Print the files name, size, and attribute string. */
			printf ( "\n%-16s\t[%s]\t[size=%d]", pxFindStruct->pcFileName,
					   pcAttrib,
					   pxFindStruct->ulFileSize);

		} while( ff_findnext( pxFindStruct ) == 0 );
	}

	/* Free the allocated FF_FindData_t structure. */
	vPortFree( pxFindStruct );

	printf("\n=============================================================");
}
