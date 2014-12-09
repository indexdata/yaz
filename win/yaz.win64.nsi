; YAZ for Windows 64 bit, VS 2013
!define VS_RUNTIME_DLL      "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x64\Microsoft.VC120.CRT\msvc*.dll"
!define VS_RUNTIME_MANIFEST ""

InstallDir "$PROGRAMFILES64\YAZ"

!include yaz.nsi

