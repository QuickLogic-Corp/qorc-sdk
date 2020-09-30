@echo off
@echo Converts FSMstates.xlsx state table to C source code
@echo using Process/Actions/Events defined in RCProcessCharacteristics.txt file.
@echo Also, simulates the state transitions specificied in RCEvents.txt file.

REM first convert FSMstates.xlsx to FSMstates.csv
cscript xls2csv.vbs

REM Simulates the Remote Control State machine
.\..\..\..\Tools\fsmgen\x64\Release\fsmgen.exe --state-machine FSMstates.csv --device-description RCProcessCharacteristics.txt --simulation-script RCEvents.txt

REM Rename fsm.c source file to fsm_tables.h
move fsm.c fsm_tables.h
