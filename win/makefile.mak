# Makefile.mak - makefile for MS NMAKE 
# $Id: makefile.mak,v 1.3 1999-05-19 08:26:22 heikki Exp $
#
# Programmed by
#  HL: Heikki Levanto, Index Data
#
# History
#  18-05-99 HL Stole this from YazX, cleaning up
#
# Missing
# - Move MS-C's whatnots into win direcotry
# - Log and ID 
# - rename to makefile (.nothing)
# - same to yazx...
#  
# Envoronment problems
# - You need to have the proper path and environment for VC set
#   up. There is a bat file VCVARS32.BAT that sets most of it up
#   for you. You can find this somewhere near DevStudio\VC\BIN
# - RegSvr32 must also be along the path, often in WINDOWS\SYSTEM

###########################################################
############### Parameters 
###########################################################

DEBUG=1   # 0 for release, 1 for debug

default: all
all: dirs dll client server ztest



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
RETDIR=$(SRCDIR)\RETRIEVAL

CLIENTDIR=$(SRCDIR)\CLIENT
SERVERDIR=$(SRCDIR)\SERVER
ZTESTDIR=$(SRCDIR)\ZTEST

###########################################################
############### Targets - what to make
###########################################################


DLL=$(BINDIR)\Yaz.dll
IMPLIB=$(BINDIR)\Yaz.lib
BSCFILE=$(LIBDIR)\Yaz.bsc

CLIENT=$(BINDIR)\client.exe
SERVER=$(BINDIR)\server.lib
ZTEST=$(BINDIR)\ztest.exe

# shortcut names defined here
dll : $(DLL)  $(BSCFILE)
client: $(CLIENT)
server: $(SERVER)
ztest: $(ZTEST)

###########################################################
############### Compiler and linker options 
###########################################################


### C and CPP compiler  (the same thing)
# Note: $(CPP) has already been defined in the environment
# (if you set things up right!)

COMMON_C_OPTIONS=          \
  /nologo /W3 /GX /FD /c   \
  /D "WIN32" /D "_WINDOWS" \
  /FR"$(OBJDIR)\\"         \
  /Fo"$(OBJDIR)\\"         \
  /Fd"$(OBJDIR)\\" 

#  /Fp"$(OBJDIR)\YazX3950.pch" \
#  /D "_WIN32_DCOM" \

COMMON_C_INCLUDES= \
  /I"$(SRCDIR)\include"

#  /I"$(ROOTDIR)" \
#  /I"$(OBJDIR)" 

DEBUG_C_OPTIONS=  \
  /D "_DEBUG"      \
  /MD  /Od /YX  
 
RELEASE_C_OPTIONS=  \
  /D "NDEBUG"        \
  /MDd /O2 /Gm /ZI 


### The RC compiler (resource files)
RSC=rc.exe
COMMON_RC_OPTIONS= /l 0x406 /i"$(ROOTDIR)" 
DEBUG_RC_OPTIONS=/d "_DEBUG"
RELEASE_RC_OPTIONS=/d "NDEBUG"


### Linker options
LINK=link.exe

LINK_LIBS= kernel32.lib user32.lib   gdi32.lib   winspool.lib \
           comdlg32.lib advapi32.lib shell32.lib ole32.lib    \
           oleaut32.lib uuid.lib     odbc32.lib  odbccp32.lib \
           wsock32.lib  advapi32.lib



#odbccp32.lib yaz.lib /nologo /subsystem:console /incremental:no /pdb:".\Debug/client.pdb" /debug /machine:I386 /out:".\Debug/client.exe" /libpath:"..\debug" 
#kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib 
#ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib yaz.lib server.lib 
#ZT: /nologo /subsystem:console /incremental:no /pdb:"Debug/ztest.pdb" /debug /machine:I386 /out:"Debug/ztest.exe" /pdbtype:sept /libpath:"..\debug" /libpath:"..\server\debug" 


COMMON_LNK_OPTIONS= /nologo \
                    /subsystem:windows \
                    /machine:i386 \
			  /incremental:no

