@echo off
mkdir jar > nul 2>&1
del /s/q jar\*.class > nul 2>&1
javac -cp src -d jar src/com/amivoice/hrp/HrpListener.java
if errorlevel 1 goto _exit
javac -cp src -d jar src/com/amivoice/hrp/Hrp.java
if errorlevel 1 goto _exit
javac -cp src -d jar src/com/amivoice/hrp/Hrp_.java
if errorlevel 1 goto _exit
javac -cp src -d jar src/com/amivoice/hrp/Hrp__.java
if errorlevel 1 goto _exit
jar cf Hrp.jar -C jar .
if errorlevel 1 goto _exit
javac -cp Hrp.jar -encoding utf-8 *.java
goto _exit
:_exit
