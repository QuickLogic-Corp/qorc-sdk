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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GlobalStateMachine.h"

struct GlobalStateMachine fsm;
struct fsmrow	currentState;
int				irowCurrentState;
struct process	aprocess[kprocess];
struct interrupt ainterrupt[kprocess];

char	abuf[1000];
int		krowsActual;
int		kprocessActual;

struct StringList*	pstringlistFSMStates = NULL;
struct StringList*	pstringlistControlEvents = NULL;
struct StringList*	pstringlistActions = NULL;
struct StringList*	pstringlistProcesses = NULL;
struct StringList*	pstringlistProcessesUsed = NULL;
struct StringList*	pstringlistProcessStates = NULL;

int		g_kerror = 0;	// Error count

char acFSMC[200];
char acFSMH[200];
char acCLI[200];

void	GenerateFSM(const char* pzFileNameFSM, const char *pzFileNameProcessCharacteristics, const char* pzFileNameSimulationScript, const char* pzDirName) {
	printf("Loading Process Characteristics from '%s'...\n", pzFileNameProcessCharacteristics);
	LoadProcessCharacteristics(pzFileNameProcessCharacteristics);
	//PrintProcessNames();
	PrintEventNames();
	PrintActionNames();
	PrintProcessStateNames();

	printf("Loading FSM from '%s' ...\n", pzFileNameFSM);
	LoadFSM(pzFileNameFSM);
	if (g_kerror > 0) {
		fprintf(stderr, "Stopping due to errors\n");
		exit(1);
	}
	SetCurrentState();
	PrintFSMStateNames();
	PrintProcessStateNames();

	strcpy(acFSMC, pzDirName);
	if (acFSMC[0] != '\0')
		strcat(acFSMC, "\\");
	strcat(acFSMC, "fsm.c");

	strcpy(acFSMH, pzDirName);
	if (acFSMH[0] != '\0')
		strcat(acFSMH, "\\");
	strcat(acFSMH, "fsm.h");

	strcpy(acCLI, pzDirName);
	if (acCLI[0] != '\0')
		strcat(acCLI, "\\");
	strcat(acCLI, "fsm_cli_table.c");


	printf("Writing FSM to %s and %s\n", acFSMC, acFSMH);
	WriteFSM(pzFileNameFSM, acFSMC, acFSMH);
	WriteFSM_cli_table(acCLI);

	PrintFSM();
	PrintProcessDescriptions();
	PrintInterruptDescriptions();
	printf("Starting simulation...\n");
	SimulateEvents(pzFileNameSimulationScript);
}

// Load Process Characteristics
void	LoadProcessCharacteristics(const char* pzFileName) {
	if (pzFileName == NULL) {
		fprintf(stderr, "Error: no DeviceDescription file specified\n");
		exit(0);
	}
	FILE* pfile;
	pfile = fopen(pzFileName, "rt");
	if (!pfile) {
		fprintf(stderr, "Could not open '%s'\n", pzFileName);
		exit(0);
	}

	// Load actions and events data structures
	char	acProcess[100];
	char	acAction[100];
	char	acState[100];
	char	acEvent[100];
	int		iprocess = -1;
	int		iaction = 0;
	int		iinterrupt = 0;
	bool	fIsBlank;
	int		iline = 0;

	while (fgets(abuf, sizeof(abuf), pfile)) {
		iline++;
		fIsBlank = true;
		for (int i = 0; abuf[i] != '\0'; i++) {
			if (abuf[i] != '\r' && abuf[i] != '\n' && abuf[i] != ' ' && abuf[i] != '\t') {
				fIsBlank = false;
				break;
			}
		}
		if (fIsBlank) continue;					// Ignbore blank line
		if (strncmp(abuf, "//", 2) == 0) continue;	// Ignore comment

		if (sscanf(abuf, "Process: %[A-Za-z0-9_]", acProcess) == 1) {	// New process
			iprocess++;
			iaction = 0;
			iinterrupt = 0;
			aprocess[iprocess].psName = psStrDup(acProcess);
			RecordProcesses(acProcess);
		} else if (sscanf(abuf, "Action: %[A-Za-z0-9_] => %[A-Za-z0-9_]", acAction,  acState) == 2) { // Action
			aprocess[iprocess].aactionPlusNextState[iaction].psAction = psStrDup(acAction);
			RecordActions(acAction);
			aprocess[iprocess].aactionPlusNextState[iaction].psNextState = psStrDup(acState);
			RecordProcessStates(acState);
			iaction++;
			if (iaction == kactions) {
				fprintf(stderr, "Too many actions -- need to increase kactions (at line %d)\n", iline);
				exit(1);
			}
		} else if (sscanf(abuf, "Event: %[A-Za-z0-9_]", acEvent) == 1) {
			ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psEvent = psStrDup(acEvent);
			RecordEvents(acEvent);
			iinterrupt++;
		} else if (sscanf(abuf, "State: %[A-Za-z0-9_]", acState) == 1) {
			aprocess[iprocess].aactionPlusNextState[iaction].psNextState = psStrDup(acState);
			RecordProcessStates(acState);
			iaction++;
			if (iaction == kactions) {
				fprintf(stderr, "Too many actions -- need to increase kactions (at line %d)\n", iline);
				exit(1);
			}
		}
		else {
			fprintf(stderr, "Unknown process characteristic: '%s'\n", abuf);
			fprintf(stderr, "%d %d\n", (int)abuf[0], (int)abuf[1]);
		}
	}
	// Add some predefined actions and events
	RecordActions("NULL");
	RecordEvents("START");
	RecordEvents("CONFIG");
	RecordProcessStates("DONT_CARE");
	RecordProcessStates("UNKNOWN");
	RecordProcessStates("UNCONFIG");

}

