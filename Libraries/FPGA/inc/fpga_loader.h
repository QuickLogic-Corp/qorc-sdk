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
*    File   : fpga_loader.h
*    Purpose: Contains functionality to load FPGA
*                                                          
*=========================================================*/

// Load FPGA bitstream
int load_fpga(uint32_t img_size,uint32_t* image_ptr);

// Load FPGA bitstream and initialize memory blocks
int load_fpga_with_mem_init(uint32_t image_size, uint32_t* image_ptr, uint32_t mem_content_size, uint32_t* mem_content_ptr);

// initialize the FPGA iomux - put the IOs used in the FPGA design in the FPGA's control
int fpga_iomux_init(uint32_t iomux_size, uint32_t* iomux_ptr);

