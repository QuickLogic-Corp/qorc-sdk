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

/*==========================================================
 *                                                          
 *    File   : c_lib_retarget.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"

#include <common.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <eoss3_hal_uart.h>



int fgetc(FILE * file)
{
#ifdef DISABLE_UART_PRINTS
  return 0;
#else  
  return uart_rx(UART_ID_HW);
#endif  
}

int fputc(int ch, FILE * file)
{
#ifdef DISABLE_UART_PRINTS
  return 0;
#else  
  uart_tx(UART_ID_HW,ch);
  return ch;
#endif 
}


#if (defined( __CELERIES_GCC) || defined( __GNUC__))
#include <sys/stat.h>
#include <sys/unistd.h>
#endif   

//FILE __stdout;
//FILE __stdin;

#if 0
int fgetc(FILE * file)
{
  return uart_rx( UART_ID_CONSOLE );
}

int fputc(int ch, FILE * file)
{
  uart_tx(UART_ID_CONSOLE, ch);
  return ch;
}
#endif

#if (defined( __CELERIES_GCC) || defined( __GNUC__))


int _write(int file, char *ptr, int len)
{
  (void)file;
  uart_tx_buf( UART_ID_CONSOLE, ptr, len );
  return len;
}

int _close(int file)
{
	return -1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _read (int file, char *ptr, int len)
{
	return 0;
}
#endif
