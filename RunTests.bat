@echo off

set testsDir=tests
set outputFile=%testsDir%\output

:: compile project
mingw32-make default

:: clear output file
<nul (set/p z=) >%outputFile%

:: run tests
for /r %%f in (%testsDir%\*_) do (
    (echo Test file "%%f": &echo.)>>%outputFile%
    (farach_suftree.exe %%f) >>%outputFile%
    (echo.)>>%outputFile%
    (echo "================================================================================================================" &echo.)>>%outputFile% )

echo Done
