@rem Set up the standard stuff for Microsoft Visual Studio 2005 for flykit2D
@rem by Reza Ardekani (re.ardekani@gmail.com) = 05/01/2014
call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\VCVARS32.BAT" 

set EXTERNAL_EXPORTS=D:\flykit2D\external_exports

set PATH=%EXTERNAL_EXPORTS%\opencv200\bin\32bits;%EXTERNAL_EXPORTS%\opencv200\bin\64bits;%PATH%
"C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"

rem @devenv /USEENV

