@echo off
PATH=c:\MinGW\bin\
:CHECK
cls
echo ***********************************************************************
echo *                    ___________________________________              *
echo ***                   Build MENU for GT-10FxFloorBoard              ***
echo *                    -----------------------------------              *
echo *                      Press [1] to "Compile"                         *
echo *                               or                                    *
echo *                      Press [2] to "Compile and Run"                 *
echo *                               or                                    *
echo *                      Press [3] for "Run only"                       *
echo *                               or                                    *
echo *                      Press [4] for "EXIT"                           *
echo *                               or                                	   *
echo *                      Press [5] to "Generate translation ts scripts" *
echo *                               or                                    *
echo *                      Press [6] to "Generated translated qm source"  *
echo ***********************************************************************
echo .
choice /c:123456  Choose an option
if errorlevel 6 goto F-OPTION
if errorlevel 5 goto E-OPTION
if errorlevel 4 goto D-OPTION
if errorlevel 3 goto C-OPTION
if errorlevel 2 goto B-OPTION
if errorlevel 1 goto A-OPTION

:A-OPTION
c:\qt\4.6.0\bin\qmake
c:\MinGW\bin\mingw32-make 
pause
goto CHECK

:B-OPTION
c:\qt\4.6.0\bin\qmake
c:\MinGW\bin\mingw32-make 
cd packager
GT-10FxFloorBoard.exe
cd ..
goto CHECK

:C-OPTION
cd packager
GT-10FxFloorBoard.exe
cd ..
goto CHECK

:D-OPTION
goto END

:E-OPTION
c:\qt\4.6.0\bin\lupdate.exe GT-10FxFloorBoard.pro
pause
goto CHECK

:F-OPTION
c:\qt\4.6.0\bin\lrelease.exe GT-10FxFloorBoard.pro
pause
goto CHECK

:END
echo .
echo the end......................Colin`s Menu ..........................
exit

