#ifndef FPGA_DESIGN_H
#define FPGA_DESIGN_H

#include <stdint.h>

// extern declarations for the FPGA bitstream and memory initialization arrays and sizes.

extern uint32_t axFPGABitStream[];
extern uint32_t axFPGAMemInit[];
extern uint32_t axFPGABitStream_length;
extern uint32_t axFPGAMemInit_length;

#endif // #ifndef FPGA_DESIGN_H

