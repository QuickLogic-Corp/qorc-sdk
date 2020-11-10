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
 *    File   : ql_base64.c
 *    Purpose: Utility to encode and decode bytes into base64 format
 *                                                          
 *=========================================================*/

static const char base64EncodeTable[64] = {
    //"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" 
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', // 26
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', // +26 = 52
    '0','1','2','3','4','5','6','7','8','9','+','/' // +12 = 64
};
/*
* This module expects 3 byte input buffer and stores 4 byte output.
* The return value is the number of output bytes.
*/
int base64Encode3(char *inBuf, char *outBuf)
{
    //char temp1,temp2;
    int i;
    int packed = *inBuf++; //1 byte
    packed = (packed << 8) | ((*inBuf++) & 0xFF); //2 bytes
    packed = (packed << 8) | ((*inBuf++) & 0xFF); //3 bytes
    i = (packed >> (24 - 6)) & 0x3F;
    *outBuf++= base64EncodeTable[i];
    i = (packed >> (24 - 12)) & 0x3F;
    *outBuf++= base64EncodeTable[i];
    i = (packed >> (24 - 18)) & 0x3F;
    *outBuf++= base64EncodeTable[i];
    i = (packed) & 0x3F;
    *outBuf++= base64EncodeTable[i];
    
    return 4; //always 4 bytes output
}
/*
* This module encodes the input byte array into base64 byte array. 
* The return value is the number of output bytes.
* Output will be always 4/3 times input.
* If input is not exact multiple of 3, '=' chars are appended.
*/
int base64Encode(char *inBuf, int count, char *outBuf)
{
    int k;
    char temp[3];
    
    k = 0;
    while(count >= 3)
    {
        count -= 3;
        k += base64Encode3(inBuf,outBuf+k);
        inBuf += 3;
    }
    
    //there could be 0,1 or 2 chars left
    if(count > 0)
    {
       temp[0]= *inBuf++;
       if(count > 1)
          temp[1]= *inBuf++;
       else
          temp[1]= '='; //empty chars are always '='
       temp[2]= '='; //always a '=' filler
        k += base64Encode3(temp,outBuf+k);
    }
    
    return k;
}
/*
* Decode each base64 char into a 6 bit value from 0-63
*/
static char getBase64Index(char t)
{
  if((t >= 'A') && (t <= 'Z'))
     return t - 'A';
  if((t >= 'a') && (t <= 'z'))
     return t - 'a' + 26;
      
  if((t >= '0') && (t <= '9'))
     return t - '0' + 52;
  if(t == '+')
     return t - '+' + 62;
  if(t == '/')
     return t - '/' + 63;
  
  printf("BASE64 -Err-");
  
  return 0;
}
/*
* This module decodes the input base64 byte array into byte array. 
* The return value is the number if output bytes.
* Output will be always 3/4 times input.
* Any remaining bytes of 1, 2, or 3 will be ignored.
*/
int base64Decode(char *inBuf, int count, char *outBuf)
{
    int i,k;
    k = 0;
    while(count >= 4)
    {
        count -= 4;
        i = getBase64Index(*inBuf++); //1 byte;
        i = (i << 6) | getBase64Index(*inBuf++); //2 bytes
        i = (i << 6) | getBase64Index(*inBuf++); //3 bytes
        i = (i << 6) | getBase64Index(*inBuf++); //4 bytes
        *outBuf++ = (i >> 16) & 0xFF;
        *outBuf++ = (i >> 8) & 0xFF;
        *outBuf++ = (i >> 0) & 0xFF;
        k += 3;
    }
    
    //There could be 0,1,2 or 3 chars left. Ignore them since do not know how to decode.
    
    return k;
}

/*
* This module encodes the input byte array into base64 
* byte array and appends a new line.
* The return value is the number of output bytes.
* Output will be always 4/3 times input + 1 bytes.
* If input is not exact multiple of 3, it is an error .
*/
int base64EncodeLine(char *inBuf, int count, char *outBuf)
{
    int k;
    
    //make sure input is multiple of 3
    k = count/3;
    if(count != (k*3))
      return 0;
    
    k = 0;
    while(count >= 3)
    {
        count -= 3;
        k += base64Encode3(inBuf,outBuf+k);
        inBuf += 3;
    }
    outBuf[k++] = '\n';
    
    return k;
}
