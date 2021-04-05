#include "model_json.h"
/* The contents of model.json are not formatted as a C string and so can not be
 * easily included in C source files.
 * This source file uses the assembly directive .incbin to include contents of
 * model.json file as a C data array.
 */
__asm__ (".global recognition_model_string_json \n\t \
		  .global recognition_model_string_json_len \n\t \
		  recognition_model_string_json:        \n\t \
		       .incbin \"model.json\"           \n\t \
		  recognition_model_string_json_end:    \n\t \
		      .byte 0                           \n\t \
		  recognition_model_string_json_len:    \n\t \
		      .int recognition_model_string_json_end - recognition_model_string_json  \n\t \
        ");
