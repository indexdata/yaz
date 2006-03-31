; $Id: yaz.nsi,v 1.78 2006-03-31 12:49:44 adam Exp $

!define VERSION "2.1.17"
!define VS_RUNTIME_LOCATION "c:\Program Files\Microsoft Visual Studio .NET 2003\SDK\v1.1\Bin"

!include "MUI.nsh"

SetCompressor bzip2

Name "YAZ"
Caption "Index Data YAZ ${VERSION} Setup"
OutFile "yaz_${VERSION}.exe"

LicenseText "You must read the following license before installing:"
LicenseData license.txt

ComponentText "This will install the YAZ Toolkit on your computer:"
InstType "Full (w/ Source)"
InstType "Lite (w/o Source)"

InstallDir "$PROGRAMFILES\YAZ"
InstallDirRegKey HKLM "SOFTWARE\Index Data\YAZ" ""


;----------------------------
; Pages


  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
; Page components
; Page directory
; Page instfiles

; UninstPage uninstConfirm
; UninstPage instfiles

;--------------------------------
;Languages
 
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

Section "" ; (default section)
	SetOutPath "$INSTDIR"
	; add files / whatever that need to be installed here.
	WriteRegStr HKLM "SOFTWARE\Index Data\YAZ" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\YAZ" "DisplayName" "YAZ ${VERSION} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\YAZ" "UninstallString" '"$INSTDIR\uninst.exe"'
	; write out uninstaller
	WriteUninstaller "$INSTDIR\uninst.exe"
	SetOutPath $SMPROGRAMS\YAZ
 	CreateShortCut "$SMPROGRAMS\YAZ\YAZ Program Directory.lnk" \
                 "$INSTDIR"
	WriteINIStr "$SMPROGRAMS\YAZ\YAZ Home page.url" \
              "InternetShortcut" "URL" "http://www.indexdata.dk/yaz/"
	CreateShortCut "$SMPROGRAMS\YAZ\Uninstall YAZ.lnk" \
		"$INSTDIR\uninst.exe"
	SetOutPath $INSTDIR
	File LICENSE.txt
	File ..\README
	File ..\NEWS
	SetOutPath $INSTDIR
	SetOutPath $INSTDIR\ztest
	File ..\ztest\dummy-records
	File ..\ztest\dummy-grs
	File ..\ztest\dummy-words
	SetOutPath $INSTDIR\etc
	File ..\etc\*.xml
	File ..\etc\*.xsl
	File ..\etc\pqf.properties

SectionEnd ; end of default section

Section "YAZ Runtime" YAZ_Runtime
	SectionIn 1 2
	IfFileExists "$INSTDIR\bin\yaz-ztest.exe" 0 Noservice
	ExecWait '"$INSTDIR\bin\yaz-ztest.exe" -remove'
Noservice:
	SetOutPath $INSTDIR\bin
	File "${VS_RUNTIME_LOCATION}\msvcr71.dll"
	File ..\bin\iconv.dll
	File ..\bin\zlib1.dll
	File ..\bin\libxml2.dll
	File ..\bin\yaz.dll
	File ..\bin\*.exe
	SetOutPath $SMPROGRAMS\YAZ
 	CreateShortCut "$SMPROGRAMS\YAZ\YAZ Client.lnk" \
                 "$INSTDIR\bin\yaz-client.exe"
	SetOutPath $SMPROGRAMS\YAZ\Server
 	CreateShortCut "$SMPROGRAMS\YAZ\Server\Server on console on port 9999.lnk" \
                 "$INSTDIR\bin\yaz-ztest.exe" '-w"$INSTDIR\ztest"'
  	CreateShortCut "$SMPROGRAMS\YAZ\Server\Install Z39.50 service on port 210.lnk" \
                  "$INSTDIR\bin\yaz-ztest.exe" '-installa tcp:@:210'
 	CreateShortCut "$SMPROGRAMS\YAZ\Server\Remove Z39.50 service.lnk" \
                 "$INSTDIR\bin\yaz-ztest.exe" '-remove'
