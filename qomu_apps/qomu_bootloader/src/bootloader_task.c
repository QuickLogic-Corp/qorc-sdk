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
*    File   : bootloader_task.c 
*    Purpose: This file has task to execute flash Update functionality.
*             
*                                                          
*=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "spi_flash.h"
#include "eoss3_hal_gpio.h"
#include "dbg_uart.h"
#include "flash_defines.h"
#include "bootloader_defines.h"


extern uint32_t image_metadata[FLASH_APP_META_SIZE/4];
extern int load_m4app(void);
extern int load_usb_flasher(void);
extern int load_appfpga(void);
int check_active_images(void);

#define MAX_BOOTLOADER_WAIT_MSEC  (5*1000)
#define MIN_USER_BTN_PRESS_WAIT_MSEC  (200)
#define USER_BUTTON_GPIO_NUM      (0) //Not Used
#define BLUE_LED_GPIO_NUM         (3) //PAD 30, GPIO is connected to Blue LED
#define GREEN_LED_GPIO_NUM        (0) //PAD 6, GPIO is connected to Green LED
#define RED_LED_GPIO_NUM          (0) //PAD 24, GPIO is connected to Red LED


#define METADATA_UNKNOWN_FLASH_STATE        (0x0)
#define METADATA_APPFPGA_IS_FLASHED         (0x1)
#define METADATA_M4APP_IS_FLASHED           (0x2)
#define METADATA_APPFFE_IS_FLASHED          (0x4)
#define METADATA_VALID_IMAGE_IS_FLASHED     (METADATA_APPFPGA_IS_FLASHED|METADATA_M4APP_IS_FLASHED) // ffe not supported yet

TaskHandle_t 	BLTaskHandle;
static int user_button_pressed = 0;
//static int user_btn_on_count = 0;
static int user_btn_on_start = 0;
void check_user_button(void)
{
	  uint32_t mailbox = 0;

	  mailbox = MISC->SW_MB_1;

	  if(mailbox == 0xFEED1BEE)
    {
      user_button_pressed = 1;
    }

  return;
}
/* 
* This will set the red LED state to ON or OFF
*/
void set_boot_error_led(uint8_t value)
{
  //Use the red LED to indicate fatal erro
  HAL_GPIO_Write(RED_LED_GPIO_NUM, value);
}
/* 
* This will set the green LED state to ON or OFF
*/
void set_downloading_led(uint8_t value)
{
  //we use green LED toggle to indicate waiting
  //So, set to constant ON during Flashing using USB  
  //as an Acknowledgement of User button press
  HAL_GPIO_Write(GREEN_LED_GPIO_NUM, value);
}
/* 
* This will toggle green LED with time passed. 
* The state is maintained internal.
* Uses Tickcount to change the ON-OFF states.
*/
void toggle_downloading_led(int high_time_msec, int low_time_msec)
{
  static uint8_t led_state = 1;
  static int led_state_count = 0;
  
  //if the time exceeds the toggle time, change state and note time
  if(led_state == 1 && (xTaskGetTickCount() - led_state_count) > high_time_msec)
  {
    led_state_count = xTaskGetTickCount();
    led_state = 0;
	HAL_GPIO_Write(GREEN_LED_GPIO_NUM, led_state);
  }
  if(led_state == 0 && (xTaskGetTickCount() - led_state_count) > low_time_msec)
  {
    led_state_count = xTaskGetTickCount();
    led_state = 1;
	HAL_GPIO_Write(GREEN_LED_GPIO_NUM, led_state);
  }
  return;
}
void set_waiting_led(uint8_t value)
{
  //we use blue LED toggle to indicate waiting
  //So, set to constant ON during Flashing using USB  
  //as an Acknowledgement of User button press
  HAL_GPIO_Write(BLUE_LED_GPIO_NUM, value);
}
/* 
* This will toggle blue LED with time passed. 
* The state is maintained internal.
* Uses Tickcount to change the ON-OFF states.
*/
void toggle_waiting_led(int toggle_time_msec)
{
  static uint8_t led_state = 1;
  static int led_state_count = 0;
  
  //set the led state first to last state
  HAL_GPIO_Write(BLUE_LED_GPIO_NUM, led_state);
  
  //if the time exceeds the toggle time, change state and note time
  if((xTaskGetTickCount() - led_state_count) > toggle_time_msec)
  {
    led_state_count = xTaskGetTickCount();
    if(led_state == 0)
      led_state = 1;
    else
      led_state = 0;
  }
  return;
}
/* 
* This will toggle red LED with time passed. 
* The state is maintained internal.
* Uses Tickcount to change the ON-OFF states.
*/
void toggle_red_led(int toggle_time_msec)
{
  static uint8_t led_state = 1;
  static int led_state_count = 0;
  
  //set the led state first to last state
  HAL_GPIO_Write(RED_LED_GPIO_NUM, led_state);
  
  //if the time exceeds the toggle time, change state and note time
  if((xTaskGetTickCount() - led_state_count) > toggle_time_msec)
  {
    led_state_count = xTaskGetTickCount();
    if(led_state == 0)
      led_state = 1;
    else
      led_state = 0;
  }
  return;
}

