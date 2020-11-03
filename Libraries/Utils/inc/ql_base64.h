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
 *    File   : ql_base64.h
 *    Purpose: Utility to encode and decode bytes into base64 format
 *                                                          
 *=========================================================*/
/*
* This module expects 3 byte input buffer and stores 4 byte output.
* The return value is the number of output bytes.
*/
extern int base64Encode3(char *inBuf, char *outBuf);
/*
* This module encodes the input byte array into base64 byte array. 
* The return value is the number of output bytes.
* Output will be always 4/3 times input.
* If input is not exact multiple of 3, '=' chars are appended.
*/
extern int base64Encode(char *inBuf, int count, char *outBuf);
/*
* This module decodes the input base64 byte array into byte array. 
* The return value is the number if output bytes.
* Output will be always 3/4 times input.
* Any remaining bytes of 1, 2, or 3 will be ignored.
*/
extern int base64Decode(char *inBuf, int count, char *outBuf);
/*
* This module encodes the input byte array into base64 
* byte array and appends a new line.
* The return value is the number of output bytes.
* Output will be always 4/3 times input + 1 bytes.
* If input is not exact multiple of 3, it is an error .
*/
extern int base64EncodeLine(char *inBuf, int count, char *outBuf);