SectionEnd

Section "YAZ Development" YAZ_Development
	SectionIn 1 2
	SetOutPath $INSTDIR\include\yaz
	File ..\include\yaz\*.h
	SetOutPath $INSTDIR\lib
	File ..\lib\yaz.lib
	File ..\lib\yazd.lib
	SetOutPath $INSTDIR\bin
	File ..\bin\yazd.dll
SectionEnd

Section "YAZ Documentation" YAZ_Documentation
	SectionIn 1 2
	SetOutPath $INSTDIR\doc
	File ..\doc\*.html
	File ..\doc\*.png
	File ..\doc\*.pdf
	File ..\doc\*.xml
	File ..\doc\*.in
	File ..\doc\*.dsl
	File ..\doc\*.xsl
	File ..\doc\*.css
	File ..\doc\*.ent
	SetOutPath $SMPROGRAMS\YAZ
	CreateShortCut "$SMPROGRAMS\YAZ\HTML Documentation.lnk" \
                 "$INSTDIR\doc\yaz.html"
	CreateShortCut "$SMPROGRAMS\YAZ\PDF Documentaion.lnk" \
                 "$INSTDIR\doc\yaz.pdf"
SectionEnd

Section "YAZ Source" YAZ_Source
	SectionIn 1
	SetOutPath $INSTDIR\util
	File ..\util\*.c
	File ..\util\yaz-asncomp
	SetOutPath $INSTDIR\src
	File ..\src\*.c
	File ..\src\*.h
	File ..\src\*.y
	File ..\src\*.tcl
	File ..\src\*.csv
	File ..\src\*.asn
	File ..\src\codetables.xml
	SetOutPath $INSTDIR\test
	File ..\test\marc*.*
	File ..\test\*.c
	File ..\test\*.sh
	File ..\test\*.xml
	File ..\test\*.asn
	SetOutPath $INSTDIR\zoom
	File ..\zoom\*.c
	SetOutPath $INSTDIR\ztest
	File ..\ztest\*.c
	SetOutPath $INSTDIR\client
	File ..\client\*.c
	File ..\client\*.h
	SetOutPath $INSTDIR\win
	File makefile
	File *.nsi
	File *.rc
	File *.h
SectionEnd

; begin uninstall settings/section
UninstallText "This will uninstall YAZ ${VERSION} from your system"

Section Uninstall
; add delete commands to delete whatever files/registry keys/etc you installed here.
	Delete "$INSTDIR\uninst.exe"
	DeleteRegKey HKLM "SOFTWARE\Index Data\YAZ"
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\YAZ"
	ExecWait '"$INSTDIR\bin\yaz-ztest" -remove'
	RMDir /r $SMPROGRAMS\YAZ
	RMDir /r $INSTDIR
        IfFileExists $INSTDIR 0 Removed 
		MessageBox MB_OK|MB_ICONEXCLAMATION \
                 "Note: $INSTDIR could not be removed."
Removed:
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
LangString DESC_YAZ_Runtime ${LANG_ENGLISH} "YAZ runtime files needed in order for YAZ to run, such as DLLs."
LangString DESC_YAZ_Development ${LANG_ENGLISH} "Header files and import libraries required for developing software using YAZ."
LangString DESC_YAZ_Documentation ${LANG_ENGLISH} "YAZ Users' guide and reference in HTML and PDF. Describes both YAZ applications and the API."
LangString DESC_YAZ_Source ${LANG_ENGLISH} "Source code of YAZ. Required if you need to rebuild YAZ (for debugging purposes)."

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Runtime} $(DESC_YAZ_Runtime)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Development} $(DESC_YAZ_Development)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Documentation} $(DESC_YAZ_Documentation)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Source} $(DESC_YAZ_Source)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; eof
