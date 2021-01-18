@echo off 
cd /d %~dp0
if not exist build mkdir build
cd build
cmake .. -G"Visual Studio 16 2019"
