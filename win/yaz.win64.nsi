; YAZ for Windows 64 bit, VS 2013
!define VS_REDIST_EXE "vcredist_x64.exe"
!define VS_REDIST_FULL "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\1033\${VS_REDIST_EXE}"
!define VS_REDIST_KEY "SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x64"

InstallDir "$PROGRAMFILES64\YAZ"

!include yaz.nsi

