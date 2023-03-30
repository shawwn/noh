@echo off
call :initialize_vars

rem Read source version
for /f "tokens=2-3" %%i in (..\shell\shell_common.h) do call :parse_source %%i %%j

rem Read local version info
if not exist ..\buildnumber.txt call :nofile
if not exist ..\buildnumber.txt goto error_nofile
for /f "tokens=1-7" %%i in (..\buildnumber.txt) do call :parse_buildversion %%i %%j %%k %%l %%m %%n %%o
if %found_id%==0 call :no_entry

rem Check for a version number change
for /f "tokens=1-7" %%i in (..\buildnumber.txt) do call :check_other_user %%i %%j %%k %%l %%m %%n %%o
if %out_of_date_source%==1 goto :eof

if %found_id%==1 set /a local_buildnumber+=1
if %no_file%==1 set local_buildnumber=0
if %source_majorversion% GTR %local_majorversion% set local_buildnumber=0
if %source_minorversion% GTR %local_minorversion% set local_buildnumber=0
if %source_microversion% GTR %local_microversion% set local_buildnumber=0
if %source_hotfixversion% GTR %local_hotfixversion% set local_buildnumber=0
call :update_record

rem Set the number in the source
echo #define BUILDNUMBER	_T("%local_buildnumber%") > ..\shell\buildnumber.h
goto :eof

:initialize_vars
set source_majorversion=0
set source_minorversion=0
set source_microversion=0
set source_hotfixversion=0
set found_id=0
set no_file=0
set out_of_date_source=0
set local_majorversion=0
set local_minorversion=0
set local_microversion=0
set local_hotfixversion=0
set local_buildnumber=0

if exist ~temp del ~temp
cd > ~temp
set /p srcdir= < ~temp
if exist ~temp del ~temp
goto :eof

:parse_source
if %1foo==foo goto :eof
if %1==MAJOR_VERSION set source_majorversion=%2
if %1==MINOR_VERSION set source_minorversion=%2
if %1==MICRO_VERSION set source_microversion=%2
if %1==HOTFIX_VERSION set source_hotfixversion=%2
goto :eof

:parse_buildversion
if %1foo==foo goto :eof
if %2foo==foo goto :eof
if not "%1 %2"=="%userdomain% %username%" goto :eof
set found_id=1
set local_majorversion=%3
set local_minorversion=%4
set local_microversion=%5
set local_hotfixversion=%6
set local_buildnumber=%7
goto :eof

:check_other_user
if %out_of_date_source%==1 goto :eof
if %1foo==foo goto :eof
if %2foo==foo goto :eof
if "%1 %2"=="%userdomain% %username%" goto :eof
if %source_majorversion% GTR %3 goto version_ok
if %source_majorversion% LSS %3 goto version_bad
if %source_minorversion% GTR %4 goto version_ok
if %source_minorversion% LSS %4 goto version_bad
if %source_microversion% GTR %5 goto version_ok
if %source_microversion% LSS %5 goto version_bad
if %source_hotfixversion% GTR %6 goto version_ok
if %source_hotfixversion% LSS %6 goto version_bad
goto version_ok
:version_bad
set K2_BUILDNUMBER="OUT OF DATE SOURCE"
set out_of_date_source=1
goto :eof
:version_ok
if %local_majorversion% GTR %3 goto :eof
if %local_majorversion% LSS %3 goto out_of_date
if %local_minorversion% GTR %4 goto :eof
if %local_minorversion% LSS %4 goto out_of_date
if %local_microversion% GTR %5 goto :eof
if %local_microversion% LSS %5 goto out_of_date
if %local_hotfixversion% GTR %6 goto :eof
if %local_hotfixversion% LSS %6 goto out_of_date
if %local_buildnumber% LSS %7 set local_buildnumber=%7
goto :eof
:out_of_date
set local_majorversion=%3
set local_minorversion=%4
set local_microversion=%5
set local_hotfixversion=%6
set local_buildnumber=%7
goto :eof

:update_record
copy ..\buildnumber.txt ..\buildnumber.old > nul
del ..\buildnumber.txt
for /f "tokens=1-7" %%i in (..\buildnumber.old) do call :compare_record %%i %%j %%k %%l %%m %%n %%o
del ..\buildnumber.old
goto :eof

:compare_record
if %1foo==foo goto :eof
if %2foo==foo goto :eof
if not "%1 %2"=="%userdomain% %username%" echo %1 %2 %3 %4 %5 %6 %7 >> ..\buildnumber.txt
if not "%1 %2"=="%userdomain% %username%" echo. >> ..\buildnumber.txt
if "%1 %2"=="%userdomain% %username%" call :write_entry %source_majorversion% %source_minorversion% %source_microversion% %source_hotfixversion% %local_buildnumber%
goto :eof

:nofile
set no_file=1
call :write_entry %source_majorversion% %source_minorversion% %source_microversion% %source_hotfixversion% 0
goto :eof

:no_entry
set local_majorversion=%source_majorversion%
set local_minorversion=%source_minorversion%
set local_microversion=%source_microversion%
set local_hotfixversion=%source_hotfixversion%
set local_buildnumber=0
call :write_entry %source_majorversion% %source_minorversion% %source_microversion% %source_hotfixversion% 0
goto :eof

:error_nofile
echo Could not find or create file "buildversion"
goto :eof

:write_entry
echo %userdomain% %username% %1 %2 %3 %4 %5 >> ..\buildnumber.txt
echo. >> ..\buildnumber.txt
goto :eof
