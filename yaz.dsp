# Microsoft Developer Studio Project File - Name="yaz" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=yaz - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "yaz.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yaz.mak" CFG="yaz - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yaz - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "yaz - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yaz - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386
# SUBTRACT LINK32 /map
# Begin Special Build Tool
OutDir=.\.\Release
ProjDir=.
TargetName=yaz
SOURCE=$(InputPath)
PostBuild_Desc=Copy Lib and Dll
PostBuild_Cmds=copy    $(OutDir)\$(TargetName).dll $(ProjDir)\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "yaz - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\yaz___Wi"
# PROP BASE Intermediate_Dir ".\yaz___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386
# SUBTRACT LINK32 /pdb:none /force
# Begin Special Build Tool
OutDir=.\.\Debug
ProjDir=.
TargetName=yaz
SOURCE=$(InputPath)
PostBuild_Desc=Copy Dll
PostBuild_Cmds=copy    $(OutDir)\$(TargetName).dll $(ProjDir)\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "yaz - Win32 Release"
# Name "yaz - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Util\atoin.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_any.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_bit.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_bool.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_int.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_len.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_null.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_oct.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_oid.c
# End Source File
# Begin Source File

SOURCE=.\odr\ber_tag.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclerrms.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclfind.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclptree.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclqfile.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclqual.c
# End Source File
# Begin Source File

SOURCE=.\ccl\cclstr.c
# End Source File
# Begin Source File

SOURCE=.\ccl\ccltoken.c
# End Source File
# Begin Source File

SOURCE=.\comstack\comstack.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_absyn.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_attset.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_doespec.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_espec.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_expout.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_grs.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_handle.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_map.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_marc.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_prtree.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_read.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_soif.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_sumout.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_sutrs.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_tagset.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_varset.c
# End Source File
# Begin Source File

SOURCE=.\retrieval\d1_write.c
# End Source File
# Begin Source File

SOURCE=.\asn\diagbib1.c
# End Source File
# Begin Source File

SOURCE=.\util\dmalloc.c
# End Source File
# Begin Source File

SOURCE=.\odr\dumpber.c
# End Source File
# Begin Source File

SOURCE=.\util\log.c
# End Source File
# Begin Source File

SOURCE=.\util\logrpn.c
# End Source File
# Begin Source File

SOURCE=.\util\marcdisp.c
# End Source File
# Begin Source File

SOURCE=.\util\nmem.c
# End Source File
# Begin Source File

SOURCE=.\util\nmemsdup.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_any.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_bit.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_bool.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_choice.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_cons.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_int.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_mem.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_null.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_oct.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_oid.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_priv.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_seq.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_tag.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_use.c
# End Source File
# Begin Source File

SOURCE=.\odr\odr_util.c
# End Source File
# Begin Source File

SOURCE=.\util\oid.c
# End Source File
# Begin Source File

SOURCE=.\util\options.c
# End Source File
# Begin Source File

SOURCE=.\util\pquery.c
# End Source File
# Begin Source File

SOURCE=.\asn\proto.c
# End Source File
# Begin Source File

SOURCE=".\asn\prt-acc.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-add.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-arc.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-dat.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-dia.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-esp.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-exd.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-exp.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-ext.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-grs.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-rsc.c"
# End Source File
# Begin Source File

SOURCE=".\asn\prt-univ.c"
# End Source File
# Begin Source File

SOURCE=.\util\query.c
# End Source File
# Begin Source File

SOURCE=.\util\readconf.c
# End Source File
# Begin Source File

SOURCE=.\comstack\tcpip.c
# End Source File
# Begin Source File

SOURCE=.\util\tpath.c
# End Source File
# Begin Source File

SOURCE=.\comstack\waislen.c
# End Source File
# Begin Source File

SOURCE=.\util\wrbuf.c
# End Source File
# Begin Source File

SOURCE=.\util\xmalloc.c
# End Source File
# Begin Source File

SOURCE=".\util\yaz-ccl.c"
# End Source File
# Begin Source File

SOURCE=".\util\yaz-util.c"
# End Source File
# Begin Source File

SOURCE=.\asn\zget.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\include\ccl.h
# End Source File
# Begin Source File

SOURCE=.\include\comstack.h
# End Source File
# Begin Source File

SOURCE=.\include\d1_attset.h
# End Source File
# Begin Source File

SOURCE=.\include\d1_map.h
# End Source File
# Begin Source File

SOURCE=.\include\data1.h
# End Source File
# Begin Source File

SOURCE=.\include\diagbib1.h
# End Source File
# Begin Source File

SOURCE=.\include\dmalloc.h
# End Source File
# Begin Source File

SOURCE=.\server\eventl.h
# End Source File
# Begin Source File

SOURCE=.\include\log.h
# End Source File
# Begin Source File

SOURCE=.\include\marcdisp.h
# End Source File
# Begin Source File

SOURCE=.\include\nmem.h
# End Source File
# Begin Source File

SOURCE=.\include\odr.h
# End Source File
# Begin Source File

SOURCE=.\include\odr_use.h
# End Source File
# Begin Source File

SOURCE=.\include\oid.h
# End Source File
# Begin Source File

SOURCE=.\include\options.h
# End Source File
# Begin Source File

SOURCE=.\include\pquery.h
# End Source File
# Begin Source File

SOURCE=.\include\proto.h
# End Source File
# Begin Source File

SOURCE=".\include\prt-acc.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-add.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-arc.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-dia.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-esp.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-exd.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-exp.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-ext.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-grs.h"
# End Source File
# Begin Source File

SOURCE=".\include\prt-rsc.h"
# End Source File
# Begin Source File

SOURCE=.\include\prt.h
# End Source File
# Begin Source File

SOURCE=.\include\readconf.h
# End Source File
# Begin Source File

SOURCE=.\include\tcpip.h
# End Source File
# Begin Source File

SOURCE=.\include\tpath.h
# End Source File
# Begin Source File

SOURCE=.\include\wrbuf.h
# End Source File
# Begin Source File

SOURCE=.\include\xmalloc.h
# End Source File
# Begin Source File

SOURCE=".\include\yaz-ccl.h"
# End Source File
# Begin Source File

SOURCE=".\include\yaz-util.h"
# End Source File
# Begin Source File

SOURCE=".\include\yaz-version.h"
# End Source File
# Begin Source File

SOURCE=.\include\yconfig.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
