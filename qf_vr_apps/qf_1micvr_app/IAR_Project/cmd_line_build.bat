@echo off

rem Where is IAR installed

@rem you will need to adjust this accordingly.
set IAR_ROOT_DIR=C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.0

set IAR_BIN_DIR=%IAR_ROOT_DIR%\common\bin

set IARBUILD_EXE=%IAR_BIN_DIR%\iarbuild.exe

if exist "%IARBUILD_EXE%" (
   rem Good nothing to do
) else (
   echo "Cannot find: %IARBUILD_EXE%"
   rem exit 1
)

"%IARBUILD_EXE%" qf_1micvr_app.ewp -clean Debug
"%IARBUILD_EXE%" qf_1micvr_app.ewp -build Debug