/*
* This BootLoader Task will do 2 things.
* 1. Wait for 5sec for the User Button to be pressed.
*    If pressed, loads USB FPGA image and waits forever
*    for the Reset Button to be pressed
* 2. If the User Button is not pressed, M4 App is loaded.
*    If it fails to load M4 App, waits forever for 
*    User Button to be pressed to re-flash.
*/
static void BLTaskHandler(void *pvParameters)
{
    int wait_time_msec = 0;

	while(1)
	{
      // green led indicates waiting for button press
      toggle_waiting_led(200);
      
      //if User button is pressed load USB FPGA image
      check_user_button();
      if(user_button_pressed)
      {
        //Acknowledge User Button press
        dbg_str("User button pressed: switch to download mode\n");
        set_waiting_led(0);
        set_downloading_led(1);
        int error = load_usb_flasher(); //this should never return
        //if can not load USB FPGA image, it is fatal error. wait indefinitely
        while(1)
        {
          //set red LED for error and turn off green LED
          set_boot_error_led(1);
          set_downloading_led(0);
          dbg_str("ERROR loading USB FPGA Image. Please re-flash USB FPGA Image .. \n");
          dbg_str("Press Reset, then User Button and start Flash script .. \n\n");
          vTaskDelay(5*1000);
        }
      }
      //wait for a maximum of 3 secs before loading M4 App
      vTaskDelay(1);
      wait_time_msec++;
      if(wait_time_msec > MAX_BOOTLOADER_WAIT_MSEC)
      {
        dbg_str("User button not pressed: proceeding to load application\n");
        set_waiting_led(0);

        // check the image_info of appfpga, m4app and load the images accordingly
        uint8_t current_active_images = check_active_images();

        if((current_active_images & METADATA_VALID_IMAGE_IS_FLASHED) == 0)
        {
            while(1)
            {
                //set red LED for error and turn off green LED
                set_boot_error_led(1);
                set_downloading_led(0);
                dbg_str("ERROR: No Valid Image Found To Load! Waiting for re-flashing .. \n");
                dbg_str("Press Reset then User Button and start Flash script .. \n\n");
                vTaskDelay(5*1000);
            }
        }

        if(current_active_images & METADATA_APPFPGA_IS_FLASHED)
        {
            dbg_str("Loading Application FPGA...\r\n");
            int error = load_appfpga();

            if(error != BL_NO_ERROR) // error occurred?
            {
                while(1)
                {
                    //set red LED for error and turn off green LED
                    set_boot_error_led(1);
                    set_downloading_led(0);
                    dbg_str("ERROR loading App FPGA. Waiting for re-flashing .. \n");
                    dbg_str("Press Reset then User Button and start Flash script .. \n\n");
                    vTaskDelay(5*1000);
                }
            }
        }

        // ffe loader in the future

        if(current_active_images & METADATA_M4APP_IS_FLASHED)
        {
            dbg_str("Loading M4 Application...\r\n");
            int error = load_m4app(); //this should never return
            //if the M4 image is corrupted it needs to be re-flashed. wait indefinitely
            while(1)
            {
                //set red LED for error and turn off green LED
                set_boot_error_led(1);
                set_downloading_led(0);
                dbg_str("ERROR loading M4 APP. Waiting for re-flashing .. \n");
                dbg_str("Press Reset then User Button and start Flash script .. \n\n");
                vTaskDelay(5*1000);
            }
        }
        
        // if we reach here, then probably the m4app image is not active, we sit tight.
        while(1);        
      }
	}
}

