# makefile.mak - makefile for MS NMAKE 
# $Id: makefile,v 1.30 2001-10-28 23:28:26 adam Exp $
#
# Programmed by
#  HL: Heikki Levanto, Index Data
#
# Log at the end of the file
#
# Missing
# - Move MS-C's whatnots into win direcotry
# - The TCL script produces C and H files in the same dir as the 
#   ASN files. H's are copied to INCL, C's are left there.
#   They should be produced into OBJ...
#  
# Envoronment problems
# - You need to have the proper path and environment for VC set
#   up. There is a bat file VCVARS32.BAT that sets most of it up
#   for you. You can find this somewhere near DevStudio\VC\BIN
# - RegSvr32 must also be along the path, often in WINDOWS\SYSTEM
# - TCL has to be available too, if compiling for NEW_Z3950

###########################################################
############### Parameters 
###########################################################

DEBUG=0   # 0 for release, 1 for debug

default: all

all: dirs yaz_date_h generate dll client ztest zoomsh

generate: generated_files

###########################################################
############### Directories
###########################################################
# The current directory is supposed to be something like
# ..../Yaz/Win, everything is relative to that
ROOTDIR=..   # The home of Yaz

INCLDIR=$(ROOTDIR)\include  # our includes
LIBDIR=$(ROOTDIR)\lib       # We produce .lib, .exp etc there
BINDIR=$(ROOTDIR)\bin       # We produce exes and dlls there
WINDIR=$(ROOTDIR)\win       # all these Win make things
OBJDIR=$(WINDIR)\obj        # where we store intermediate files
UNIXDIR=$(ROOTDIR)\unix     # corresponding unix things
SRCDIR=$(ROOTDIR)           # for the case we move them under src

ASNDIR=$(SRCDIR)\ASN
COMSTACKDIR=$(SRCDIR)\COMSTACK
ODRDIR=$(SRCDIR)\ODR
UTILDIR=$(SRCDIR)\UTIL
ZUTILDIR=$(SRCDIR)\ZUTIL
RETDIR=$(SRCDIR)\RETRIEVAL
Z3950DIR=$(SRCDIR)\Z39.50
ILLDIR=$(SRCDIR)\ill
CCLDIR=$(SRCDIR)\ccl
ZOOMDIR=$(SRCDIR)\zoom

CLIENTDIR=$(SRCDIR)\CLIENT
SERVERDIR=$(SRCDIR)\SERVER
ZTESTDIR=$(SRCDIR)\ZTEST

TMPDIR=$(ROOTDIR)\win\tmp
TMP=$(TMPDIR)

###########################################################
############### Targets - what to make
###########################################################


DLL=$(BINDIR)\Yaz.dll
IMPLIB=$(LIBDIR)\Yaz.lib

CLIENT=$(BINDIR)\yaz-client.exe
ZOOMSH=$(BINDIR)\zoomsh.exe
ZTEST=$(BINDIR)\yaz-ztest.exe
YAZ_DATE_H=$(INCLDIR)\yaz\yaz-date.h

ZOOMTST1=$(BINDIR)\zoomtst1.exe
ZOOMTST2=$(BINDIR)\zoomtst2.exe
ZOOMTST3=$(BINDIR)\zoomtst3.exe
ZOOMTST4=$(BINDIR)\zoomtst4.exe
ZOOMTST5=$(BINDIR)\zoomtst5.exe
ZOOMTST6=$(BINDIR)\zoomtst6.exe
ZOOMTST7=$(BINDIR)\zoomtst7.exe


# shortcut names defined here
dll : $(DLL) 
client: $(CLIENT)
ztest: $(ZTEST)
zoomsh: $(ZOOMSH) $(ZOOMTST1) $(ZOOMTST2) $(ZOOMTST3) \
 $(ZOOMTST4) $(ZOOMTST5) $(ZOOMTST6) $(ZOOMTST7)
yaz_date_h: $(YAZ_DATE_H)

###########################################################
############### Compiler and linker options 
###########################################################


### C and CPP compiler  (the same thing)
# Note: $(CPP) has already been defined in the environment
# (if you set things up right!)