void	RecordEvents(const char* pzEvent) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistControlEvents; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzEvent, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
		pstringlistNew->psName = psStrDup(pzEvent);
		pstringlistNew->pnext = pstringlistControlEvents;
		pstringlistControlEvents = pstringlistNew;
	}
}
void	PrintEventNames(void) {
	struct StringList* pstringlist;
	fprintf(stderr, "List of events:\n");
	for (pstringlist = pstringlistControlEvents; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(stderr, "\t'%s'\n", pstringlist->psName);
	}
}
void	CheckEvent(const char* pzName) {
	struct StringList* pstringlist;
	for (pstringlist = pstringlistControlEvents; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzName, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		fprintf(stderr, "No definition in Process Characteristic file for event named '%s' found in state table\n", pzName);
		g_kerror++;
	}
}

void	RecordActions(const char* pzAction) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistActions; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzAction, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
		pstringlistNew->psName = psStrDup(pzAction);
		pstringlistNew->pnext = pstringlistActions;
		pstringlistActions = pstringlistNew;
	}
}
void	PrintActionNames(void) {
	struct StringList* pstringlist;
	fprintf(stderr, "List of actions:\n");
	for (pstringlist = pstringlistActions; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(stderr, "\t'%s'\n", pstringlist->psName);
	}
}
void	CheckAction(const char* pzName) {
	struct StringList* pstringlist;
	for (pstringlist = pstringlistActions; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzName, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		fprintf(stderr, "No definition in Process Characteristic file for action named '%s' found in state table\n", pzName);
		g_kerror++;
	}
}

void	RecordProcesses(const char* pzProcess) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistProcesses; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzProcess, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
		pstringlistNew->psName = psStrDup(pzProcess);
		pstringlistNew->pnext = pstringlistProcesses;
		pstringlistProcesses = pstringlistNew;
	}
}
void	PrintProcessNames(void) {
	struct StringList* pstringlist;
	fprintf(stderr, "Process list:\n");
	for (pstringlist = pstringlistProcesses; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(stderr, "\t'%s'\n", pstringlist->psName);
	}
}
void	CheckProcessName(const char* pzName) {
	struct StringList* pstringlist;
	for (pstringlist = pstringlistProcesses; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzName, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		fprintf(stderr, "No definition in Process Characteristic file for process named '%s' found in state table\n", pzName);
		g_kerror++;
	}
}

void	RecordProcessesUsed(const char* pzProcess) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistProcessesUsed; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzProcess, pstringlist->psName) == 0) break;
	}
	if (pstringlist == NULL) {
		pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
		pstringlistNew->psName = psStrDup(pzProcess);
		pstringlistNew->pnext = pstringlistProcessesUsed;
		pstringlistProcessesUsed = pstringlistNew;
	}
}

void	RecordFSMStates(const char* pzState) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistFSMStates; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzState, pstringlist->psName) == 0) return; // On list, do not need to duplicate
	}
	pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
	pstringlistNew->psName = psStrDup(pzState);
	pstringlistNew->pnext = pstringlistFSMStates;
	pstringlistFSMStates = pstringlistNew;
}
void	PrintFSMStateNames(void) {
	struct StringList* pstringlist;
	fprintf(stderr, "List of FSM (not process) states:\n");
	for (pstringlist = pstringlistFSMStates; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(stderr, "\t'%s'\n", pstringlist->psName);
	}
}

void	RecordProcessStates(const char* pzState) {
	struct StringList* pstringlistNew;
	struct StringList* pstringlist;

	for (pstringlist = pstringlistProcessStates; pstringlist; pstringlist = pstringlist->pnext) {
		if (strcmp(pzState, pstringlist->psName) == 0) return;
	}
	pstringlistNew = (struct StringList*)calloc(1, sizeof(struct StringList));
	pstringlistNew->psName = psStrDup(pzState);
	pstringlistNew->pnext = pstringlistProcessStates;
	pstringlistProcessStates = pstringlistNew;
}
void	PrintProcessStateNames(void) {
	struct StringList* pstringlist;
	fprintf(stderr, "List of process states:\n");
	for (pstringlist = pstringlistProcessStates; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(stderr, "\t'%s'\n", pstringlist->psName);
	}
}


