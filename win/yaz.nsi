; $Id: yaz.nsi,v 1.6 2002-03-25 15:12:25 adam Exp $

!define VERSION "1.8.7"

Name "YAZ"
Caption "Index Data YAZ ${VERSION} Setup"
OutFile "yaz_${VERSION}.exe"

LicenseText "You must read the following license before installing:"
LicenseData license.txt

ComponentText "This will install the YAZ Toolkit on your computer:"
InstType "Full (w/ Source)"
InstType "Lite (w/o Source)"

; Some default compiler settings (uncomment and change at will):
; SetCompress auto ; (can be off or force)
; SetDatablockOptimize on ; (can be off)
; CRCCheck on ; (can be off)
; AutoCloseWindow false ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
; SetDateSave off ; (can be on to have files restored to their orginal date)

InstallDir "$PROGRAMFILES\YAZ"
InstallDirRegKey HKLM "SOFTWARE\Index Data\YAZ" ""
DirShow show ; (make this hide to not let the user change it)
DirText "Select the directory to install YAZ in:"

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
	SetOutPath $INSTDIR
	File /r ..\tab
	SetOutPath $INSTDIR\ztest
	File ..\ztest\dummy-records
	File ..\ztest\dummy-grs
	File ..\ztest\dummy-words
SectionEnd ; end of default section

Section "YAZ Runtime"
	SectionIn 12
	IfFileExists "$INSTDIR\bin\yaz-ztest.exe" 0 Noservice
	ExecWait '"$INSTDIR\bin\yaz-ztest.exe" -remove'
Noservice:
	SetOutPath $INSTDIR\bin
	File ..\bin\*.exe
	File ..\bin\*.dll
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

Section "YAZ Development"
	SectionIn 12
	SetOutPath $INSTDIR\include\yaz
	File ..\include\yaz\*.h
	SetOutPath $INSTDIR\lib
	File ..\lib\*.lib
SectionEnd

Section "YAZ Documentation"
	SectionIn 12
	SetOutPath $INSTDIR
	File /r ..\doc
	SetOutPath $SMPROGRAMS\YAZ
	CreateShortCut "$SMPROGRAMS\YAZ\HTML Documentation.lnk" \
                 "$INSTDIR\doc\yaz.html"
	CreateShortCut "$SMPROGRAMS\YAZ\PDF Documentaion.lnk" \
                 "$INSTDIR\doc\yaz.pdf"
SectionEnd

Section "YAZ Source"
	SectionIn 1
	SetOutPath $INSTDIR\util
	File ..\util\*.c
	File ..\util\*.tcl
	SetOutPath $INSTDIR\odr
	File ..\odr\*.c
	SetOutPath $INSTDIR\z39.50
	File ..\z39.50\*.c
	File ..\z39.50\*.asn
	SetOutPath $INSTDIR\ill
	File ..\ill\*.c
	File ..\ill\*.asn
	SetOutPath $INSTDIR\zutil
	File ..\zutil\*.c
	SetOutPath $INSTDIR\ccl
	File ..\ccl\*.c
	SetOutPath $INSTDIR\zoom
	File ..\zoom\*.c
	File ..\zoom\*.h
	SetOutPath $INSTDIR\comstack
	File ..\comstack\*.c
	SetOutPath $INSTDIR\server
	File ..\server\*.c
	File ..\server\*.h
	SetOutPath $INSTDIR\retrieval
	File ..\retrieval\*.c
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
; eof
