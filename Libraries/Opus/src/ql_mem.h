#ifndef QL_MEM__H
#define QL_MEM__H

#define _QL_OPUS_DEBUG_
#ifdef WIN32
#define QL_OPUS_INTERNAL_MEM_ALLOC   /* for test purpose */
#endif
#ifdef _QL_OPUS_DEBUG_
#include <stdio.h>
#define QL_OPUS_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define QL_OPUS_LOG(fmt, ...)  
#endif

typedef struct 
{
  char *p_buff;
  int  size;
  int  max;
}ql_mem_block_t;

extern ql_mem_block_t o_ql_mem_block1;
extern ql_mem_block_t o_ql_mem_block2;
void* MALLOC(int size);
void FREE2_ql(int size);
void MALLOC2_RESET();
int MALLOC2_MAX(void);

void* MALLOC2(int size);

#endif /* QL_MEM__H */
