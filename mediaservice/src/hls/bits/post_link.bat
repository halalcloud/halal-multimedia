set platform=%1%
set config=%2%
xcopy .\%config%\*.dll ..\..\..\lib\%platform%\%config%\ /y
xcopy .\%config%\*.lib ..\..\..\lib\%platform%\%config%\ /y

