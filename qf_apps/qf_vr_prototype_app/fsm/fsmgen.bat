@echo

set "var=%cd%"
	cd /D %1
	REM Simulates the Remote Control State machine
	.\..\..\..\Tools\fsmgen\x64\Release\fsmgen.exe --state-machine FSMstates.csv --device-description RCProcessCharacteristics.txt --simulation-script RCEvents.txt
	cd /D %var%

cmd /k
exit

call :FileModTime FSMstates.csv A
call :FileModTime fsm.h B
set "diff=0"
if defined A if defined B set /a diff=B-A
if %A% gtr %B% if %diff% lss -2 (
	set "var=%cd%"
	cd /D %1
	REM Simulates the Remote Control State machine
	.\..\..\..\..\Tools\fsmgen\x64\Release\fsmgen.exe --state-machine FSMstates.csv --device-description RCProcessCharacteristics.txt --simulation-script RCEvents.txt
	cd /D %var%
)
cmd /k
exit

:FileModTime  File  [TimeVar]
::
::  Computes the Unix time (epoch time) for the last modified timestamp for File.
::  The result is an empty string if the file does not exist. Stores the result
::  in TimeVar, or prints the result if TimeVar is not specified.
::
::  Unix time = number of seconds since midnight, January 1, 1970 GMT
::
setlocal disableDelayedExpansion
:: Get full path of file
for %%F in ("%~1") do set "file=%%~fF"
:: Get last modified time in YYYYMMDDHHMMSS format
set "time="
for /f "skip=1 delims=,. tokens=2" %%A in (
  'wmic datafile where "name='%file:\=\\%'" get lastModified /format:csv 2^>nul'
) do set "ts=%%A"
set "ts=%ts%"
:: Convert time to Unix time (aka epoch time)
if defined ts (
  set /a "yy=10000%ts:~0,4% %% 10000, mm=100%ts:~4,2% %% 100, dd=100%ts:~6,2% %% 100"
  set /a "dd=dd-2472663+1461*(yy+4800+(mm-14)/12)/4+367*(mm-2-(mm-14)/12*12)/12-3*((yy+4900+(mm-14)/12)/100)/4"
  set /a "ss=(((1%ts:~8,2%*60)+1%ts:~10,2%)*60)+1%ts:~12,2%-366100-%ts:~21,1%((1%ts:~22,3%*60)-60000)"
  set /a "ts=ss+dd*86400"
)
:: Return the result
endlocal & if "%~2" neq "" (set "%~2=%ts%") else echo(%ts%