void	LoadFSM(const char* pzFileName) {
	if (pzFileName == NULL) {
		fprintf(stderr, "Error: no 'state-machine' file specified\n");
		exit(0);
	}
	FILE*	pfile;
	pfile = fopen(pzFileName, "rt");
	if (!pfile) {
		fprintf(stderr, "Could not open file '%s'\n", pzFileName);
		exit(0);
	}

	// Read header row
	char*	pcBuf;
	char*	pcontext = NULL;
	int		icol;
	int		icolActions;
	fgets(abuf, sizeof(abuf), pfile);
	pcBuf = abuf;

	// Ignore System State, Events, and Expected Next State Entries
	pcBuf = strtok_ts(pcBuf, ",", &pcontext);
	pcBuf = strtok_ts(NULL, ",", &pcontext);
	pcBuf = strtok_ts(NULL, ",", &pcontext);
	// Next entry should be Guards
	pcBuf = strtok_ts(NULL, ",", &pcontext);
	if (strcmp(pcBuf, "Guards") != 0) {
		fprintf(stderr, "Error: Did not find 'Guards'\n");
		exit(0);
	}
	// Scan for start of Actions section
	for (icol = 1; ; icol++) {
		pcBuf = strtok_ts(NULL, ",", &pcontext);
		if (pcBuf == NULL) break;
		if (strcmp(pcBuf, "Actions") == 0) {
			icolActions = icol;
			break;
		}
	}
	kprocessActual = icol;
	printf("There are %d processes\n", kprocessActual);

	// Read process definition row
	fgets(abuf, sizeof(abuf), pfile);
	pcBuf = abuf;
	// Ignore System State, Events, and Expected Next State Entries
	pcBuf = strtok_ts(pcBuf, ",", &pcontext);
	pcBuf = strtok_ts(NULL, ",", &pcontext);
	pcBuf = strtok_ts(NULL, ",", &pcontext);
	// Scan through setting up process names
	int iprocess;
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		pcBuf = strtok_ts(NULL, ",", &pcontext);
		if (pcBuf == NULL) break;
		fsm.apsProcess[iprocess] = psStrDup(pcBuf);
		CheckProcessName(pcBuf);
		RecordProcessesUsed(pcBuf);
	}
	// Check same names in action section
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		pcBuf = strtok_ts(NULL, ",", &pcontext);
		if (pcBuf == NULL) break;
		if (strcmp(pcBuf, fsm.apsProcess[iprocess]) != 0) {
			fprintf(stderr, "Error: Mismatch in process names.  Guard has '%s' and Actions has '%s'\n", fsm.apsProcess[iprocess], pcBuf);
			exit(0);
		}
	}

	// Loop through state table
	int irow;
	for (irow = 0; irow != krows; irow++) {
		if (fgets(abuf, sizeof(abuf), pfile) == NULL) break;
		pcBuf = abuf;
		// Process state/event rows
		
		fsm.afsmrow[irow].fState = (abuf[0] != ','); // If it does not start with a comma then it must be a state name
		//Set system state name field
		pcBuf = strtok_ts(pcBuf, ",", &pcontext);;
		fsm.afsmrow[irow].psSystemState = psStrDup(pcBuf);
		if (*pcBuf != '\0' && strncmp(pcBuf, "//", 2) != 0)
			RecordFSMStates(pcBuf);
		//Set event name field
		pcBuf = strtok_ts(NULL, ",", &pcontext);
		fsm.afsmrow[irow].psEvent = psStrDup(pcBuf);
		if (*pcBuf != '\0')
			CheckEvent(pcBuf);
		//Set expected next state field
		pcBuf = strtok_ts(NULL, ",", &pcontext);
		fsm.afsmrow[irow].psExpectedNextState = psStrDup(pcBuf);
		
		// Loop through processes setting state/guard
		for (int iprocess = 0; iprocess != kprocessActual; iprocess++) {
			pcBuf = strtok_ts(NULL, ",", &pcontext);
			if (pcBuf == NULL) break;
			fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard = psStrDup(pcBuf);
			CheckProcessState(fsm.apsProcess[iprocess], pcBuf);
		}
		
		// Loop through processes setting actions
		for (int iprocess = 0; iprocess != kprocessActual; iprocess++) {
			pcBuf = strtok_ts(NULL, ",", &pcontext);
			if (pcBuf != NULL) {
				if (!fsm.afsmrow[irow].fState) {
					fsm.afsmrow[irow].astatePlusAction[iprocess].psAction = psStrDup(pcBuf);
					if (*pcBuf != '\0')
						CheckAction(pcBuf);
				} else {
					fsm.afsmrow[irow].astatePlusAction[iprocess].psAction = psStrDup("");
				}
			}
		}
	}
	// Add predefined states
	RecordFSMStates("NULL");
	RecordFSMStates("CONFIG");
	RecordFSMStates("UNCONFIG");
	fclose(pfile);
	if (irow == krows) {
		fprintf(stderr, "Overflowed fsm table\n");
		exit(0);
	}
	krowsActual = irow;
	printf("There are %d state/event rows\n", irow);
}
void	WriteFSM(const char* psStateFileName, const char* pzCFileName, const char* pzHFileName) {
	char	acTemp[100];

	if (pzCFileName == NULL) {
		fprintf(stderr, "Error: no C file specified\n");
		exit(0);
	}
	FILE* pfileC;
	pfileC = fopen(pzCFileName, "w");
	if (!pfileC) {
		fprintf(stderr, "Could not open '%s'\n", pzCFileName);
		exit(0);
	}

	if (pzHFileName == NULL) {
		fprintf(stderr, "Error: no H file specified\n");
		exit(0);
	}
	FILE* pfileH;
	pfileH = fopen(pzHFileName, "w");
	if (!pfileH) {
		fprintf(stderr, "Could not open '%s'\n", pzHFileName);
		exit(0);
	}

	int		irow;
	int		iprocess;

	fprintf(pfileC, "/* State Machine Description */\n");
	fprintf(pfileC, "/* Created from state file: '%s' */\n", psStateFileName);
	fprintf(pfileC, "/* In a separate file for ease of creation.  Should only be included in ql_controlTask.c */\n");
	fprintf(pfileC, "#include \"ql_controlDefines.h\"\n");

	// Check that KPROCESS is correctly defined
	fprintf(pfileC, "#if KPROCESS < %d\n", kprocessActual);
	fprintf(pfileC, "#error KPROCESS < %d\n", kprocessActual);
	fprintf(pfileC, "#endif\n");

	// Check that KSTATES is correctly defined
	fprintf(pfileC, "#if KSTATES < %d\n", krowsActual);
	fprintf(pfileC, "#error KSTATES < %d\n", krowsActual);
	fprintf(pfileC, "#endif\n");

	// Define the actual number of processes and rows in the state table
	fprintf(pfileC, "#define KPROCESSACTUAL %d\n", kprocessActual);
	fprintf(pfileC, "#define KSTATESACTUAL %d\n", krowsActual);

	fprintf(pfileC, "\n%-55s", "//");
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		fprintf(pfileC, " %-20s", fsm.apsProcess[iprocess]);
	}
	fprintf(pfileC, "\nstruct GSMrow afsmrow[KSTATES] = {\n");
	for (irow = 0; irow != krowsActual; irow++) {
		WriteFSMrow(pfileC, &(fsm.afsmrow[irow]), (irow == (krowsActual - 1)));
	}
	fprintf(pfileC, "};\n\n");
	fprintf(pfileC, "struct GSMrow   currentState = \n");
	WriteFSMrow(pfileC, &currentState, true);
	fprintf(pfileC, ";\n");
	// reference to states
	fprintf(pfileC, "extern enum process_state LPSD_State;\n");
	fprintf(pfileC, "\n");

	// define routine calls <process_name>_{Stop(int),Start(int);
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		fprintf(pfileC, "enum process_state %s_FSMAction(enum process_action, void*);\n", fsm.apsProcess[iprocess]);
		fprintf(pfileC, "extern int         %s_FSMConfigData;\n", fsm.apsProcess[iprocess]);
	}
	fprintf(pfileC, "\n");

	// Define ProcessActions structures
	fprintf(pfileC, "struct ProcessActions apaction[KPROCESSACTUAL] = {\n");
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		strcpy(acTemp, "(void*)&");
		strcat(acTemp, fsm.apsProcess[iprocess]);
		fprintf(pfileC, "    {%15s_FSMAction, %25s_FSMConfigData},\n", fsm.apsProcess[iprocess], acTemp);
	}
	fprintf(pfileC, "};\n\n");

	// Define map from process number to user friendly string
	fprintf(pfileC, "char* apsFromProcess[] = {\n");
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		fprintf(pfileC, "    \"%s\",\n", fsm.apsProcess[iprocess]);
	}
	fprintf(pfileC, "};\n\n");

	// Start fsm.h
	fprintf(pfileH, "/* Defines for FSM Control */\n");
	fprintf(pfileH, "#ifndef FSM_H_\n");
	fprintf(pfileH, "#define FSM_H_\n");

	// Enumerate Process States
	struct StringList*	pstringlist;
	fprintf(pfileH, "/* States */\n");
	fprintf(pfileH, "enum process_state {\n");
	pstringlist = pstringlistProcessStates;
	fprintf(pfileH, "	PSTATE_%s = 0,\n", pstringlist->psName);
	for (pstringlist = pstringlist->pnext; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileH, "	PSTATE_%s,\n", pstringlist->psName);
	}
	fprintf(pfileH, "};\n\n");

	// Define map from FSM state number to friendly string
	fprintf(pfileC, "char* apsFromFSMS[] = {\n");

	for (pstringlist = pstringlistFSMStates; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileC, "	\"%s\",\n", pstringlist->psName);
	}
	fprintf(pfileC, "};\n\n");

	// Define map from process state number to friendly string
	fprintf(pfileC, "char* apsFromPS[] = {\n");

	for (pstringlist = pstringlistProcessStates; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileC, "	\"%s\",\n", pstringlist->psName);
	}
	fprintf(pfileC, "};\n\n");

	// Enumerate Control Events
	fprintf(pfileH, "/* Events */\n");
	fprintf(pfileH, "enum control_event {\n");
	pstringlist = pstringlistControlEvents;
	fprintf(pfileH, "	CEVENT_%s = 0,\n", pstringlist->psName);
	for (pstringlist = pstringlist->pnext; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileH, "	CEVENT_%s,\n", pstringlist->psName);
	}
	fprintf(pfileH, "};\n\n");

	// Define map from Control Event number to friendly string
	fprintf(pfileC, "char* apsFromCE[] = {\n");

	for (pstringlist = pstringlistControlEvents; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileC, "	\"%s\",\n", pstringlist->psName);
	}
	fprintf(pfileC, "};\n\n");

	// Enumerate actions
	fprintf(pfileH, "/* Actions */\n");
	fprintf(pfileH, "enum process_action {\n");
	pstringlist = pstringlistActions;
	fprintf(pfileH, "	PACTION_%s = 0,\n", pstringlist->psName);
	for (pstringlist = pstringlist->pnext; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileH, "	PACTION_%s,\n", pstringlist->psName);
	}
	fprintf(pfileH, "};\n\n");

	// Define map from Action number to friendly string
	fprintf(pfileC, "char* apsFromPA[] = {\n");

	for (pstringlist = pstringlistActions; pstringlist; pstringlist = pstringlist->pnext) {
		fprintf(pfileC, "	\"%s\",\n", pstringlist->psName);
	}
	fprintf(pfileC, "};\n\n");
	fclose(pfileC);

	fprintf(pfileH, "#endif //FSM_H_\n");
	fclose(pfileH);
}
void	WriteFSMrow(FILE* pfile, struct fsmrow* pfsmrow, bool fLastRow) {
	int		iprocess;
	char	acTemp[100];

	if (pfsmrow->fState) {
		fprintf(pfile, "\n  {true,  %-20s, %-22s,{ ", psQuotedString(pfsmrow->psSystemState), "0");
	}
	else {
		fprintf(pfile, "\n  {false, %-20s, CEVENT_%-15s,{ ", "\"\"", pfsmrow->psEvent);
	}
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		if (*(pfsmrow->astatePlusAction[iprocess].psStateOrGuard) == '\0') {
			strcpy(acTemp, "0");
		}
		else {
			strcpy(acTemp, "PSTATE_");
			strcat(acTemp, pfsmrow->astatePlusAction[iprocess].psStateOrGuard);
		}
		fprintf(pfile, "%-20s,", acTemp);
	}
	fprintf(pfile, "},\n%55s{ ", "");
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		if (pfsmrow->astatePlusAction[iprocess].psAction == NULL) continue;
		if (*(pfsmrow->astatePlusAction[iprocess].psAction) == '\0') {
			strcpy(acTemp, "0");
		}
		else {
			strcpy(acTemp, "PACTION_");
			strcat(acTemp, pfsmrow->astatePlusAction[iprocess].psAction);
		}
		fprintf(pfile, "%-20s,", acTemp);
	}
	fprintf(pfile, "}");
	if (fLastRow) {
		fprintf(pfile, "}\n");
	}
	else {
		fprintf(pfile, "},");
	}
}
void	PrintFSM(void) {
	int	irow;
	int	iprocess;

	printf("------------------GSM-----------------------------------");
	printf("\n%53s", "");
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		printf(" %-12s", fsm.apsProcess[iprocess]);
	}
	printf("\n");
	for (irow = 0; irow != krowsActual; irow++) {
		if (fsm.afsmrow[irow].fState) {
			printf("\n%2d State[%-42s]:", irow, fsm.afsmrow[irow].psSystemState);
		}
		else {
			printf("\n        >%-20s->%-20s :", fsm.afsmrow[irow].psEvent, fsm.afsmrow[irow].psExpectedNextState);
		}
		for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
			printf(" %-12s", fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard);
		}
		printf("\n%53s", "");
		for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
			printf(" %-12s", fsm.afsmrow[irow].astatePlusAction[iprocess].psAction);
		}
	}
	printf("\n\n");
}

