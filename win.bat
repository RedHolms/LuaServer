@echo !!!!!!!!!!!!!!!!!!!!!!!!
@echo Before compiling make sure Microsoft C Compiler, linker and Windows Kits is in your path. Else, use a Visual Studio developer console
@echo !!!!!!!!!!!!!!!!!!!!!!!!
@pause

@set MYCOMPILE=cl /nologo /MD /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
@set MYLINK=link /nologo
@set MYMT=mt /nologo

@mkdir out
@cd out

@echo Compiling... Please, wait
@echo Binaries will be stored in "out" directory

@rem Compiling DLL without Lua Stand-Alone interpreter
@%MYCOMPILE% /DLUA_BUILD_AS_DLL ../src/lua/l*.c
@%MYCOMPILE% ../src/socket/*.cpp
@%MYCOMPILE% /DLUA_BUILD_AS_DLL ../src/lua/l*.cpp
@del lua.obj
@%MYLINK% /DLL /out:lua51.dll *.obj
@if exist lua51.dll.manifest^
  @%MYMT% -manifest lua51.dll.manifest -outputresource:lua51.dll;2

@rem Coplining Lua Stand-Alone interpreter
@%MYCOMPILE% /DLUA_BUILD_AS_DLL ../src/lua/lua.c
@%MYLINK% /out:lua.exe lua.obj lua51.lib
@if exist lua.exe.manifest^
  @%MYMT% -manifest lua.exe.manifest -outputresource:lua.exe
@del *.obj *.manifest

@cd ..

@echo Complining ended!