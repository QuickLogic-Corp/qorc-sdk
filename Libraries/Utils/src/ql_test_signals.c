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

#include "Fw_global_config.h"
#include "ql_test_signals.h"
#include "ql_util.h"

int16_t SineTable_test[240] = {
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271,
  0,6271,11587,15140,16387,15140,11587,6271,  0,-6271,-11587,-15140,-16387,-15140,-11587,-6271
};

#define QN 13
#define QA 12
#define QP 15
/// A sine approximation via a third-order approx.
/// @param x    Angle (with 2^15 units/circle)
/// @return     Sine value (Q12)
int32_t isin_S3(int32_t x)
{
	// S(x) = x * ( (3<<p) - (x*x>>r) ) >> s
	// n : Q-pos for quarter circle             13
	// A : Q-pos for output                     12
	// p : Q-pos for parentheses intermediate   15
	// r = 2n-p                                 11
	// s = A-1-p-n                              17

	static const int qN = QN, qP = QP, qR = 2 * QN - QP, qS = QN + QP + 1 - QA;

	x = x << (30 - qN);          // shift to full s32 range (Q13->Q30)

	if ((x ^ (x << 1)) < 0)     // test for quadrant 1 or 2
		x = (1 << 31) - x;

	x = x >> (30 - qN);

	return x * ((3 << qP) - (x*x >> qR)) >> qS;
}
#if 0
static float ql_cos(float x)
{
	float ql_cos_tab[] = {
	 -0.5f,
	 0.04166666667f,
	 -0.001388888889f,
	 0.0000248015873f
	};
	float y = 1;
	float _XX = x * x;

	y += _XX * (ql_cos_tab[0] + _XX * (ql_cos_tab[1] + _XX * (ql_cos_tab[2] + ql_cos_tab[3] * _XX)));

	return y;
}
static float sin_fast(float x)
{
	float val = 0.0f;
#if 0
	if (x < -3.14159265f)
		x += 6.28318531f;
	else
		if (x > 3.14159265f)
			x -= 6.28318531f;
#endif
	if (x < 0)
	{
		val = x * (1.27323954f + 0.405284735f * x);

		if (val < 0)
			val = val * (-0.255f * (val + 1) + 1);
		else
			val = val * (0.255f * (val - 1) + 1);
	}
	else
	{
		val = x * (1.27323954f - 0.405284735f * x);

		if (val < 0)
			val = val * (-0.255f * (val + 1) + 1);
		else
			val = val * (0.255f * (val - 1) + 1);
	}

	return val;
}
#endif
#define FREQ_START 100
#define PI 3.14159265359f
#define FS 16000

#define PH_D (2.0f * PI *FREQ_START / FS)
#define SCALE   (32)
short chirp(short *p_dest, uint32_t n)
{
	static float ph = 0, ph_d = PH_D;
	static float ph_dd = PH_D / 1000.0f;
	static int limit =5*16000;
	static int limit_cnt = 0;
//	static int sign = 1;

    for(int i = 0; i < n; i++)
	{
		ph += ph_d;
		ph_d += ph_dd;

		if (ph > PI)
		{
			ph -= 2 * PI;
		}
#if 0
		else
		{
			if (ph < -PI)
			{
				ph += 2 * PI;
			}
			else
			{
				printf(".");
			}
		}
#endif
		limit_cnt++;
		if (limit_cnt > limit)
		{
			ph_dd = -ph_dd;
			limit_cnt = 0;
		}
		float val = 0.0f;
		float x = ph;
		float C1 = 1.27323954f*SCALE;
		float C2 = 0.405284735f*SCALE;
		float B1 = 0.255f;
		if (ph < 0)
		{
			val = x * (C1 + C2 * x);

			if (val < 0)
				val = val * (-B1 * (val + SCALE) + SCALE);
			else
				val = val * (B1 * (val - SCALE) + SCALE);
		}
		else
		{
			val = x * (C1 - C2 * x);

			if (val < 0)
				val = val * (-B1 * (val + SCALE) + SCALE);
			else
				val = val * (B1 * (val - SCALE) + SCALE);
		}

		p_dest[i] = (int)val;
     }

	return 0;
}

short tone_lut(short *p_dest, uint32_t n)
{
   // static int idx = 0;
    ql_assert(n = 240);
    for(int i = 0; i < n; i++)
    {
      p_dest[i] = SineTable_test[i];
    }
    return 0;
}
short tone(short *p_dest, uint32_t n)
{
	static float ph = 0, ph_d = PH_D*10;
	static float ph_dd = 0.0f;
	static int limit = 16000;
	static int limit_cnt = 0;
//	static int sign = 1;

    for(int i = 0; i < n; i++)
	{
		ph += ph_d;
		ph_d += ph_dd;

		if (ph > PI)
		{
			ph -= 2 * PI;
		}
#if 0
		else
		{
			if (ph < -PI)
			{
				ph += 2 * PI;
			}
			else
			{
				printf(".");
			}
		}
#endif
		limit_cnt++;
		if (limit_cnt > limit)
		{
			ph_dd = -ph_dd;
			limit_cnt = 0;
		}
		float val = 0.0f;
		float x = ph;
		float C1 = 1.27323954f*SCALE;
		float C2 = 0.405284735f*SCALE;
		float B1 = 0.255f;
		if (ph < 0)
		{
			val = x * (C1 + C2 * x);

			if (val < 0)
				val = val * (-B1 * (val + SCALE) + SCALE);
			else
				val = val * (B1 * (val - SCALE) + SCALE);
		}
		else
		{
			val = x * (C1 - C2 * x);

			if (val < 0)
				val = val * (-B1 * (val + SCALE) + SCALE);
			else
				val = val * (B1 * (val - SCALE) + SCALE);
		}

		p_dest[i] = (int)(val + SineTable_test[i]*0.001);
     }

	return 0;
}
short amp(short *p_dest, uint32_t n)
{
float val = 3.0f;
    for(int i = 0; i < n; i++)
    {
      p_dest[i] *= val;
    }
    return 0;
}
#ifdef TST_QL_MATHOPS
#include <stdio.h>
#include <math.h>
void main(void)
{
	float pi = 3.14159265359f;
	FILE *fp = fopen("t.txt", "w");
	float freq = 100;
	float fs = 16000.0f;
	float ph_d = 2.0f * pi *freq/fs;
	float ph = 0;
	float ph_dd = ph_d / 1000.0f;
	int limit = 16000;
	int limit_cnt = 0;
	int sign = 1;
	short arr[240];

	
	for ( int i = 0; i < 30*16000; i+=240 )
	{
		chirp(arr, 240);

		for(int j = 0; j < 240; j++)
			fprintf(fp, "%f\n", arr[j]*1.0f/(1<<15));
	}
	printf("testing gen\n");
	fclose(fp);
}
#endif  // TST_QL_MATHOPS

