@echo off
setlocal

call :initialize_vars

rem Read source version
for /f "tokens=2-3" %%i in (Heroes~1\shell_common.h) do call :parse_source %%i %%j
for /f "tokens=2-3" %%i in (shell\buildnumber.h) do call :parse_source %%i %%j
for /f "tokens=1-2" %%i in (Heroes~1\k2_settings.h) do call :parse_branch %%i %%j
set buildnumber=%buildnumber:_T("=%
set buildnumber=%buildnumber:")=%
set honversion=%source_majorversion%.%source_minorversion%.%source_microversion%.%source_hotfixversion%-%buildnumber%
if %is_server%==1 set honversion=%honversion%_s
if %is_client%==1 set honversion=%honversion%_c
rem Move to working directory
if not exist ..\archive mkdir ..\archive
cd ..\archive

rem Copy source files
if exist src rmdir /s /q src
if not exist src mkdir src
cd src

mkdir k2
xcopy /s "%srcdir%\k2\*.cpp" k2
xcopy /s "%srcdir%\k2\*.h" k2

if %is_server%==1 goto skip_vid
mkdir vid_d3d9
xcopy /s "%srcdir%\vid_d3d9\*.cpp" vid_d3d9
xcopy /s "%srcdir%\vid_d3d9\*.h" vid_d3d9
:skip_vid

mkdir "Heroes of Newerth_shell"
xcopy /s "%srcdir%\Heroes of Newerth_shell\*.cpp" "Heroes of Newerth_shell"
xcopy /s "%srcdir%\Heroes of Newerth_shell\*.h" "Heroes of Newerth_shell"

mkdir "shell"
xcopy /s "%srcdir%\shell\*.cpp" "shell"
xcopy /s "%srcdir%\shell\*.h" "shell"

if %is_server%==1 goto skip_client
mkdir "hon_client"
xcopy /s "%srcdir%\hon_client\*.cpp" hon_client
xcopy /s "%srcdir%\hon_client\*.h" hon_client
:skip_client

mkdir "hon_server"
xcopy /s "%srcdir%\hon_server\*.cpp" hon_server
xcopy /s "%srcdir%\hon_server\*.h" hon_server

mkdir "hon_shared"
xcopy /s "%srcdir%\hon_shared\*.cpp" hon_shared
xcopy /s "%srcdir%\hon_shared\*.h" hon_shared

if %is_server%==1 goto skip_tools
if %is_client%==1 goto skip_tools
mkdir "editor"
xcopy /s "%srcdir%\editor\*.cpp" editor
xcopy /s "%srcdir%\editor\*.h" editor
mkdir "modelviewer"
xcopy /s "%srcdir%\modelviewer\*.cpp" modelviewer
xcopy /s "%srcdir%\modelviewer\*.h" modelviewer
:skip_tools
cd ..

rem Copy pdbs
if exist pdb rmdir /s /q pdb
if not exist pdb mkdir pdb
cd pdb
xcopy "%srcdir%\k2\release\k2.pdb"
if %is_server%==0 xcopy "%srcdir%\vid_d3d9\release\vid_d3d9.pdb"
xcopy "%srcdir%\Heroes of Newerth_shell\release\hon.pdb"
mkdir game
if %is_server%==0 xcopy "%srcdir%\hon_client\release\cgame.pdb" game
xcopy "%srcdir%\hon_server\release\game.pdb" game
xcopy "%srcdir%\hon_shared\release\game_shared.pdb" game
if %is_server%==1 goto skip_tool_pdb
if %is_client%==1 goto skip_tool_pdb
mkdir editor
xcopy "%srcdir%\editor\release\cgame.pdb" editor
mkdir modelviewer
xcopy "%srcdir%\modelviewer\release\cgame.pdb" modelviewer
:skip_tool_pdb
cd ..

rem Copy binaries and game settings
if exist bin rmdir /s /q bin
if not exist bin mkdir bin
cd bin
xcopy "..\..\Heroes of Newerth\hon.exe"
xcopy "..\..\Heroes of Newerth\k2.dll"
if %is_server%==0 xcopy "..\..\Heroes of Newerth\vid_d3d9.dll"
mkdir game
if %is_server%==0 xcopy "..\..\Heroes of Newerth\game\cgame.dll" game
xcopy "..\..\Heroes of Newerth\game\game.dll" game
xcopy "..\..\Heroes of Newerth\game\game_shared.dll" game
if %is_server%==1 goto skip_tool_bin
if %is_client%==1 goto skip_tool_bin
mkdir editor
xcopy "..\..\Heroes of Newerth\editor\cgame.dll" editor
mkdir modelviewer
xcopy "..\..\Heroes of Newerth\modelviewer\cgame.dll" modelviewer
:skip_tool_bin
cd ..

"%srcdir%\7z" a -tzip -mx=9 -x!*.zip "hon-%honversion%.zip" *

rem cleanup
rmdir /s /q src
rmdir /s /q bin
rmdir /s /q pdb

call :ftp_send

cd "%srcdir%"
goto :eof

:initialize_vars
set ftpserver=kongor
set source_majorversion=0
set source_minorversion=0
set source_microversion=0
set source_hotfixversion=0
set is_server=0
set is_client=0
set buildnumber=0
set honversion=unknown
set /p ftpserver=Local server name (just press enter if remote): 
set /p archiveftppassword=FTP password: 
set srcdir=%cd%
goto :eof

:parse_source
if %1foo==foo goto :eof
if %1==MAJOR_VERSION set source_majorversion=%2
if %1==MINOR_VERSION set source_minorversion=%2
if %1==MICRO_VERSION set source_microversion=%2
if %1==HOTFIX_VERSION set source_hotfixversion=%2
if %1==BUILDNUMBER set buildnumber=%2
goto :eof

:parse_branch
echo %1 %2
if %1foo==foo goto :eof
if %2foo==foo goto :eof
if not %1==#define goto :eof
if %2==K2_SERVER set is_server=1
if %2==K2_CLIENT set is_client=1
goto :eof

:ftp_send
echo open %ftpserver% 27182> ~~~~
echo buildarchive>> ~~~~
echo %archiveftppassword%>> ~~~~
echo binary>> ~~~~
echo put hon-%honversion%.zip>> ~~~~
echo quit>> ~~~~
ftp -s:~~~~
del ~~~~
goto :eof