// Run simulations
void SimulateEvents(const char* psFileName) {
	if (psFileName == NULL) {
		fprintf(stderr, "Error: no EventList file specified\n");
		exit(0);
	}
	FILE* pfile;
	pfile = fopen(psFileName, "rt");
	if (!pfile) {
		fprintf(stderr, "Could not open '%s'\n", psFileName);
		exit(0);
	}

	char*	ps;
	char*	psEvent;
	char*	psInterrupt;
	while (fgets(abuf, sizeof(abuf), pfile)) {
		if (abuf[0] == '\0' || abuf[0] == '\n')
			continue;
		if (abuf[strlen(abuf) - 1] == '\n') abuf[strlen(abuf) - 1] = '\0';
			ps = strtok_ts(abuf, ",", NULL);
			if (strcmp(ps, "EVENT") == 0) { // Process event
				psEvent = strtok_ts(NULL, ",", NULL); // Get pointer to event
				ProcessEvent(psEvent);
			} else if (strcmp(ps, "INT") == 0) { // Process interrupt
				psInterrupt = strtok_ts(NULL, ",", NULL); // Get pointer to interrupt
				ProcessInterrupt(psInterrupt);
			}
	}
}
void SetCurrentState(void) {
	int	irow;
	for (irow = 0; irow != krowsActual; irow++) {
		if (fsm.afsmrow[irow].fState && (strcmp(fsm.afsmrow[irow].psSystemState, "INITIAL") == 0)) {
			currentState = fsm.afsmrow[irow];
			irowCurrentState = FindCurrentState();
			return;
		}
	}
	fprintf(stderr, "Missing 'INITIAL' state\n");
	exit(0);
}

