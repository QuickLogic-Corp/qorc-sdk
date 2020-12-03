#include "ql_mem.h"
#ifdef QL_OPUS_INTERNAL_MEM_ALLOC
#define SZ_QL_MEM_BLOCK1  (24*1024)
#ifndef WIN32
 _Pragma("data_alignment=8")
#endif
char a_ql_mem_block_buff1[SZ_QL_MEM_BLOCK1];

#define SZ_QL_MEM_BLOCK2  (18*1024)
#ifndef WIN32
_Pragma("data_alignment=8")
#endif
char a_ql_mem_block_buff2[SZ_QL_MEM_BLOCK2];

#endif

#ifdef QL_OPUS_INTERNAL_MEM_ALLOC
ql_mem_block_t o_ql_mem_block1 =
{
	.p_buff = &a_ql_mem_block_buff1[0],
	.size = SZ_QL_MEM_BLOCK1,
	.max = SZ_QL_MEM_BLOCK1

};
ql_mem_block_t o_ql_mem_block2 =
{
	.p_buff = &a_ql_mem_block_buff2[0],
	.size = SZ_QL_MEM_BLOCK2,
	.max = SZ_QL_MEM_BLOCK2

};
#else
ql_mem_block_t o_ql_mem_block1 =
{
	.p_buff = NULL,
	.size = 0,
	.max = 0

};
ql_mem_block_t o_ql_mem_block2 =
{
	.p_buff = NULL,
	.size = 0,
	.max = 0

};
#endif

void ql_opus_set_mem(char*p1, char *p2, int size1, int size2)
{
	o_ql_mem_block1.p_buff = p1;
	o_ql_mem_block1.max = size1;
	o_ql_mem_block1.size = size1;

	o_ql_mem_block2.p_buff = p2;
	o_ql_mem_block2.max = size2;
	o_ql_mem_block2.size = size2;
}

void* MALLOC(int size)
{
  void* p = 0;
    size = (size+7)/8*8;
    if(size < o_ql_mem_block1.size)
    {
      p = &o_ql_mem_block1.p_buff[o_ql_mem_block1.max-o_ql_mem_block1.size];
      o_ql_mem_block1.size -= size; 
    }
    else
    {
      printf("MALLOC failed %d", size);
    }
    
    return p;
}
static int g_level = 0;
static int g_size2_min = 10000000;

void* MALLOC2(int size)
{
  void* p = 0;
    size = (size+7)/8*8;
    if(size < o_ql_mem_block2.size)
    {
      p = &o_ql_mem_block2.p_buff[o_ql_mem_block2.max-o_ql_mem_block2.size];
      o_ql_mem_block2.size -= size; 
	  g_level++;
    }
    else
    {
      QL_OPUS_LOG("MALLOC2 failed %d %d", size, o_ql_mem_block2.size);
    }
    
    return p;
}
void FREE2_(int size)
{
	size = (size + 7) / 8 * 8;
	g_level--; //o_ql_mem_block2.size += size;
}
void FREE2_ql(int size)
{
	size = (size + 7) / 8 * 8;
	g_level--; 
	o_ql_mem_block2.size += size;
}
void* alloca (int n)
{
  return MALLOC2(n);
}
int g_mallog_frame = 0;
void MALLOC2_RESET()
{
	if (g_size2_min > o_ql_mem_block2.size) {
		g_size2_min = o_ql_mem_block2.size;
	//	printf("size = %d, %d @%d\n", o_ql_mem_block2.size, g_size2_min, g_mallog_frame);
	}

	o_ql_mem_block2.size = o_ql_mem_block2.max; // reset scratch 
	g_mallog_frame++;
}

int MALLOC2_MAX(void)
{
	return  g_size2_min;
}