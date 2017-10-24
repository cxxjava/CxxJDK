@echo off

FOR /L %%G IN (10,1,%1) DO (
		cl /? 2>&1 | findstr /C:" %%G." > nul && goto %%G
	)
EXIT /B 0
:8
EXIT /B 8
:9
EXIT /B 9
:10
EXIT /B 10
:11
EXIT /B 11
:12
EXIT /B 12
:13
EXIT /B 13
:14
EXIT /B 14
:15
EXIT /B 15
:16
EXIT /B 16
:17
EXIT /B 17
:18
EXIT /B 18
:19
EXIT /B 19
:20
EXIT /B 20
:21
EXIT /B 21
:22
EXIT /B 22
:23
EXIT /B 23
:24
EXIT /B 24
