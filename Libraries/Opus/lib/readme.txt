QL Opus Library Version: 1.1.0.a
================================
Opus Encoder Library updated to support 20mSec Frame size. 
Following are the changes required to integrate/replace the 10mSec library:
1. increase the SZ_QL_MEM_BLOCK2 to 27*1024 from 16*1024 
2. Pass E_QL_OPUS_ENCODER_OPTION4 to the ql_opus_init() function
3. pass the 320 as the thrid argument to ql_opus_encode(). Also makesure that there are equal number of samples in the p_pcm_samples buffer
4. makesure that the output buffer has atleast 88 bytes space.

Increase in :
 a. code size : 37.8k Bytes from  32.9k
 b. table memory : minor change 
 c. state memory : minor change
 d. scratch memory :  27k from 16kBytes

Test status : Only basic testing performed. Only intergration activitiy. 

Date : 18-09-2018
 

QL Opus Library Version: 1.1.0.b
================================
Opus Encoder Library updated to support 20mSec Frame size. 
Fixed range issue.
Procedure is same as that for previous library version  1.1.0.a.

Date : 19-09-2018

QL Opus Library Version: 1.2.0.a
================================
Opus Encoder Library updated to support complexity 4. Built with 20ms frame size.
Procedure is same as that for previous library version  1.1.0.b.

Date : 14-11-2018

QL Opus Library Version: 1.2.1.a
================================
Opus Encoder Library updated to support complexity 4. Built with 20ms frame size.
Procedure is same as that for previous library version  1.1.0.b.
Only basic testing performed.

Date : 16-11-2018

QL Opus Library Version: 1.2.1.0 (RC)
================================
Opus Encoder Library updated to support complexity 4. Built with 20ms frame size.
Procedure is same as that for previous library version  1.1.1.a.
Testing performed with complete set of vectors - SQAM and AMA vector sets.

MIPS = 24.95 measured with test application usind cycle count for worst case frame. 
Opus Code  = 39,852 Bytes
Opus Table = 10,500 Bytes
Opus RAM(state)    =  9216 Bytes
Opus RAM(scrach)   = 27648 Bytes
Stack Size  = measured to be 2K with standalone test app. All tests are conducted with 8K stack.  


Date : 21-11-2018