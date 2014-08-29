@echo off

if exist "%~dp0sign-release.cmd" call "%~dp0sign-release.cmd"

cd /d "%~dp0"

if "%~1"=="" echo No parm & goto :EOF
if exist "..\_Arc\far3.%~1.x86.x64.7z" echo far3.%~1.x86.x64.7z already exists & goto :EOF
if not exist ..\_Arc md ..\_Arc

if exist ..\bis_release rd /q /s "%~dp0..\bis_release"

md ..\bis_release
md ..\bis_release\x86
md ..\bis_release\x64

copy Include\plugin.hpp ..\bis_release\

copy Release.32.vc\far.exe ..\bis_release\x86\Far.exe
copy Release.32.vc\Far.exe.example.ini ..\bis_release\x86\Far.exe.example.ini
copy Release.32.vc\far.map ..\bis_release\x86\Far.map
copy Release.32.vc\*.lng ..\bis_release\x86\
copy Release.32.vc\*.hlf ..\bis_release\x86\
copy Release.32.vc\File_id.diz ..\bis_release\x86\
copy bis_changelog ..\bis_release\x86\
copy changelog ..\bis_release\x86\

copy Release.64.vc\far.exe ..\bis_release\x64\Far.exe
copy Release.64.vc\Far.exe.example.ini ..\bis_release\x64\Far.exe.example.ini
copy Release.64.vc\far.map ..\bis_release\x64\Far.map
copy Release.64.vc\*.lng ..\bis_release\x64\
copy Release.64.vc\*.hlf ..\bis_release\x64\
copy Release.64.vc\File_id.diz ..\bis_release\x64\
copy bis_changelog ..\bis_release\x64\
copy changelog ..\bis_release\x64\

cd ..\bis_release

7z a "..\_Arc\far3.%~1.x86.x64.7z" -r *

cd "%~dp0"
rd /q /s "%~dp0..\bis_release"

7z a "..\_Arc\far3.%~1.x86.x64.dbg.7z" Release.32.vc\far.pdb Release.64.vc\far.pdb

cd "%~dp0"
7z a -r "..\_Arc\far3.%~1.x86.x64.src.7z" *.cpp *.c *.h *.hpp *.vcxproj *.sln *.rc *.asm *.def *.m4 *.pas *.sh *.txt
