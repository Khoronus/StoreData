REM Set the variable values with the default values
set path_opencv_tmp=C:/src/opencv/build
set path_boost_tmp=C:/local/boost
set path_boost_lib_tmp=C:/local/boost/lib/x64/lib

@echo off
ECHO set opencv path (i.e. %path_opencv_tmp%)
set /p path_opencv_tmp="Enter path: "
ECHO set boost path (i.e. %path_boost_tmp%)
set /p path_boost_tmp="Enter path: "
ECHO set boost_lib path (i.e. %path_boost_lib_tmp%)
set /p path_boost_lib_tmp="Enter path: "

REM Saved in path_configuration.txt
@echo path_opencv %path_opencv_tmp:\=/%> ../path_configuration.txt
@echo path_boost %path_boost_tmp:\=/%>> ../path_configuration.txt
@echo path_boost_lib %path_boost_lib_tmp:\=/%>> ../path_configuration.txt