DEBUG_LNK_OPTIONS= /debug 

RELEASE_LNK_OPTIONS=  /pdb:none

DLL_LINK_OPTIONS= /dll  
CLIENT_LINK_OPTIONS = /subsystem:console  
SERVER_LINK_OPTIONS = -lib 
ZTEST_LINK_OPTIONS = /subsystem:console  

#shell32.lib 

### BSC compiler options

BSCMAKE=bscmake.exe


COMMON_BSC_OPTIONS= /nologo /o "$(BSCFILE)" /n
DEBUG_BSC_OPTIONS=
RELEASE_BSC_OPTIONS=

# Final opt variables
!if $(DEBUG)
COPT=   $(COMMON_C_OPTIONS)   $(DEBUG_C_OPTIONS)     $(COMMON_C_INCLUDES)
MTLOPT= $(COMMON_MTL_OPTIONS) $(DEBUG_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(DEBUG_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(DEBUG_LNK_OPTIONS)   $(LNK_LIBS)
BSCOPT= $(COMMON_BSC_OPTIONS) $(DEBUG_BSC_OPTIONS)

!else
COPT=   $(COMMON_C_OPTIONS)   $(RELEASE_C_OPTIONS)   $(COMMON_C_INCLUDES) 
MTLOPT= $(COMMON_MTL_OPTIONS) $(RELEASE_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(RELEASE_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(RELEASE_LNK_OPTIONS) $(LNK_LIBS)
BSCOPT= $(COMMON_BSC_OPTIONS) $(RELEASE_BSC_OPTIONS)
!endif



###########################################################
###############  Source and object modules
###########################################################

# The resource files

RCFILE=$(SRCDIR)\compmak.rc
# Horrible Hack: The resfile contains just one line, pointing
# to the component.tlb file (which is created by the idl compiler)
# Devstudio wants that file to live in YazX3950, this makefile in
# win/obj. So we need to RC files!

RESFILE=$(OBJDIR)\component.res


# The def file
#DEF_FILE= $(ROOTDIR)\component.def 



# Note: Ordinary source files are not specified here at 
# all, make finds them in suitable dirs. The object modules
# need to be specified, though

YAZ_CLIENT_OBJS= \
   $(OBJDIR)\client.obj

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
	

YAZ_ASN_OBJS= \
   $(OBJDIR)\diagbib1.obj \
   $(OBJDIR)\proto.obj \
   $(OBJDIR)\prt-acc.obj \
   $(OBJDIR)\prt-add.obj \
   $(OBJDIR)\prt-arc.obj \
   $(OBJDIR)\prt-dat.obj \
   $(OBJDIR)\prt-dia.obj \
   $(OBJDIR)\prt-esp.obj \
   $(OBJDIR)\prt-exd.obj \
   $(OBJDIR)\prt-exp.obj \
   $(OBJDIR)\prt-ext.obj \
   $(OBJDIR)\prt-grs.obj \
   $(OBJDIR)\prt-rsc.obj \
   $(OBJDIR)\prt-univ.obj \
   $(OBJDIR)\zget.obj 

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
   $(OBJDIR)\odr_priv.obj \
   $(OBJDIR)\odr_seq.obj \
   $(OBJDIR)\odr_tag.obj \
   $(OBJDIR)\odr_use.obj \
   $(OBJDIR)\odr_util.obj 

YAZ_UTIL_OBJS= \
   $(OBJDIR)\atoin.obj \
   $(OBJDIR)\dmalloc.obj \
   $(OBJDIR)\log.obj \
   $(OBJDIR)\logrpn.obj \
   $(OBJDIR)\marcdisp.obj \
   $(OBJDIR)\nmem.obj \
   $(OBJDIR)\nmemsdup.obj \
   $(OBJDIR)\oid.obj \
   $(OBJDIR)\options.obj \
   $(OBJDIR)\pquery.obj \
   $(OBJDIR)\query.obj \
   $(OBJDIR)\readconf.obj \
   $(OBJDIR)\tpath.obj \
   $(OBJDIR)\wrbuf.obj \
   $(OBJDIR)\xmalloc.obj \
   $(OBJDIR)\yaz-ccl.obj \
   $(OBJDIR)\yaz-util.obj 

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
   $(OBJDIR)\d1_write.obj

YAZ_OBJS= \
   $(YAZ_ASN_OBJS) \
   $(YAZ_COMSTACK_OBJS) \
   $(YAZ_ODR_OBJS) \
   $(YAZ_UTIL_OBJS) \
   $(YAZ_RET_OBJS)

DLL_OBJS= $(YAZ_OBJS)


###########################################################
############### Compiling 
###########################################################

# Note: This defines where to look for the necessary
# source files. Funny way of doing it, but it works.

# DLL sources
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@$(CPP) @<<
	$(COPT) $<
<<

# Yaz client
{$(CLIENTDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
      /D"_CONSOLE"
	$(COPT) $< 
<<

# Ztest
{$(ZTESTDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
      /D"_CONSOLE"
	/D"_MBCS"
	$(COPT) $< 
<<


# Server
{$(SERVERDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<

# Various YAZ source directories
{$(ASNDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<

{$(COMSTACKDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<

{$(ODRDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<

{$(UTILDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<

{$(RETDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
<<


### Resource file
$(RESFILE): $(RCFILE) $(IDLGENERATED)
	$(RSC) $(RCOPT) /fo"$(RESFILE)" $(RCFILE) 


###########################################################
############### Linking
###########################################################


$(DLL) $(IMPLIB): "$(BINDIR)" $(DLL_OBJS) 
	$(LINK) @<<
            $(LNKOPT) 
		$(LINK_LIBS) 
		$(DLL_LINK_OPTIONS)
		$(DLL_OBJS) 
		/out:$(DLL) 
		/implib:$(IMPLIB)
            /pdb:"$(LIBDIR)/yaz.pdb" 
            /map:"$(LIBDIR)/yaz.map"  
<<

$(CLIENT) : "$(BINDIR)" $(YAZ_CLIENT_OBJS) $(IMPLIB)
	$(LINK) @<<
            $(LNKOPT) 
		$(CLIENT_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_CLIENT_OBJS) 
		/out:$(CLIENT) 
<<

$(ZTEST) : "$(BINDIR)" $(ZTEST_OBJS) $(SERVER) $(DLL)
	$(LINK) @<<
            $(LNKOPT) 
		$(ZTEST_LINK_OPTIONS)
		$(LINK_LIBS) 
		shell32.lib 
		$(IMPLIB)
		$(SERVER)
		$(ZTEST_OBJS) 
		/out:$(ZTEST) 
<<


$(SERVER) : "$(BINDIR)" $(YAZ_SERVER_OBJS) 
	$(LINK) $(SERVER_LINK_OPTIONS) @<<
		/nologo
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_SERVER_OBJS) 
		/out:$(SERVER) 
<<
# note that this links a lib, so it uses completely different options.

#	regsvr32 /s /c "$(DLL)" 
#		/def:$(DEF_FILE)


## Linking the debug info database (or what ever this is...)
$(BSCFILE): $(DLL_OBJS)
	$(BSCMAKE) $(BSCOPT) $(OBJDIR)\*.sbr


#	@echo OPT=$(LNKOPT)
#	@echo LIB=$(LINKLIBS)
#	@echo OBJ=$(DLL_OBJS)
#	@echo DEF=$(DEF_FILE)

###########################################################
############### Special operations
###########################################################


############## clean
clean:
	deltree /y "$(OBJDIR)/*.*"


########### check directories and create if needed
dirs: $(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR)

$(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) :
 	if not exist "$@/$(NUL)" mkdir "$@"

### test target while debugging makefile...
foo:  $(RESFILE)
	echo Ok

###########################################################
############### Explicit dependencies
###########################################################

$(OBJDIR)/client.obj: $(IDLGENERATED)

###########################################################
############### Log
###########################################################
#
# $Log: makefile.mak,v $
# Revision 1.3  1999-05-19 08:26:22  heikki
# Added comments
#
#