void ProcessEvent(const char* psEvent) {
	int		irow;
	int		irowEvent;
	int		iprocess;
	char*	psAction;
	char*	psProcess;
	char*	psGuard;
	char*	psProcessState;
	bool	fValid = false;

	irow = irowCurrentState;
	printf("SysState='%s'; Event: '%s'\n", fsm.afsmrow[irow].psSystemState, psEvent);

	// Now find Event arc associated with the current state
	for (irowEvent = irow + 1; !fsm.afsmrow[irowEvent].fState && irowEvent < krowsActual; irowEvent++) {
		if (strcmp(fsm.afsmrow[irowEvent].psEvent, psEvent) == 0) {
			// Need to check if all guards are valid
			fValid = true;
			for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
				psGuard = fsm.afsmrow[irowEvent].astatePlusAction[iprocess].psStateOrGuard;
				if (psGuard && *psGuard != '\0') {
					psProcessState = currentState.astatePlusAction[iprocess].psStateOrGuard;
					if (psProcessState && strcmp(psProcessState, "DONT_CARE") == 0) continue;
					if (psProcessState && strcmp(psProcessState, psGuard) != 0) fValid = false;
				}
			}
		}
		if (fValid) break;
	}
	if (fsm.afsmrow[irowEvent].fState) {
		fprintf(stderr, "Failed to find an arc assocated with event '%s' leaving SystemState '%s'\n", psEvent, fsm.afsmrow[irow].psSystemState);
	}
	// Scan thru finding actions
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		psAction = fsm.afsmrow[irowEvent].astatePlusAction[iprocess].psAction;
		//printf("[%d] => %s\n", iprocess, fsm.afsmrow[irowEvent].astatePlusAction[iprocess].psAction);
		if (*psAction != '\0') { // Got work to do
			psProcess = fsm.apsProcess[iprocess];
			//printf("process=%s\n", psProcess);
			DoAction(psProcess, psAction, iprocess);
		}
	}
	irow = FindCurrentState();
	printf("New state: %s\n\n", fsm.afsmrow[irow].psSystemState);
	irowCurrentState = irow;
}

