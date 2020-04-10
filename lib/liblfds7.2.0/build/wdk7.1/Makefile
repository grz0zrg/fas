##### paths #####
BINDIR := ../../bin
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
	$(MAKE) -C $(SRCDIR)/liblfds720/build/wdk7.1   $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libshared/build/wdk7.1    $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libtest/build/wdk7.1      $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libbenchmark/build/wdk7.1 $(MAKECMDGOALS)

##### phony #####
.PHONY : clean dll_rel lib_rel

clean : makefiles
	$(RM) -f $(BINDIR)\* $(QUIETLY)

lib_rel dll_rel : makefiles
	$(COPY) $(SRCDIR)\liblfds720\bin\*   $(BINDIR)
	$(COPY) $(SRCDIR)\libshared\bin\*    $(BINDIR)
	$(COPY) $(SRCDIR)\libtest\bin\*      $(BINDIR)
	$(COPY) $(SRCDIR)\libbenchmark\bin\* $(BINDIR)