COMMON_C_OPTIONS=          \
  /nologo /W3 /GX /FD /c   \
  /D "YAZ_MODULE_ill=1"    \
  /D "YAZ_MODULE_ccl=1"    \
  /D "_WINDOWS"            \
  /D "WIN32"               \
  /FR"$(OBJDIR)\\"         \
  /Fo"$(OBJDIR)\\"         \
  /Fd"$(OBJDIR)\\" 

COMMON_C_INCLUDES= \
  /I"$(SRCDIR)\include"

DEBUG_C_OPTIONS=  \
  /D "_DEBUG"      \
  /MDd  /Od /YX /Zi /Gm

RELEASE_C_OPTIONS=  \
  /D "NDEBUG"        \
  /MD /O2

# /W3  = warning level
# /GX  = Enable exception handling
# /FD  = Generate file dependencies (what ever they are)
# /c   = compile without linking
# /FR  = Generate browse info (.sbr file that gets combined into .bsc)
# /Fo  = object file name (or at least path)
# /Fd  = debug database name (or path)
# /MD  = Runtime library: Multithread DLL
# /MDd = Runtime library: Multithread DLL (debug)
# /Od  = Disable optimising (debug)
# /O2  = Optimize for speed
# /YX  = Automatic use of precomipled headers
# /Gm  = Minimal rebuild (some cpp class stuff)
# /Zi  = Program database for debuggers
# /ZI  = Pgm database with special "edit&continue" stuff - not available in C5


### Linker options
LINK=link.exe

LINK_LIBS= kernel32.lib user32.lib   gdi32.lib   \
           advapi32.lib uuid.lib \
           wsock32.lib  advapi32.lib

COMMON_LNK_OPTIONS= /nologo \
                    /subsystem:windows \
                    /machine:i386 \
			  /incremental:no

DEBUG_LNK_OPTIONS= /debug 

RELEASE_LNK_OPTIONS=  /pdb:none

DLL_LINK_OPTIONS= /dll  
CLIENT_LINK_OPTIONS = /subsystem:console  
ZOOMSH_LINK_OPTIONS = /subsystem:console
SERVER_LINK_OPTIONS = -lib 
ZTEST_LINK_OPTIONS = /subsystem:console  

# TCL
TCL="C:\Program Files\Tcl\bin\tclsh83.exe"
HAVE_TCL=1

COMMON_TCL_OPTIONS= ..\util\yaz-comp -I$(INCLDIR) -i yaz

