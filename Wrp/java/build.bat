@echo off
mkdir jar > nul 2>&1
del /s/q jar\*.class > nul 2>&1
javac -cp src -d jar src/com/amivoice/wrp/WrpListener.java
if errorlevel 1 goto _exit
javac -cp src -d jar src/com/amivoice/wrp/Wrp.java
if errorlevel 1 goto _exit
javac -cp src -d jar src/com/amivoice/wrp/Wrp_.java
if errorlevel 1 goto _exit
if exist ../../_/tyrus-standalone-client-1.15.jar (
  javac -cp src;../../_/tyrus-standalone-client-1.15.jar -d jar src/com/amivoice/wrp/Wrp__.java
  if errorlevel 1 goto _exit
  javac -cp src;../../_/tyrus-standalone-client-1.15.jar -d jar src/com/amivoice/wrp/Wrp___.java
  if errorlevel 1 goto _exit
)
jar cf Wrp.jar -C jar .
if errorlevel 1 goto _exit
javac -cp Wrp.jar -encoding utf-8 *.java
goto _exit
:_exit
