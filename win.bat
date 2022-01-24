@ECHO OFF

REM Windows compiler for a Lua Server project
REM Copyright (c) 2022 RedHolms
REM Recommended to run from a Visual Studio developer console

REM Arguments parser
SET /a ARGS_COUNT=0
:REPEAT
  SET /a ARGS_COUNT+=1
  SET "arg_%ARGS_COUNT%=%~1"
  SHIFT
  IF DEFINED arg_%ARGS_COUNT% (
    GOTO :REPEAT
  ) ELSE (
    SET /a ARGS_COUNT-=1
  )    
SET /a ARG_0=ARGS_COUNT

REM Arguments decoder
SET COMPILE_BASE=1
SET COMPILE_SOCKET=1
SET COMPILE_SOCKLIB=1
SET NO_LINK=0
SET NO_MT=0
SET REMOVE_WARN=0
FOR /l %%I IN (1,1,!ARG_0!) DO (
  ECHO ARG_%%I: "!ARG_%%I!"
  IF !ARG_%%I! == /? (
    GOTO :PRINT_USAGE
  ) ELSE IF !ARG_%%I! == /cmponly_socket (
    SET COMPILE_BASE=0
    SET COMPILE_SOCKET=1
    SET COMPILE_SOCKLIB=0
  ) ELSE IF !ARG_%%I! == /cmponly_socklib (
    SET COMPILE_BASE=0
    SET COMPILE_SOCKET=0
    SET COMPILE_SOCKLIB=1
  ) ELSE IF !ARG_%%I! == /nocmp_base (
    SET COMPILE_BASE=0
    SET COMPILE_SOCKET=1
    SET COMPILE_SOCKLIB=1
  ) ELSE IF !ARG_&&I! == /nocmp (
    SET COMPILE_BASE=0
    SET COMPILE_SOCKET=0
    SET COMPILE_SOCKLIB=0
  ) ELSE IF !ARG_%%I! == /no_link (
    SET NO_LINK=1
  ) ELSE IF !ARG_%%I! == /no_mt (
    SET NO_MT=1
  ) ELSE IF !ARG_%%I! == /rm_warn (
    SET REMOVE_WARN=1
  ) ELSE (
    ECHO Invalid flag #%%I: !ARG_%%I!
    GOTO :PRINT_AVAILABLE_FLAGS
  )
)

IF REMOVE_WARN == 0 (
  ECHO !!!!!!!!!!!!!!!!!!!!!!!!
  ECHO Before compiling make sure Microsoft C Compiler, linker and Windows Kits is in your path. 
  ECHO Else, use a Visual Studio developer console
  ECHO !!!!!!!!!!!!!!!!!!!!!!!!
  PAUSE
)

REM Some macroses
SET COMPILE=cl /nologo /MD /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
SET LINK=link /nologo
SET MT=mt /nologo

REM Pathes
SET OUT_DIR=%CD%/out
SET OBJ_DIR=%OUT_DIR%/obj

SET SOURCE_DIR=%CD%/src
SET LUA_SRCDIR=%SOURCE_DIR%/lua
SET SOCKET_SRCDIR=%SOURCE_DIR%/socket

ECHO Compiling... Please, wait
ECHO Binaries will be stored in "out" directory

MKDIR %OUT_DIR%
MKDIR %OBJ_DIR%

:COMPILE_DLL
REM Compiling DLL without Lua Stand-Alone interpreter
CALL :COMPILE_BASE
CALL :COMPILE_SOCKLIB
CALL :COMPILE_SOCKET
CALL :LINK_DLL

REM Coplining Lua Stand-Alone interpreter
CALL :LINK_INTER

ECHO Complining ended!
GOTO :EOF


REM Usage printer function
:PRINT_USAGE
ECHO Usage: %n [flags]
:PRINT_AVAILABLE_FLAGS
ECHO Available flags:
ECHO.   /?                - Print Usage
ECHO.   /cmponly_socket   - Compile only socket package(expected all other packages already compiled)
ECHO.   /cmponly_socklib  - Compile only lua socket library(expected all other packages already compiled)
ECHO.   /nocmp_base       - Don't compile base lua files(expected all other packages already compiled)
ECHO.   /nocmp            - Don't compile everything(expected all packages already compiled)
ECHO.   /no_link          - Don't link all object files in executable and DLL
ECHO.   /no_mt            - Don't use Windows Kits
ECHO.   /rm_warn          - Remove warning before compilation
GOTO :EOF


REM Base compiler function
:COMPILE_BASE
IF %COMPILE_BASE% == 0^
  GOTO :EOF
SET PREV_PATH=%CD%
CD %OBJ_DIR%
%COMPILE% /DLUA_BUILD_AS_DLL %LUA_SRCDIR%/l*.c
CD %PREV_PATH%
GOTO :EOF


REM Socket compiler function
:COMPILE_SOCKET
IF %COMPILE_SOCKET% == 0^
  GOTO :EOF
SET PREV_PATH=%CD%
CD %OBJ_DIR%
%COMPILE% %SOCKET_SRCDIR%/*.cpp
CD %PREV_PATH%
GOTO :EOF


REM Socklib compiler function
:COMPILE_SOCKLIB
IF %COMPILE_SOCKLIB% == 0^
  GOTO :EOF
SET PREV_PATH=%CD%
CD %OBJ_DIR%
%COMPILE% /DLUA_BUILD_AS_DLL %LUA_SRCDIR%/lsocklib.cpp
CD %PREV_PATH%
GOTO :EOF


REM DLL Link function
:LINK_DLL
IF %NO_LINK% == 1^
  GOTO :EOF

SET PREV_PATH=%CD%

CD %OBJ_DIR%
REN lua.obj lua.obj.~

CD %OUT_DIR%
%LINK% /DLL /out:lua51.dll %OBJ_DIR%/*.obj
IF %NO_MT% == 0 (
  IF EXIST lua51.dll.manifest^
    %MT% -manifest lua51.dll.manifest -outputresource:lua51.dll;2
)

CD %OBJ_DIR%
REN lua.obj.~ lua.obj

CD %PREV_PATH%
GOTO :EOF


REM Interpreter Link function
:LINK_INTER
SET PREV_PATH=%CD%

CD %OUT_DIR%
%LINK% /out:lua.exe %OBJ_DIR%/lua.obj lua51.lib
IF %NO_MT% == 0 (
  IF EXIST lua.exe.manifest^
    %MT% -manifest lua.exe.manifest -outputresource:lua.exe
)

CD %PREV_PATH%
GOTO :EOF