# Final opt variables
!if $(DEBUG)
COPT=   $(COMMON_C_OPTIONS)   $(DEBUG_C_OPTIONS)     $(COMMON_C_INCLUDES)
MTLOPT= $(COMMON_MTL_OPTIONS) $(DEBUG_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(DEBUG_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(DEBUG_LNK_OPTIONS)   $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)

!else
COPT=   $(COMMON_C_OPTIONS)   $(RELEASE_C_OPTIONS)   $(COMMON_C_INCLUDES) 
MTLOPT= $(COMMON_MTL_OPTIONS) $(RELEASE_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(RELEASE_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(RELEASE_LNK_OPTIONS) $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)
!endif



###########################################################
###############  Source and object modules
###########################################################

# Note: Ordinary source files are not specified here at 
# all, make finds them in suitable dirs. The object modules
# need to be specified, though

YAZ_CLIENT_OBJS= \
   $(OBJDIR)\client.obj \
   $(OBJDIR)\admin.obj

YAZ_ZOOMSH_OBJS = \
   $(OBJDIR)\zoomsh.obj

YAZ_ZOOMTST1_OBJS = \
   $(OBJDIR)\zoomtst1.obj
YAZ_ZOOMTST2_OBJS = \
   $(OBJDIR)\zoomtst2.obj
YAZ_ZOOMTST3_OBJS = \
   $(OBJDIR)\zoomtst3.obj
YAZ_ZOOMTST4_OBJS = \
   $(OBJDIR)\zoomtst4.obj
YAZ_ZOOMTST5_OBJS = \
   $(OBJDIR)\zoomtst5.obj
YAZ_ZOOMTST6_OBJS = \
   $(OBJDIR)\zoomtst6.obj
YAZ_ZOOMTST7_OBJS = \
   $(OBJDIR)\zoomtst7.obj

YAZ_SERVER_OBJS= \
	"$(OBJDIR)\eventl.obj" \
	"$(OBJDIR)\requestq.obj" \
	"$(OBJDIR)\service.obj" \
	"$(OBJDIR)\seshigh.obj" \
	"$(OBJDIR)\statserv.obj" \
	"$(OBJDIR)\tcpdchk.obj" 

ZTEST_OBJS= \
	"$(OBJDIR)\read-grs.obj" \
	"$(OBJDIR)\ztest.obj" 

YAZ_CCL_OBJS= \
   $(OBJDIR)\cclerrms.obj \
   $(OBJDIR)\cclfind.obj \
   $(OBJDIR)\cclptree.obj \
   $(OBJDIR)\cclqfile.obj \
   $(OBJDIR)\cclqual.obj \
   $(OBJDIR)\cclstr.obj \
   $(OBJDIR)\ccltoken.obj

YAZ_COMSTACK_OBJS= \
   $(OBJDIR)\comstack.obj \
   $(OBJDIR)\tcpip.obj \
   $(OBJDIR)\waislen.obj 

YAZ_ODR_OBJS= \
   $(OBJDIR)\ber_any.obj \
   $(OBJDIR)\ber_bit.obj \
   $(OBJDIR)\ber_bool.obj \
   $(OBJDIR)\ber_int.obj \
   $(OBJDIR)\ber_len.obj \
   $(OBJDIR)\ber_null.obj \
   $(OBJDIR)\ber_oct.obj \
   $(OBJDIR)\ber_oid.obj \
   $(OBJDIR)\ber_tag.obj \
   $(OBJDIR)\dumpber.obj \
   $(OBJDIR)\odr.obj \
   $(OBJDIR)\odr_any.obj \
   $(OBJDIR)\odr_bit.obj \
   $(OBJDIR)\odr_bool.obj \
   $(OBJDIR)\odr_choice.obj \
   $(OBJDIR)\odr_cons.obj \
   $(OBJDIR)\odr_enum.obj \
   $(OBJDIR)\odr_int.obj \
   $(OBJDIR)\odr_mem.obj \
   $(OBJDIR)\odr_null.obj \
   $(OBJDIR)\odr_oct.obj \
   $(OBJDIR)\odr_oid.obj \
   $(OBJDIR)\odr_seq.obj \
   $(OBJDIR)\odr_tag.obj \
   $(OBJDIR)\odr_use.obj \
   $(OBJDIR)\odr_util.obj 

YAZ_UTIL_OBJS= \
   $(OBJDIR)\atoin.obj \
   $(OBJDIR)\log.obj \
   $(OBJDIR)\marcdisp.obj \
   $(OBJDIR)\nmem.obj \
   $(OBJDIR)\nmemsdup.obj \
   $(OBJDIR)\oid.obj \
   $(OBJDIR)\options.obj \
   $(OBJDIR)\readconf.obj \
   $(OBJDIR)\tpath.obj \
   $(OBJDIR)\wrbuf.obj \
   $(OBJDIR)\xmalloc.obj \
   $(OBJDIR)\matchstr.obj 

YAZ_ZUTIL_OBJS= \
   $(OBJDIR)\diagbib1.obj \
   $(OBJDIR)\zget.obj \
   $(OBJDIR)\logrpn.obj \
   $(OBJDIR)\pquery.obj \
   $(OBJDIR)\yaz-ccl.obj \
   $(OBJDIR)\otherinfo.obj \
   $(OBJDIR)\sortspec.obj \
   $(OBJDIR)\z3950oid.obj


YAZ_RET_OBJS= \
   $(OBJDIR)\d1_absyn.obj\
   $(OBJDIR)\d1_attset.obj\
   $(OBJDIR)\d1_doespec.obj\
   $(OBJDIR)\d1_espec.obj\
   $(OBJDIR)\d1_expout.obj\
   $(OBJDIR)\d1_grs.obj\
   $(OBJDIR)\d1_handle.obj\
   $(OBJDIR)\d1_map.obj\
   $(OBJDIR)\d1_marc.obj\
   $(OBJDIR)\d1_prtree.obj\
   $(OBJDIR)\d1_read.obj\
   $(OBJDIR)\d1_soif.obj\
   $(OBJDIR)\d1_sumout.obj\
   $(OBJDIR)\d1_sutrs.obj\
   $(OBJDIR)\d1_tagset.obj\
   $(OBJDIR)\d1_varset.obj\
   $(OBJDIR)\d1_write.obj\
   $(OBJDIR)\d1_if.obj

Z3950_OBJS= \
   $(OBJDIR)\z-date.obj\
   $(OBJDIR)\z-univ.obj\
   $(OBJDIR)\zes-update.obj\
   $(OBJDIR)\zes-admin.obj \
   $(OBJDIR)\z-accdes1.obj \
   $(OBJDIR)\z-accform1.obj \
   $(OBJDIR)\z-acckrb1.obj \
   $(OBJDIR)\z-core.obj \
   $(OBJDIR)\z-diag1.obj \
   $(OBJDIR)\z-espec1.obj \
   $(OBJDIR)\z-estask.obj \
   $(OBJDIR)\z-exp.obj \
   $(OBJDIR)\z-grs.obj \
   $(OBJDIR)\z-opac.obj \
   $(OBJDIR)\z-uifr1.obj \
   $(OBJDIR)\z-rrf1.obj \
   $(OBJDIR)\z-rrf2.obj \
   $(OBJDIR)\z-sum.obj \
   $(OBJDIR)\z-sutrs.obj \
   $(OBJDIR)\zes-expi.obj \
   $(OBJDIR)\zes-exps.obj \
   $(OBJDIR)\zes-order.obj \
   $(OBJDIR)\zes-pquery.obj \
   $(OBJDIR)\zes-psched.obj \
   $(OBJDIR)\zes-pset.obj \
   $(OBJDIR)\zes-update0.obj \
   $(OBJDIR)\prt-ext.obj

ILL_OBJS= \
   $(OBJDIR)\ill-get.obj\
   $(OBJDIR)\ill-core.obj\
   $(OBJDIR)\item-req.obj

YAZ_ZOOM_OBJS = \
   $(OBJDIR)\zoom-c.obj \
   $(OBJDIR)\zoom-opt.obj

COMMON_YAZ_OBJS= \
   $(YAZ_UTIL_OBJS) \
   $(YAZ_ODR_OBJS) \
   $(YAZ_COMSTACK_OBJS) \
   $(YAZ_ZUTIL_OBJS) \
   $(YAZ_CCL_OBJS) \
   $(YAZ_RET_OBJS) \
   $(YAZ_SERVER_OBJS) \
   $(YAZ_ZOOM_OBJS)

YAZ_OBJS= \
	$(COMMON_YAZ_OBJS) \
	$(Z3950_OBJS) \
	$(ILL_OBJS) 

DLL_OBJS= $(YAZ_OBJS)

ALL_OBJS= \
	$(YAZ_OBJS) \
	$(YAZ_CLIENT_OBJS) \
	$(ZTEST_OBJS)

###########################################################
############### Generated C and H files
#####
######################################################

Z3950_C_DIR=$(Z3950DIR)   
ILL_C_DIR=$(ILLDIR)
#!!! Should be moved to OBJ, but that requires too much trickery

# Files generated from datetime.asn
DATETIME_H_FILES = $(INCLDIR)\yaz\z-date.h
DATETIME_C_FILES = $(Z3950_C_DIR)\z-date.c

# Files generated from univres.asn
UNIVRES_H_FILES = $(INCLDIR)\yaz\z-univ.h
UNIVRES_C_FILES = $(Z3950_C_DIR)\z-univ.c

# Files generated from esupdate.asn
ESUPDATE_H_FILES = $(INCLDIR)\yaz\zes-update.h
ESUPDATE_C_FILES = $(Z3950_C_DIR)\zes-update.c

# Files generated from esadmin.asn
ESADMIN_H_FILES = $(INCLDIR)\yaz\zes-admin.h
ESADMIN_C_FILES = $(Z3950_C_DIR)\zes-admin.c

# Files created from z3950v3.asn
Z3950V3_H_FILES= \
   $(INCLDIR)\yaz\z-accdes1.h \
   $(INCLDIR)\yaz\z-core.h

Z3950V3_C_FILES= \
   $(Z3950_C_DIR)\z-accdes1.c \
   $(Z3950_C_DIR)\z-accform1.c \
   $(Z3950_C_DIR)\z-acckrb1.c \
   $(Z3950_C_DIR)\z-core.c \
   $(Z3950_C_DIR)\z-diag1.c \
   $(Z3950_C_DIR)\z-espec1.c \
   $(Z3950_C_DIR)\z-estask.c \
   $(Z3950_C_DIR)\z-exp.c \
   $(Z3950_C_DIR)\z-grs.c \
   $(Z3950_C_DIR)\z-opac.c \
   $(Z3950_C_DIR)\z-uifr1.c \
   $(Z3950_C_DIR)\z-rrf1.c \
   $(Z3950_C_DIR)\z-rrf2.c \
   $(Z3950_C_DIR)\z-sum.c \
   $(Z3950_C_DIR)\z-sutrs.c \
   $(Z3950_C_DIR)\zes-expi.c \
   $(Z3950_C_DIR)\zes-exps.c \
   $(Z3950_C_DIR)\zes-order.c \
   $(Z3950_C_DIR)\zes-pquery.c \
   $(Z3950_C_DIR)\zes-psched.c \
   $(Z3950_C_DIR)\zes-pset.c \
   $(Z3950_C_DIR)\zes-update0.c

# Files generated from ill9702.asn
ILL_CORE_H_FILES= \
   $(INCLDIR)\yaz\ill-core.h

ILL_CORE_C_FILES= \
   $(ILL_C_DIR)\ill-core.c

# Files generated from itemreq.asn
ITEM_REQ_H_FILES= \
   $(INCLDIR)\yaz\item-req.h

ITEM_REQ_C_FILES= \
   $(ILL_C_DIR)\item-req.c

# Combined..
DATETIME_FILES = $(DATETIME_H_FILES) $(DATETIME_C_FILES)
UNIVRES_FILES = $(UNIVRES_H_FILES) $(UNIVRES_C_FILES)
ESUPDATE_FILES = $(ESUPDATE_H_FILES) $(ESUPDATE_C_FILES)
ESADMIN_FILES = $(ESADMIN_H_FILES) $(ESADMIN_C_FILES)
Z3950V3_FILES= $(Z3950V3_C_FILES) $(Z3950V3_H_FILES)
ILL_CORE_FILES= $(ILL_CORE_C_FILES) $(ILL_CORE_H_FILES)
ITEM_REQ_FILES= $(ITEM_REQ_C_FILES) $(ITEM_REQ_H_FILES)

GENERATED_C_FILES= \
   $(Z3950V3_C_FILES)  \
   $(ESUPDATE_C_FILES) \
   $(UNIVRES_C_FILES)  \
   $(DATETIME_C_FILES) \
   $(ESADMIN_C_FILES)

GENERATED_H_FILES= \
   $(Z3950V3_H_FILES)  \
   $(ESUPDATE_H_FILES) \
   $(UNIVRES_H_FILES)  \
   $(DATETIME_H_FILES) \
   $(ESADMIN_H_FILES)

generated_files: \
	$(GENERATED_H_FILES) \
	$(GENERATED_C_FILES) \

###########################################################
############### Compiling 
###########################################################

# Note: This defines where to look for the necessary
# source files. Funny way of doing it, but it works.

# DLL sources
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@$(CPP) $(COPT) $<

# Yaz client
{$(CLIENTDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) $(COPT) $< /D"_CONSOLE"

# Ztest
{$(ZTESTDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) $(COPT) $< /D"_CONSOLE"

# Server
{$(SERVERDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

# Various YAZ source directories
{$(ASNDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(COMSTACKDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(ODRDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(UTILDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(ZUTILDIR)}.c{$(OBJDIR)}.obj:
	$(CPP)	$(COPT) $< 

{$(RETDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(Z3950_C_DIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(ILL_C_DIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(CCLDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

{$(ZOOMDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< 

############### ASN-generated files

!if $(HAVE_TCL)

$(YAZ_DATE_H):
	@cd ..
	$(TCL) util\cvs-date.tcl include\yaz\yaz-date.h
	@cd win

$(Z3950V3_FILES): $(Z3950DIR)\z3950v3.asn
	@cd $(Z3950DIR)
	$(TCL) $(TCLOPT) -d z.tcl z3950v3.asn
	@cd $(WINDIR)

$(DATETIME_FILES): $(Z3950DIR)\datetime.asn
	@cd $(Z3950DIR)
	$(TCL) $(TCLOPT) -d z.tcl datetime.asn
	@cd $(WINDIR)

$(UNIVRES_FILES): $(Z3950DIR)\univres.asn
	@cd $(Z3950DIR)
	$(TCL) $(TCLOPT) -d z.tcl univres.asn
	@cd $(WINDIR)

$(ESUPDATE_FILES): $(Z3950DIR)\esupdate.asn
	@cd $(Z3950DIR)
	$(TCL) $(TCLOPT) -d z.tcl esupdate.asn
	@cd $(WINDIR)

$(ESADMIN_FILES): $(Z3950DIR)\esadmin.asn
	@cd $(Z3950DIR)
	$(TCL) $(TCLOPT) -d z.tcl esadmin.asn
	@cd $(WINDIR)

$(ILL_CORE_FILES): $(ILLDIR)\ill9702.asn
	@cd $(ILLDIR)
	$(TCL) $(TCLOPT) -d ill.tcl ill9702.asn
	@cd $(WINDIR)

$(ITEM_REQ_FILES): $(ILLDIR)\item-req.asn
	@cd $(ILLDIR)
	$(TCL) $(TCLOPT) -d ill.tcl item-req.asn
	@cd $(WINDIR)

!endif


###########################################################
############### Resources
###########################################################

### The RC compiler (resource files)
RSC=rc.exe
COMMON_RC_OPTIONS= /l 0x406 /i"$(ROOTDIR)" 
DEBUG_RC_OPTIONS=/d "_DEBUG"
RELEASE_RC_OPTIONS=/d "NDEBUG"

YAZ_RES=$(OBJDIR)\yaz.res
YAZ_RC=$(WINDIR)\yaz.rc

!if $(DEBUG)
RSOPT=/d_DEBUG
!else
RSOPT=/d_NDEBUG
!endif

$(YAZ_RES): $(YAZ_RC)
	$(RSC) $(RSOPT) /fo"$(YAZ_RES)" $(YAZ_RC) 

###########################################################
############### Linking
###########################################################

$(DLL) $(IMPLIB): "$(BINDIR)" $(DLL_OBJS) $(YAZ_RES)
	@echo Linking the dll  $(DLL)
	$(LINK) @<<
		$(LNKOPT) 
		$(LINK_LIBS) 
		$(DLL_LINK_OPTIONS)
		$(DLL_OBJS)
		$(YAZ_RES)
		/out:$(DLL) 
		/implib:"$(LIBDIR)\yaz.lib"
		/pdb:"$(LIBDIR)\yaz.pdb" 
		/map:"$(LIBDIR)\yaz.map"  
<<

$(CLIENT) : "$(BINDIR)" $(YAZ_CLIENT_OBJS) 
	@echo Linking $(CLIENT)
	$(LINK) @<<
        $(LNKOPT) 
		$(CLIENT_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_CLIENT_OBJS)
		/pdb:"$(LIBDIR)\yaz-client.pdb"
		/map:"$(LIBDIR)\yaz-client.map"
		/out:$(CLIENT)
<<

$(ZOOMSH) : "$(BINDIR)" $(YAZ_ZOOMSH_OBJS) 
	@echo Linking $(ZOOMSH)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMSH_OBJS)
		/pdb:"$(LIBDIR)\zoomsh.pdb"
		/map:"$(LIBDIR)\zoomsh.map"
		/out:$(ZOOMSH)
<<

$(ZOOMTST1) : "$(BINDIR)" $(YAZ_ZOOMTST1_OBJS) 
	@echo Linking $(ZOOMTST1)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST1_OBJS)
		/pdb:"$(LIBDIR)\zoomtst1.pdb"
		/map:"$(LIBDIR)\zoomtst1.map"
		/out:$(ZOOMTST1)
<<

$(ZOOMTST2) : "$(BINDIR)" $(YAZ_ZOOMTST2_OBJS) 
	@echo Linking $(ZOOMTST2)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST2_OBJS)
		/pdb:"$(LIBDIR)\zoomtst2.pdb"
		/map:"$(LIBDIR)\zoomtst2.map"
		/out:$(ZOOMTST2)
<<

$(ZOOMTST3) : "$(BINDIR)" $(YAZ_ZOOMTST3_OBJS) 
	@echo Linking $(ZOOMTST3)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST3_OBJS)
		/pdb:"$(LIBDIR)\zoomtst3.pdb"
		/map:"$(LIBDIR)\zoomtst3.map"
		/out:$(ZOOMTST3)
<<

$(ZOOMTST4) : "$(BINDIR)" $(YAZ_ZOOMTST4_OBJS) 
	@echo Linking $(ZOOMTST4)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST4_OBJS)
		/pdb:"$(LIBDIR)\zoomtst4.pdb"
		/map:"$(LIBDIR)\zoomtst4.map"
		/out:$(ZOOMTST4)
<<

$(ZOOMTST5) : "$(BINDIR)" $(YAZ_ZOOMTST5_OBJS) 
	@echo Linking $(ZOOMTST5)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST5_OBJS)
		/pdb:"$(LIBDIR)\zoomtst5.pdb"
		/map:"$(LIBDIR)\zoomtst5.map"
		/out:$(ZOOMTST5)
<<

$(ZOOMTST6) : "$(BINDIR)" $(YAZ_ZOOMTST6_OBJS) 
	@echo Linking $(ZOOMTST6)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST6_OBJS)
		/pdb:"$(LIBDIR)\zoomtst6.pdb"
		/map:"$(LIBDIR)\zoomtst6.map"
		/out:$(ZOOMTST6)
<<

$(ZOOMTST7) : "$(BINDIR)" $(YAZ_ZOOMTST7_OBJS) 
	@echo Linking $(ZOOMTST7)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZOOMSH_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_ZOOMTST7_OBJS)
		/pdb:"$(LIBDIR)\zoomtst7.pdb"
		/map:"$(LIBDIR)\zoomtst7.map"
		/out:$(ZOOMTST7)
<<

$(ZTEST) : "$(BINDIR)" $(ZTEST_OBJS) $(DLL)
	@echo Linking the ztest  $(ZTEST)
	$(LINK) @<<
        $(LNKOPT) 
		$(ZTEST_LINK_OPTIONS)
		$(LINK_LIBS) 
		shell32.lib
		$(IMPLIB)
		$(ZTEST_OBJS) 
		/implib:"$(LIBDIR)\yaz-ztest.lib"
		/pdb:"$(LIBDIR)\yaz-ztest.pdb"
		/map:"$(LIBDIR)\yaz-ztest.map"
		/out:$(ZTEST) 
<<


# note that this links a lib, so it uses completely different options.

###########################################################
############### Special operations
###########################################################


############## clean
clean:
	del $(BINDIR)\*.exe
	del $(BINDIR)\*.dll
	del $(TMPDIR)\*.

	del $(LIBDIR)\*.MAP

	del $(LIBDIR)\*.LIB
	del $(OBJDIR)\*.OBJ

realclean: clean
	del $(Z3950_C_DIR)\*.c
	del $(Z3950_C_DIR)\*.h
	del $(INCLDIR)\yaz\z-accdes1.h
	del $(INCLDIR)\yaz\z-core.h
	del $(DATETIME_H_FILES)
	del $(UNIVRES_H_FILES)
	del $(ESUPDATE_H_FILES)

# Because DOS del will only accept one file name to delete,
# the _H_ files work only on sets that have just one file.
# Z3950_H_FILES had to be spelled out. One more point for MS!

########### check directories and create if needed
dirs: $(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR)

$(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR):
 	if not exist "$@/$(NUL)" mkdir "$@"


###########################################################
############### Explicit dependencies
###########################################################

# force recompilation of everything, if makefile changed

$(Z3950_OBJS): $(GENERATED_C_FILES) $(GENERATED_H_FILES)

$(ILL_OBJS): $(ILL_CORE_FILES) $(ITEM_REQ_FILES)

# makes sure we generate before compiling anything, as the
# new proto.h refers to the generated files, and is included
# in various places

###########################################################
############### Log
###########################################################
#
# $Log: makefile,v $
# Revision 1.30  2001-10-28 23:28:26  adam
# Enable CCL-to-RPN for YAZ client on WIN32.
#
# Revision 1.29  2001/10/25 12:46:07  adam
# codecs not built as they should.
#
# Revision 1.28  2001/10/25 10:45:08  adam
# Fixed make clean.
#
# Revision 1.27  2001/10/24 12:24:43  adam
# WIN32 updates: ZOOM runs, nmem_init/nmem_exit called in DllMain.
#
# Revision 1.26  2001/09/27 12:09:18  adam
# Function nmem_exit calls oid_exit (when reference is 0).
#
# Revision 1.25  2001/09/24 21:51:56  adam
# New Z39.50 OID utilities: yaz_oidval_to_z3950oid, yaz_str_to_z3950oid
# and yaz_z3950oid_to_str.
#
# Revision 1.24  2001/08/14 12:05:58  adam
# Added sortspec.
#
# Revision 1.23  2001/05/18 11:42:03  adam
# YAZ Build date for WIN32.
#
# Revision 1.22  2001/03/09 14:58:20  adam
# Updated version resource.
#
# Revision 1.21  2001/02/28 16:56:33  adam
# Added version info for WIN32.
#
# Revision 1.20  2000/12/01 17:56:41  adam
# on WIN32 function statserv_closedown closes socket(s) to provoke close.
#
# Revision 1.19  2000/11/16 13:03:13  adam
# Function ccl_rpn_query sets attributeSet to Bib-1.
#
# Revision 1.18  2000/11/01 14:47:00  adam
# Added CCL support for WIN32.
#
# Revision 1.17  2000/10/06 12:01:12  adam
# Updates regarding ASN-code generation (mostly).
#
# Revision 1.16  2000/05/05 13:48:15  adam
# Minor changes.
#
# Revision 1.15  2000/04/17 14:21:38  adam
# WIN32 update.
#
# Revision 1.14  2000/03/02 08:48:21  adam
# Renamed ASN.1 compiler to yaz-comp (used to be yc.tcl).
#
# Revision 1.13  2000/02/28 11:13:03  adam
# Removed odr_priv.obj.
#
# Revision 1.12  2000/01/06 11:27:16  adam
# Updated for ILL.
#
# Revision 1.11  1999/12/21 14:16:20  ian
# Changed retrieval module to allow data1 trees with no associated absyn.
# Also added a simple interface for extracting values from data1 trees using
# a string based tagpath.
#
# Revision 1.10  1999/12/08 13:10:48  adam
# New version.
#
# Revision 1.9  1999/11/30 13:47:12  adam
# Improved installation. Moved header files to include/yaz.
#
# Revision 1.8  1999/07/21 08:48:02  adam
# Removed dmalloc.obj.
#
# Revision 1.7  1999/06/09 15:10:08  heikki
# Cleaning up. Seems to work all right
#
# Revision 1.6  1999/06/09 13:33:32  heikki
# Compiles and links both old and new type stuff all right
#
# Revision 1.5  1999/06/09 11:05:30  heikki
# At least it can compile
#
# Revision 1.4  1999/06/09 09:41:09  heikki
# More work on the ASN-generated files.
#
# Revision 1.3  1999/06/08 14:32:30  heikki
# Proto.h works all right, removed linker warnings from server.lib
#
# Revision 1.2  1999/06/08 14:07:24  heikki
# Renamed a pile of files
# Tmpdir (to get around Ms leaving temp files around, and crashing
# when too many with same number...)
#
# Revision 1.1  1999/06/08 12:15:41  heikki
# Renamed to makefile (.nothing) (from .mak)
# Working on the proto.h problems and alternative confiigurations
#
# Revision 1.5  1999/06/04 10:04:28  heikki
# Cleaning up
#
# Revision 1.4  1999/06/02 13:23:29  heikki
# Debug options for C compiler
#
# Revision 1.3  1999/05/19 08:26:22  heikki
# Added comments
#
#