char*	psQuotedString(const char* pzIn) {
	static char	acTemp[50];
	strcpy(acTemp, "\"");
	strcat(acTemp, pzIn);
	strcat(acTemp, "\"");
	return(acTemp);
}

void	WriteFSM_cli_table(const char* psFileName) {
	if (psFileName == NULL) {
		fprintf(stderr, "Error: no cli_table file specified\n");
		exit(0);
	}
	FILE* pfile;
	pfile = fopen(psFileName, "w");
	if (!pfile) {
		fprintf(stderr, "Could not open '%s'\n", psFileName);
		exit(0);
	}

	// Enumerate Control Events
	struct StringList* pstringlist;
	char acTemp[100];

	fprintf(pfile, "/* FSM sub menu that defines all control events */\n");
	fprintf(pfile, "#include \"Fw_global_config.h\"\n");
	fprintf(pfile, "#include \"ql_controlTask.h\"\n");
	fprintf(pfile, "#include \"cli.h\"\n");
	fprintf(pfile, "\nvoid cli_fsm_event(const struct cli_cmd_entry *pEntry);\n\n");
	fprintf(pfile, "const struct cli_cmd_entry fsm_menu[] = {\n");
	pstringlist = pstringlistControlEvents;
	
	for (pstringlist = pstringlist->pnext; pstringlist; pstringlist = pstringlist->pnext) {
		strcpy(acTemp, pstringlist->psName);
		for (char* pc = acTemp;*pc; pc++) {
			if ('A' <= *pc && *pc <= 'Z') {
				*pc = *pc - ('A' - 'a');
			}
		}
		fprintf(pfile, "	    CLI_CMD_WITH_ARG( \"%s\", cli_fsm_event, CEVENT_%s, \"sends %s event\" ),\n", acTemp, pstringlist->psName, pstringlist->psName);
	}
	fprintf(pfile, "	CLI_CMD_TERMINATE()\n};\n");
}


