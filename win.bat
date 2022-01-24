@ECHO OFF

REM Windows compiler for a Lua Server project
REM Copyright (c) 2022 RedHolms
REM Recommended to run from a Visual Studio developer console

SET FILE_NAME=%~n0
SET FILE_EXT=%~x0

SETLOCAL enabledelayedexpansion

REM Arguments parser
SET /a ARGS_COUNT=0
:REPEAT
  SET /a ARGS_COUNT+=1
  SET "ARG_%ARGS_COUNT%=%~1"
  SHIFT
  IF DEFINED ARG_%ARGS_COUNT% (
    GOTO :REPEAT
  ) ELSE (
    SET /a ARGS_COUNT-=1
  )    
SET /a ARG_0=%ARGS_COUNT%

REM Config
SET FLAG_COMPILE_BASE=1
SET FLAG_COMPILE_SOCKET=1
SET FLAG_COMPILE_SOCKLIB=1
SET FLAG_NO_LINK=0
SET FLAG_NO_MT=0
SET FLAG_REMOVE_WARN=0

REM Arguments decoder
FOR /l %%i IN (1,1,!ARG_0!) DO (
  IF !ARG_%%i! == /? (
    GOTO :PRINT_USAGE
  ) ELSE IF !ARG_%%i! == /cmponly_socket (
    SET FLAG_COMPILE_BASE=0
    SET FLAG_COMPILE_SOCKET=1
    SET FLAG_COMPILE_SOCKLIB=0
  ) ELSE IF !ARG_%%i! == /cmponly_socklib (
    SET FLAG_COMPILE_BASE=0
    SET FLAG_COMPILE_SOCKET=0
    SET FLAG_COMPILE_SOCKLIB=1
  ) ELSE IF !ARG_%%i! == /nocmp_base (
    SET FLAG_COMPILE_BASE=0
    SET FLAG_COMPILE_SOCKET=1
    SET FLAG_COMPILE_SOCKLIB=1
  ) ELSE IF !ARG_%%i! == /nocmp (
    SET FLAG_COMPILE_BASE=0
    SET FLAG_COMPILE_SOCKET=0
    SET FLAG_COMPILE_SOCKLIB=0
  ) ELSE IF !ARG_%%i! == /no_link (
    SET FLAG_NO_LINK=1
  ) ELSE IF !ARG_%%i! == /no_mt (
    SET FLAG_NO_MT=1
  ) ELSE IF !ARG_%%i! == /rm_warn (
    SET FLAG_REMOVE_WARN=1
  ) ELSE (
    ECHO Invalid flag #%%i: !ARG_%%i!
    GOTO :PRINT_AVAILABLE_FLAGS
  )
)

IF %FLAG_REMOVE_WARN% == 0 (
  ECHO !!!!!!!!!!!!!!!!!!!!!!!!
  ECHO Before compiling make sure Microsoft C Compiler, linker and Windows Kits is in your path. 
  ECHO Else, use a Visual Studio developer console
  ECHO !!!!!!!!!!!!!!!!!!!!!!!!
  PAUSE
)

REM Some macroses
SET LUA_SERVER_COMPILE=cl /nologo /MD /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
SET LUA_SERVER_LINK=link /nologo
SET LUA_SERVER_MT=mt /nologo

REM Pathes
SET _OUT_DIR=out
REM OBJ_DIR may be in OUT_DIR directory
SET _OBJ_DIR=obj

SET _OBJ_PATH=%_OUT_DIR%/%_OBJ_DIR%
SET _OBJ_RETPATH=../..

SET _OUT_PATH=%_OUT_DIR%
SET _OUT_RETPATH=..

SET _SOURCE_DIR=%CD%/src
SET _LUA_SRCDIR=%_SOURCE_DIR%/lua
SET _SOCKET_SRCDIR=%_SOURCE_DIR%/socket

ECHO Compiling... Please, wait
ECHO Binaries will be stored in "out" directory

MKDIR %_OUT_DIR%
CD %_OUT_DIR%
MKDIR %_OBJ_DIR%
CD ..

:COMPILE_DLL
REM Compiling DLL without Lua Stand-Alone interpreter
CALL :COMPILE_BASE
CALL :COMPILE_SOCKLIB
CALL :COMPILE_SOCKET
CALL :_LINK_DLL

REM Coplining Lua Stand-Alone interpreter
CALL :_LINK_INTER

ECHO Complining ended!
GOTO :EOF


REM Base compiler function
:COMPILE_BASE
IF %FLAG_COMPILE_BASE% == 0 (
  GOTO :EOF
)

CD %_OBJ_PATH%

%LUA_SERVER_COMPILE% /DLUA_BUILD_AS_DLL %_LUA_SRCDIR%/l*.c

CD %_OBJ_RETPATH%
GOTO :EOF


REM Socket compiler function
:COMPILE_SOCKET
IF %FLAG_COMPILE_SOCKET% == 0 (
  GOTO :EOF
)

CD %_OBJ_PATH%

%LUA_SERVER_COMPILE% %_SOCKET_SRCDIR%/*.cpp

CD %_OBJ_RETPATH%
GOTO :EOF


REM Socklib compiler function
:COMPILE_SOCKLIB
IF %FLAG_COMPILE_SOCKLIB% == 0 (
  GOTO :EOF
)

CD %_OBJ_PATH%

%LUA_SERVER_COMPILE% /DLUA_BUILD_AS_DLL %_LUA_SRCDIR%/lsocklib.cpp

CD %_OBJ_RETPATH%
GOTO :EOF


REM DLL Link function
:_LINK_DLL
IF %FLAG_NO_LINK% == 1 (
  GOTO :EOF
)

CD %_OBJ_PATH%
REN lua.obj lua.obj.~

CD %_OBJ_RETPATH%
CD %_OUT_PATH%

%LUA_SERVER_LINK% /DLL /out:lua51.dll %_OBJ_DIR%/*.obj
IF %FLAG_NO_MT% == 0 (
  IF EXIST lua51.dll.manifest (
    %LUA_SERVER_MT% -manifest lua51.dll.manifest -outputresource:lua51.dll;2
  )
)

CD %_OUT_RETPATH%

CD %_OBJ_PATH%
REN lua.obj.~ lua.obj

CD %_OBJ_RETPATH%
GOTO :EOF


REM Interpreter Link function
:_LINK_INTER
IF %FLAG_NO_LINK% == 1 (
  GOTO :EOF
)

CD %_OUT_PATH%

%LUA_SERVER_LINK% /out:lua.exe %_OBJ_DIR%/lua.obj lua51.lib
IF %FLAG_NO_MT% == 0 (
  IF EXIST lua.exe.manifest (
    %LUA_SERVER_MT% -manifest lua.exe.manifest -outputresource:lua.exe
  )
)

CD %_OUT_RETPATH%
GOTO :EOF

ENDLOCAL

REM Usage printer function
:PRINT_USAGE
ECHO Usage: %FILE_NAME% [flags]
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