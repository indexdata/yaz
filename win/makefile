# Makefile.mak - makefile for MS NMAKE 
# $Id: makefile,v 1.3 1999-06-08 14:32:30 heikki Exp $
#
# Programmed by
#  HL: Heikki Levanto, Index Data
#
# Log at the end of the file
#
# Missing
# - Move MS-C's whatnots into win direcotry
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

NEW_Z3950=0  # 0= use old asn files
             # 1= generate files from *.asn (needs tcl)


default: all

all: dirs proto_h dll client server ztest 


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

CLIENT=$(BINDIR)\client.exe
SERVER=$(LIBDIR)\server.lib
ZTEST=$(BINDIR)\ztest.exe
PROTOH=$(INCLDIR)\proto.h

# shortcut names defined here
dll : $(DLL) 
client: $(CLIENT)
server: $(SERVER)
ztest: $(ZTEST)
proto_h: $(PROTOH)

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


# Final opt variables
!if $(DEBUG)
COPT=   $(COMMON_C_OPTIONS)   $(DEBUG_C_OPTIONS)     $(COMMON_C_INCLUDES)
MTLOPT= $(COMMON_MTL_OPTIONS) $(DEBUG_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(DEBUG_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(DEBUG_LNK_OPTIONS)   $(LNK_LIBS)

!else
COPT=   $(COMMON_C_OPTIONS)   $(RELEASE_C_OPTIONS)   $(COMMON_C_INCLUDES) 
MTLOPT= $(COMMON_MTL_OPTIONS) $(RELEASE_MTL_OPTIONS)
RCOPT=  $(COMMON_RC_OPTIONS)  $(RELEASE_RC_OPTIONS)
LNKOPT= $(COMMON_LNK_OPTIONS) $(RELEASE_LNK_OPTIONS) $(LNK_LIBS)
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
   $(OBJDIR)\proto.obj \
   $(OBJDIR)\prt-acc.obj \
   $(OBJDIR)\prt-add.obj \
   $(OBJDIR)\prt-arc.obj \
   $(OBJDIR)\prt-dat.obj \
   $(OBJDIR)\prt-dia.obj \
   $(OBJDIR)\prt-esp.obj \
   $(OBJDIR)\prt-exd.obj \
   $(OBJDIR)\prt-exp.obj \
   $(OBJDIR)\prt-grs.obj \
   $(OBJDIR)\prt-rsc.obj \
   $(OBJDIR)\prt-univ.obj 

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
   $(OBJDIR)\prt-ext.obj \
   $(OBJDIR)\logrpn.obj \
   $(OBJDIR)\pquery.obj \
   $(OBJDIR)\query.obj \
   $(OBJDIR)\yaz-ccl.obj \
   $(OBJDIR)\otherinfo.obj

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
   $(YAZ_ZUTIL_OBJS) \
   $(YAZ_RET_OBJS)

DLL_OBJS= $(YAZ_OBJS)


##########################################################
############## proto.h
##########################################################

!if $(NEW_Z3950)
$(PROTOH):
	copy $(INCLDIR)\z-proto.h $(INCLDIR)\proto.h
!else
$(PROTOH):
	copy $(INCLDIR)\prt-proto.h $(INCLDIR)\proto.h
!endif



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
	$(COPT) $< 
	/D"_CONSOLE"
<<

# Ztest
{$(ZTESTDIR)}.c{$(OBJDIR)}.obj:
	@$(CPP) @<<  
	$(COPT) $< 
      /D"_CONSOLE"
	/D"_MBCS"
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

{$(ZUTILDIR)}.c{$(OBJDIR)}.obj:
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
	@echo Linking the dll  $(DLL)
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

$(CLIENT) : "$(BINDIR)" $(YAZ_CLIENT_OBJS) #####$(IMPLIB)
	@echo Linking the client  $(CLIENT)
	$(LINK) @<<
            $(LNKOPT) 
		$(CLIENT_LINK_OPTIONS)
		$(LINK_LIBS) 
		$(IMPLIB)
		$(YAZ_CLIENT_OBJS) 
		/out:$(CLIENT) 
<<

$(ZTEST) : "$(BINDIR)" $(ZTEST_OBJS) $(SERVER) $(DLL)
	@echo Linking the ztest  $(ZTEST)
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
	@echo Linking the server  $(SERVER)
	$(LINK) $(SERVER_LINK_OPTIONS) @<<
		/nologo
		$(IMPLIB)
		$(YAZ_SERVER_OBJS) 
		/out:$(SERVER) 
<<

# 		$(LINK_LIBS) 

# note that this links a lib, so it uses completely different options.



###########################################################
############### Special operations
###########################################################


############## clean
clean:
	del $(OBJDIR)\*.obj
	del $(OBJDIR)\*.sbr
	del $(DLL) 
	del $(CLIENT)
	del $(SERVER)
	del $(ZTEST)
	del $(TMPDIR)\*.

########### check directories and create if needed
dirs: $(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR)

$(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR):
 	if not exist "$@/$(NUL)" mkdir "$@"


###########################################################
############### Explicit dependencies
###########################################################

$(OBJDIR)/client.obj: $(IDLGENERATED)

$(DLL_OBJS): makefile $(PROTOH)


# Debug test
foo: $(OBJDIR)\ztest.obj

###########################################################
############### Log
###########################################################
#
# $Log: makefile,v $
# Revision 1.3  1999-06-08 14:32:30  heikki
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



