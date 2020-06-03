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

// StateMachineSimulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GlobalStateMachine.h"

int main(int kargs, char** apzArgs)
{
	char*	pzFileNameStateMachine = NULL;
	char*	pzFileNameDeviceDescription = NULL;
	char*	pzFileNameEventList = NULL;
	char*	pzDirName = (char*)"";

	int iarg;
	for (iarg = 1; iarg != kargs; iarg++) {
		if (iarg == kargs - 1) {
			fprintf(stderr, "No argument to last switch\n");
			Usage();
			exit(0);
		}
		if (strcmp(apzArgs[iarg], "--state-machine") == 0) {
			pzFileNameStateMachine = apzArgs[++iarg];
		} else if (strcmp(apzArgs[iarg], "--device-description") == 0) {
			pzFileNameDeviceDescription = apzArgs[++iarg];
		} else if (strcmp(apzArgs[iarg], "--simulation-script") == 0) {
			pzFileNameEventList = apzArgs[++iarg];
		} else if (strcmp(apzArgs[iarg], "--target-dir") == 0) {
			pzDirName = apzArgs[++iarg];
		} else {
			fprintf(stderr, "Unknown argument: '%s'\n", apzArgs[iarg]);
			Usage();
			exit(0);
		}
	}
	GenerateFSM(pzFileNameStateMachine, pzFileNameDeviceDescription, pzFileNameEventList, pzDirName );
	exit(0);
}

void	Usage(void) {
	fprintf(stderr, "Usage: StateMachineSimulator --state-machine file.csv\n");
	fprintf(stderr, "                             --device-description file.txt\n");
	fprintf(stderr, "                             --simulation-script file.txt\n");
	fprintf(stderr, "                             --target-dir dir\n");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tipz for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
