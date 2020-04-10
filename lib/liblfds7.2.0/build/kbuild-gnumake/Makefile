##### paths #####
BINDIR := ../../bin
SRCDIR := ../../src

##### misc #####
QUIETLY := 1>/dev/null 2>/dev/null

##### tools #####
MAKE := make
COPY := cp
RM   := rm -f

##### phony #####
.PHONY : default clean help modules

clean      :
	$(MAKE) -C $(SRCDIR)/liblfds720/build/kbuild-gnumake   $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libshared/build/kbuild-gnumake    $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libtest/build/kbuild-gnumake      $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libbenchmark/build/kbuild-gnumake $(MAKECMDGOALS)
	$(RM) $(BINDIR)/* $(QUIETLY)

# TRD : so_cov does not work
ar_cov ar_dbg ar_prof ar_rel ar_tsan ar_vanilla so_dbg so_prof so_rel so_tsan so_vanilla :
	$(MAKE) -C $(SRCDIR)/liblfds720/build/kbuild-gnumake   $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libshared/build/kbuild-gnumake    $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libtest/build/kbuild-gnumake      $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libbenchmark/build/kbuild-kbuild  $(MAKECMDGOALS)
	$(COPY) $(SRCDIR)/liblfds720/bin/*   $(BINDIR)
	$(COPY) $(SRCDIR)/libshared/bin/*    $(BINDIR)
	$(COPY) $(SRCDIR)/libtest/bin/*      $(BINDIR)
	$(COPY) $(SRCDIR)/libbenchmark/bin/* $(BINDIR)