int check_active_images()
{
    uint8_t active_images = 0x0;
    unsigned char* bufPtr;
    uint32_t image_crc;
    uint32_t image_size;
    uint32_t image_info;
    uint8_t* image_info_ptr8;

    // check the image_info metadata and see which images are marked as active
    // return a value indicating this
    // TODO do we want to do a better image integrity check here?
    // perhaps we can move the crc check here itself, rather than in the loaders?
    // for now, I think our organization is ok.

    // read the fpga metadata
    bufPtr = (unsigned char *)image_metadata; 
    read_flash((unsigned char *)FLASH_APPFPGA_META_ADDRESS, FLASH_APPFPGA_META_SIZE, bufPtr);
    image_crc = image_metadata[0];
    image_size = image_metadata[1];
    image_info = image_metadata[2];
    image_info_ptr8 = (uint8_t*)(&image_info);
    //dbg_str("fpga: 0x");dbg_hex8(image_info_ptr8[0]);dbg_str("\r\n");
    if(image_info_ptr8[0] == 0x03)
    {
        if(image_size == 0xFFFFFFFF || image_crc == 0xFFFFFFFF)
        {
            // error, image is marked active, but doesn't seem to have image flashed!
            dbg_str("warning: appfpga image marked active, but seems to have invalid image\r\n");
        }
        else
        {
            // ok, active image, and looks to have a proper image
            active_images |= METADATA_APPFPGA_IS_FLASHED;
        }
        
    }

    // read the ffe metadata
    bufPtr = (unsigned char *)image_metadata; 
    read_flash((unsigned char *)FLASH_APPFFE_META_ADDRESS, FLASH_APPFFE_META_SIZE, bufPtr);
    image_crc = image_metadata[0];
    image_size = image_metadata[1];
    image_info = image_metadata[2];
    image_info_ptr8 = (uint8_t*)(&image_info);
    //dbg_str("ffe: 0x");dbg_hex8(image_info_ptr8[0]);dbg_str("\r\n");
    if(image_info_ptr8[0] == 0x03)
    {
        if(image_size == 0xFFFFFFFF || image_crc == 0xFFFFFFFF)
        {
            // error, image is marked active, but doesn't seem to have image flashed!
            dbg_str("warning: appffe image marked active, but seems to have invalid image\r\n");
        }
        else
        {
            // ok, active image, and looks to have a proper image
            active_images |= METADATA_APPFFE_IS_FLASHED;
        }
        
    }

    // read the m4 metadata
    bufPtr = (unsigned char *)image_metadata; 
    read_flash((unsigned char *)FLASH_APP_META_ADDRESS, FLASH_APP_META_SIZE, bufPtr);
    image_crc = image_metadata[0];
    image_size = image_metadata[1];
    image_info = image_metadata[2];
    image_info_ptr8 = (uint8_t*)(&image_info);
    //dbg_str("m4: 0x");dbg_hex8(image_info_ptr8[0]);dbg_str("\r\n");
    if(image_info_ptr8[0] == 0x03)
    {
        if(image_size == 0xFFFFFFFF || image_crc == 0xFFFFFFFF)
        {
            // error, image is marked active, but doesn't seem to have image flashed!
            dbg_str("warning: m4app image marked active, but seems to have invalid image\r\n");
        }
        else
        {
            // ok, active image, and looks to have a proper image
            active_images |= METADATA_M4APP_IS_FLASHED;
        }
        
    }

    return active_images;
}

/*!
* \fn void BL_Task_Init()
* \brief  Init function to create BootloaderTask to be called from main()
*
*/
void BL_Task_Init(void)
{
	xTaskCreate(BLTaskHandler, "BL_Task", 256, NULL, tskIDLE_PRIORITY + 4, &BLTaskHandle);
	configASSERT(BLTaskHandle);
	return;
}