void	PrintCurrentState(void) {
	int iprocess;

	if (currentState.fState) {
		printf("\nState[%-20s]:", currentState.psSystemState);
	} else {
		printf("Event[%-20s->%-20s]:", currentState.psEvent, currentState.psExpectedNextState);
	}
	for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
		printf(" <%s/%s>", currentState.astatePlusAction[iprocess].psStateOrGuard, currentState.astatePlusAction[iprocess].psAction);
	}
	printf("\n");
}

void	CheckProcessState(const char* pzProcessName, const char* pzState) {
	int	iaction;
	int iprocess;
	char*	pzNextState;
	char*	pzAction;

	for (iprocess = 0; iprocess != 50; iprocess++) {
		if (strcmp(pzProcessName, aprocess[iprocess].psName) == 0) break;
	}

	if (*pzState == '\0') return;
	if (strcmp(pzState, "DONT_CARE") == 0) return;
	if (strcmp(pzState, "UNCONFIG") == 0) return;

	for (iaction = 0; iaction != kactions; iaction++) {
		pzAction = aprocess[iprocess].aactionPlusNextState[iaction].psAction;
		pzNextState = aprocess[iprocess].aactionPlusNextState[iaction].psNextState;
		if ((pzNextState == NULL || *pzNextState == '\0') && (pzAction == NULL || *pzAction == '\0')) break;
		if (pzNextState == NULL || *pzNextState == '\0') pzNextState = (char *)"--";
		if (pzAction == NULL || *pzAction == '\0') pzAction = (char *)"--";
		if (strcmp(pzNextState, pzState) == 0)
			return;
	}
	fprintf(stderr, "Did not find state '%s' for process '%s'\n", pzState, aprocess[iprocess].psName);
	g_kerror++;
}
void	PrintProcessDescriptions(void) {
	int	iprocess;
	int	iaction;
	char*	psNextState;
	char*	psAction;

	printf("--------------Process Descriptions--------------\n");
	for (iprocess = 0; iprocess != kprocess; iprocess++) {	
		if (aprocess[iprocess].psName != NULL && *aprocess[iprocess].psName != '\0') {
			printf("%-20s\n", aprocess[iprocess].psName);
			for (iaction = 0; iaction != kactions; iaction++) {
				psAction = aprocess[iprocess].aactionPlusNextState[iaction].psAction;
				psNextState = aprocess[iprocess].aactionPlusNextState[iaction].psNextState;
				if ((psNextState == NULL || *psNextState == '\0') && (psAction == NULL || *psAction == '\0')) break;
				if (psNextState == NULL || *psNextState == '\0') psNextState = (char *)"--";
				if (psAction == NULL || *psAction == '\0') psAction = (char *)"--";
				printf("    [%-20s --> %-10s]\n", psAction, psNextState);
			}
		}
	}
	printf("--------------Process Descriptions--------------\n\n");

}

void	PrintInterruptDescriptions(void) {
	int	iprocess;
	int	iinterrupt;
	char*	psNextState;
	char*	psEvent;
	char*	psInterrupt;

	printf("--------------Interrupt Descriptions--------------\n");
	for (iprocess = 0; iprocess != kprocess; iprocess++) {
		if (ainterrupt[iprocess].psName != NULL && *ainterrupt[iprocess].psName != '\0') {
			printf("%-20s\n", ainterrupt[iprocess].psName);
			for (iinterrupt = 0; iinterrupt != kactions; iinterrupt++) {
				psNextState = ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psNextState;
				psEvent = ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psEvent;
				psInterrupt = ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psInterrupt;
				if ((psNextState == NULL || *psNextState == '\0') && (psEvent == NULL || *psEvent == '\0')) break;
				if (psNextState == NULL || *psNextState == '\0') psNextState = (char *)"--";
				if (psEvent == NULL || *psEvent == '\0') psEvent = (char *)"--";
				printf("    [%-20s --> %-10s %-10s]\n", psInterrupt, psNextState, psEvent);
			}
		}
	}
	printf("--------------Interrupt Descriptions--------------\n\n");

}

