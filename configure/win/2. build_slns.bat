REM location is set with the current called batch location. If different, please change
SET "location=%cd%"
REM SET "build=C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild.exe"
SET "build=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\msbuild.exe"
SET "params_debug=/t:build /m:8 /p:RunCodeAnalysis=false;BuildInParallel=True /v:n /property:Configuration=Debug /property:Platform=x64"
SET "params_release=/t:build /m:8 /p:RunCodeAnalysis=false;BuildInParallel=True /v:n /property:Configuration=Release /property:Platform=x64"

REM Create a log build file
@echo "Projects build result [==0 success, >0 error]" > "build_result.txt"

REM C++ Libraries
call :build_function "%build%" "%location%\..\..\build\StoreData.sln" "%params_debug%" "build_result.txt"
call :build_function "%build%" "%location%\..\..\build\INSTALL.vcxproj" "%params_debug%" "build_result.txt"
call :build_function "%build%" "%location%\..\..\build\StoreData.sln" "%params_release%" "build_result.txt"
call :build_function "%build%" "%location%\..\..\build\INSTALL.vcxproj" "%params_release%" "build_result.txt"

REM Skip the function
goto:eof

REM function to build a process
:build_function
"%~1" %~2 %~3
cd "%location%
echo "%~2" Exit Code is %errorlevel% >> %~4
goto:eof


