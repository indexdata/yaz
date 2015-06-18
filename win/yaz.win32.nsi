; YAZ for Windows 32 bit, VS 2013
!define VS_REDIST_EXE "vcredist_x86.exe"
!define VS_REDIST_FULL "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\1033\${VS_REDIST_EXE}"
!define VS_REDIST_KEY "SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x86"

InstallDir "$PROGRAMFILES\YAZ"

!include yaz.nsi

