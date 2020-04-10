##### paths #####
BINDIR   := ..\..\bin
SRCDIR   := ..\..\src
# TRD : gnuwin32 make -C arg is broken, needs forward slashes
SRCDIRFS := ../../src

##### misc #####
QUIETLY := 1>nul 2>nul

##### tools #####
MAKE := make
COPY := copy
RM   := erase /q

##### defaults #####
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS := lib_rel
endif

##### targets #####
makefiles : 
	$(MAKE) -C $(SRCDIRFS)/liblfds720/build/msvc-gnumake    $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIRFS)/libshared/build/msvc-gnumake     $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIRFS)/libtest/build/msvc-gnumake       $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIRFS)/libbenchmark/build/msvc-gnumake  $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIRFS)/test/build/msvc-gnumake          $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIRFS)/benchmark/build/msvc-gnumake     $(MAKECMDGOALS)

##### phony #####
.PHONY : clean lib_rel lib_dbg dll_rel dll_dbg

clean : makefiles
	$(RM) $(BINDIR)\* $(QUIETLY)

lib_rel lib_dbg dll_rel dll_dbg : makefiles
	$(COPY) $(SRCDIR)\liblfds720\bin\*    $(BINDIR)
	$(COPY) $(SRCDIR)\libshared\bin\*     $(BINDIR)
	$(COPY) $(SRCDIR)\libtest\bin\*       $(BINDIR)
	$(COPY) $(SRCDIR)\libbenchmark\bin\*  $(BINDIR)
	$(COPY) $(SRCDIR)\test\bin\*          $(BINDIR)
	$(COPY) $(SRCDIR)\benchmark\bin\*     $(BINDIR)

