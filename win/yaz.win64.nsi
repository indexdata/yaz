; YAZ for Windows 64 bit, VS 2015
!define VS_REDIST_EXE "vcredist_x64.exe"
!define VS_REDIST_FULL "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\1033\${VS_REDIST_EXE}"
!define VS_REDIST_KEY "SOFTWARE\Classes\Installer\Products\75B815F0A80081D379E08346B5DB5B6E"

InstallDir "$PROGRAMFILES64\YAZ"

!include yaz.nsi


