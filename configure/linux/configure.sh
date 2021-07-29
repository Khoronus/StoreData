#!/usr/bin/env bash

# Set the variable values with the default values
export path_opencv_tmp=/home/moro/opencv_build/opencv/build
export path_boost_tmp=/home/moro/local/boost
export path_boost_lib_tmp=/home/moro/local/boost/stage/lib

# https://serverfault.com/questions/7503/how-to-determine-if-a-bash-variable-is-empty

echo set opencv path [enter to set:$path_opencv_tmp]
read -p "Enter path: " path_opencv_t
if ! [ -z "${path_opencv_t}" ]; then 
  path_opencv_tmp=$path_opencv_t; 
fi

echo set boost path [enter to set:$path_boost_tmp]
read -p "Enter path: " path_boost_t
if ! [ -z "${path_boost_t}" ]; then 
  path_boost_tmp=$path_boost_t; 
fi

echo set boost lib path [enter to set:$path_boost_lib_tmp]
read -p "Enter path: " path_boost_lib_t
if ! [ -z "${path_boost_lib_t}" ]; then 
  path_boost_lib_tmp=$path_boost_lib_t; 
fi

# Saved in path_configuration.txt
echo path_opencv $path_opencv_tmp> ../path_configuration.txt
echo path_boost $path_boost_tmp>> ../path_configuration.txt
echo path_boost_lib $path_boost_lib_tmp>> ../path_configuration.txt