int	FindCurrentState(void) {
	int		irow;
	int		iprocess;
	bool	fMatch;

	for (irow = 0; irow != krowsActual; irow++) {
		if (fsm.afsmrow->fState) {
			fMatch = true;
			//printf("SysState = '%s'\n", fsm.afsmrow[irow].psSystemState);
			for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
				//printf("irow=%d, iprocess=%d, GSM=%s, CS=%s\n", irow, iprocess, fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard, currentState.astatePlusAction[iprocess].psStateOrGuard);
				if (strcmp(fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard, "DONT_CARE") != 0) {
					if (strcmp(fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard, currentState.astatePlusAction[iprocess].psStateOrGuard) != 0) {
						fMatch = false;
						break;
					}
				}
			}
		}
		if (fMatch) break;
	}
	if (irow == krowsActual) {
		PrintCurrentState();
		fprintf(stderr, "Error: Can't find CurrentState in GSM table\n");
		for (irow = 0; irow != krowsActual; irow++) {
			if (fsm.afsmrow[irow].fState) {
				fMatch = true;
				printf("SysState = '%s'\n", fsm.afsmrow[irow].psSystemState);
				for (iprocess = 0; iprocess != kprocessActual; iprocess++) {
					printf("%15s, CS=%10s, FSM=%10s:", fsm.apsProcess[iprocess], currentState.astatePlusAction[iprocess].psStateOrGuard, fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard);
					if (strcmp(fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard, "DONT_CARE") != 0) {
						if (strcmp(fsm.afsmrow[irow].astatePlusAction[iprocess].psStateOrGuard, currentState.astatePlusAction[iprocess].psStateOrGuard) != 0) {
							fMatch = false;
							printf(" fail\n");
							break;
						} else {
							printf(" match\n");
						}
					} else {
						printf(" match\n");
					}
				}
			}
			if (fMatch) break;
		}

		exit(0);
	}
	currentState.psSystemState = fsm.afsmrow[irow].psSystemState;
	return(irow);
}

void	DoAction(const char* psProcess, const char* psAction, int iprocessGSM) {
	int	iprocess;
	int	iaction;

	for (iprocess = 0; iprocess != kprocess; iprocess++) {
		if (aprocess[iprocess].psName != NULL && strcmp(psProcess, aprocess[iprocess].psName) == 0) {
			for (iaction = 0; iaction != kactions; iaction++) {
				if (aprocess[iprocess].aactionPlusNextState[iaction].psAction && strcmp(psAction, aprocess[iprocess].aactionPlusNextState[iaction].psAction) == 0) {
					currentState.astatePlusAction[iprocessGSM].psStateOrGuard = aprocess[iprocess].aactionPlusNextState[iaction].psNextState;
					printf("   %s = %s->%s()\n", currentState.astatePlusAction[iprocessGSM].psStateOrGuard, psProcess, psAction);
					return;
				}
			}
			if (iaction == kactions) {
				fprintf(stderr, "DoAction failed to find %s->%s()\n", psProcess, psAction);
				exit(0);
			}
		}
	}
}

void	ProcessInterrupt(const char* psInterrupt) {
	int	iprocess;
	int	iinterrupt;

	printf("Got interrupt: %s\n", psInterrupt);
	for (iprocess = 0; iprocess != kprocess; iprocess++) {
			for (iinterrupt = 0; iinterrupt != kactions; iinterrupt++) {
				if (ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psInterrupt && strcmp(psInterrupt, ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psInterrupt) == 0) {
					if (*ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psNextState != '\0') {
						currentState.astatePlusAction[iprocess].psStateOrGuard = ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psNextState;
						printf("  Updating state: %s\n", ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psNextState);
					}
					if (*ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psEvent != '\0') {
						printf("  Generating event: %s\n", ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psEvent);
						ProcessEvent(ainterrupt[iprocess].ainterruptPlusNextState[iinterrupt].psEvent);
					}
					return;
				}
			}
	}
	if (iprocess == kprocess) {
		fprintf(stderr, "ProcessInterrupt failed to find %s\n", psInterrupt);
		exit(0);
	}
}

char*	psStrDup(const char * psIn) {
	char*	psOut;
	size_t	kchar = strlen(psIn) + 1;

	psOut = (char*)malloc(kchar);
	strcpy(psOut, psIn);
	return(psOut);
}

char acTemp[100];

char* strtok_ts(char* pc, const char* psDelim, char** context) {
	static char*	pcStart;
	static char*	pcEnd;
	char*	pcDest = acTemp;
	*pcDest = '\0';

	if (pc != NULL) pcStart = pc;
	if (*pcStart == '\0') return (NULL);
	pcEnd = pcStart;
	while (*pcEnd != '\0' && *pcEnd != ',' && *pcEnd != '\n') {
		// Strip initial white space
		if (*pcEnd != ' ' && *pcEnd != '\t') {
			*pcDest++ = *pcEnd;
		}
		pcEnd++;
	}
	*pcDest = '\0';
	// Remove any trailing white space
	for (--pcDest; pcDest >= acTemp && (*pcDest == ' ' || *pcDest == '\t' || *pcDest == '\r'); --pcDest) {
		*pcDest = '\0';
	}
	pcStart = pcEnd + 1;
	return (acTemp);
}
