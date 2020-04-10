##### paths #####
BINDIR := ../../bin
SRCDIR := ../../src

##### misc #####
QUIETLY := 1>/dev/null 2>/dev/null

##### tools #####
MAKE := make
COPY := cp
RM   := rm -f

##### defaults #####
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS := ar_rel
endif

##### variants #####
BINVAR := $(subst ar_,,$(MAKECMDGOALS))
BINVAR := $(subst so_,,$(BINVAR))

##### targets #####
makefiles : 
	$(MAKE) -C $(SRCDIR)/liblfds720/build/gcc-gnumake        $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libshared/build/gcc-gnumake         $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libtest/build/gcc-gnumake           $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/libbenchmark/build/gcc-gnumake      $(MAKECMDGOALS)
	$(MAKE) -C $(SRCDIR)/test/build/gcc-gnumake-hosted       $(BINVAR)
	$(MAKE) -C $(SRCDIR)/benchmark/build/gcc-gnumake-hosted  $(BINVAR)

##### phony #####
.PHONY : makefiles clean ar_cov ar_dbg ar_prof ar_rel ar_tsan ar_vanilla so_dbg so_prof so_rel so_tsan so_vanilla

clean : makefiles
	$(RM) $(BINDIR)/* $(QUIETLY)

ar_cov ar_dbg ar_prof ar_rel ar_tsan ar_vanilla so_dbg so_prof so_rel so_tsan so_vanilla : makefiles
	$(COPY) $(SRCDIR)/liblfds720/bin/*   $(BINDIR)
	$(COPY) $(SRCDIR)/libshared/bin/*    $(BINDIR)
	$(COPY) $(SRCDIR)/libtest/bin/*      $(BINDIR)
	$(COPY) $(SRCDIR)/libbenchmark/bin/* $(BINDIR)
	$(COPY) $(SRCDIR)/test/bin/*         $(BINDIR)
	$(COPY) $(SRCDIR)/benchmark/bin/*    $(BINDIR)

##### notes #####
# TRD : so_cov is an invalid combination, which is why it is omitted

