/*
 * Copyright (c) 2017, SensiML Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _KB_TYPEDEFS_H_
#define _KB_TYPEDEFS_H_

#include <stdlib.h>
#include <stdint.h>

typedef float FLOAT;
typedef signed short SENSOR_DATA_T;
typedef float FVCOMP_T;
typedef unsigned short NORMTYPE;

#ifdef __cplusplus
extern "C"
{
#endif

    struct compx
    {
        float real;
        float imag;
    };

    struct compx_int16_t
    {
        int16_t real;
        int16_t imag;
    };

    typedef struct
    {
        uint16_t influence; //influence of a pattern
        uint16_t category;  //category of pattern
        uint8_t *vector;    // vector containing the features of a pattern
    } pme_pattern_t;

    typedef struct
    {
        uint16_t number_patterns; //influence of a pattern
        uint16_t pattern_length;  //category of pattern
    } pme_model_header_t;

    /** KB Log levels. */
    enum kb_log_levels
    {
        KB_LOG_LEVEL_1 = 1, /*!< Default log level, classifier output */
        KB_LOG_CLASSIFIER_OUTPUT = KB_LOG_LEVEL_1,
        KB_LOG_LEVEL_2, /*!< A little verbose log level, feature output */
        KB_LOG_FEATURE_OUTPUT = KB_LOG_LEVEL_2,
        KB_LOG_LEVEL_3, /*!< Very Verbose log level, segmenter output */
        KB_LOG_SEGMENTER_OUTPUT = KB_LOG_LEVEL_3,
        KB_LOG_LEVEL_4, /*!< Very Very Verbose log level, kitchen sink included */
        KB_LOG_LEVEL_EVERYTHING = KB_LOG_LEVEL_4,
        KB_LOG_LEVEL_NUM = KB_LOG_LEVEL_4
    };

#ifdef __cplusplus
}
#endif

#endif //_KB_TYPEDEFS_H_
