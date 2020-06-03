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

#pragma once

const int kprocess = 50;
const int krows = 100;
const int kactions = 20;

void	Usage(void);

void	GenerateFSM(const char*, const char*, const char*, const char*);
void	LoadFSM(const char*);
void	PrintFSM(void);
void	WriteFSM(const char*, const char*, const char*);
void	WriteFSMrow(FILE*, struct fsmrow*, bool);
void	WriteFSM_cli_table(const char*);

void	LoadProcessCharacteristics(const char*);

void	RecordFSMStates(const char*);
void	PrintFSMStateNames(void);

void	RecordProcessStates(const char*);
void	PrintProcessStateNames(void);

void	RecordEvents(const char*);
void	PrintEventNames(void);
void	CheckEvent(const char*);

void	RecordActions(const char*);
void	PrintActionNames(void);
void	CheckAction(const char*);

void	RecordProcesses(const char*);
void	PrintProcessNames(void);
void	CheckProcessName(const char*);
void	CheckProcessState(const char* pzProcessName, const char* pzState);
 
void	SimulateEvents(const char*);
void	SetCurrentState(void);




void	PrintProcessDescriptions(void);
void	PrintInterruptDescriptions(void);
void	PrintCurrentState(void);
int		FindCurrentState(void);

void	ProcessEvent(const char*);
void	ProcessInterrupt(const char*);
void	DoAction(const char*, const char*, const int);


char*	strtok_ts(char*, const char*, char**);
char*	psStrDup(const char *);
char*	psQuotedString(const char* psIn);

struct StringList {
	char*	psName;
	struct	StringList*	pnext;
};

struct actionPlusNextState {
	char*	psAction;
	char*	psNextState;
};
struct process {
	char*	psName;
	struct actionPlusNextState aactionPlusNextState[kactions];
};


struct interruptPlusNextState {
	char*	psInterrupt;
	char*	psNextState;
	char*	psEvent;
};

struct interrupt {
	char*	psName;
	struct interruptPlusNextState ainterruptPlusNextState[kactions];
};

struct statePlusAction {
	char*	psStateOrGuard;
	char*	psAction;
};

struct fsmrow {
	bool	fState;
	char*	psEvent;
	char*	psExpectedNextState;
	char*	psSystemState;
	struct  statePlusAction astatePlusAction[kprocess];
};

struct GlobalStateMachine {
	char*			apsProcess[kprocess];
	struct fsmrow	afsmrow[krows];
};

extern struct fsmrow currentState;