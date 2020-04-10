##### paths #####
BINDIR := ..\..\bin

##### misc #####
QUIETLY := 1>nul 2>nul
NULL := 
SPACE := $(NULL) # TRD : with a trailing space

##### sources, objects and libraries #####
BINNAME    := liblfds720
LIB_BINARY := $(BINDIR)\$(BINNAME).lib
DLL_BINARY := $(BINDIR)\$(BINNAME).dll

##### tools #####
BUILD := build

##### defaults #####
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS := dll_rel
endif

##### explicit rules #####
$(LIB_BINARY) : 
	@runme_before_win_kernel_static_lib_build $(QUIETLY)
	$(BUILD)

$(DLL_BINARY) : 
	@runme_before_win_kernel_dynamic_lib_build $(QUIETLY)
	$(BUILD)

##### phony #####
.PHONY : clean lib_rel dll_rel

clean : 
	# @erase /Q $(BINDIR)\$(BINNAME).* $(OBJDIR)\*.obj $(QUIETLY)
	$(BUILD) clean

dll_rel : $(DLL_BINARY)

lib_rel : $(LIB_BINARY)

