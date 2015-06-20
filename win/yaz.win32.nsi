; YAZ for Windows 32 bit, VS 2015
!define VS_REDIST_EXE "vcredist_x86.exe"
!define VS_REDIST_FULL "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\1033\${VS_REDIST_EXE}"
!define VS_REDIST_KEY "SOFTWARE\Classes\Installer\Products\21EE4A31AE32173319EEFE3BD6FDFFE3"

InstallDir "$PROGRAMFILES\YAZ"

!include yaz.nsi

