@echo off
rem "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
rem "C:\Program Files\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"

echo ===============================
echo make win eso8.lib
echo ===============================
rem ..\..\tools\bmake.exe -f Makefile_win all
nmake.exe -f Makefile_win all

echo ===============================
echo make arm libeso_l.a
echo ===============================
rem ..\..\tools\bmake.exe -DLIBNAME=libeso_l.a -DENDIANMODE=littleend -f Makefile_arm all
nmake.exe -DLIBNAME=libeso_l.a -DENDIANMODE=littleend -f Makefile_arm all

pause
