##### paths #####
BINDIR    := ../../bin
INCDIR    := ../../inc
OBJDIR    := ../../obj
SRCDIR    := ../../src
INSINCDIR := /usr/local/include
INSLIBDIR := /usr/local/lib

##### misc #####
QUIETLY        := 1>/dev/null 2>/dev/null
VERSION_NUMBER := 1
MINOR_NUMBER   := 0
RELEASE_NUMBER := 0

##### sources, objects and libraries #####
BINNAME    := liblfds720
ARFILENAME := $(BINNAME).a
ARPATHNAME := $(BINDIR)/$(ARFILENAME)
SOBASENAME := $(BINNAME).so
SONAME     := $(SOBASENAME).$(VERSION_NUMBER)
SOFILENAME := $(SONAME).$(MINOR_NUMBER).$(RELEASE_NUMBER)
SOPATHNAME := $(BINDIR)/$(SOFILENAME)
INCNAME    := $(INCDIR)/$(BINNAME).h

SRCDIRS    := lfds720_btree_nodelete_unbalanced                                                          \
              lfds720_freelist                                                                           \
              lfds720_freelist_smrg                                                                      \
              lfds720_freelist_nodeallocate                                                              \
              lfds720_freelist_nodeallocate_positionindependent                                          \
              lfds720_hash_nodelete                                                                      \
              lfds720_list_nodelete_singlylinked_ordered                                                 \
              lfds720_list_nodelete_singlylinked_unordered                                               \
              lfds720_misc                                                                               \
              lfds720_pseudo_random_number_generator                                                     \
              lfds720_queue_bounded_manyproducer_manyconsumer                                            \
              lfds720_queue_bounded_singleproducer_singleconsumer                                        \
              lfds720_queue_nodeallocate_positionindependent_unbounded_manyproducer_manyconsumer         \
              lfds720_queue_nodeallocate_positionindependent_unbounded_singleproducer_singleconsumer     \
              lfds720_queue_nodeallocate_unbounded_manyproducer_manyconsumer                             \
              lfds720_queue_nodeallocate_unbounded_singleproducer_singleconsumer                         \
              lfds720_queue_unbounded_manyproducer_manyconsumer                                          \
              lfds720_ringbuffer_nodeallocate                                                            \
              lfds720_safe_memory_reclamation_generational                                               \
              lfds720_safe_memory_reclamation_hazard_pointers                                            \
              lfds720_stack_nodeallocate                                                                 \
              lfds720_stack_nodeallocate_positionindependent                                             \
              singlethreaded_data_structures_for_liblfds_internal_use/libstds/src/stds_freelist          \
              singlethreaded_data_structures_for_liblfds_internal_use/libstds/src/stds_list_du

SOURCES    := lfds720_btree_nu_cleanup.c lfds720_btree_nu_get.c lfds720_btree_nu_init.c lfds720_btree_nu_insert.c lfds720_btree_nu_query.c \
              lfds720_freelist_cleanup.c lfds720_freelist_init.c lfds720_freelist_pop.c lfds720_freelist_push.c lfds720_freelist_query.c lfds720_freelist_smr.c \
              lfds720_freelist_n_cleanup.c lfds720_freelist_n_elimination_array.c lfds720_freelist_n_init.c lfds720_freelist_n_pop.c lfds720_freelist_n_push.c lfds720_freelist_n_query.c \
              lfds720_freelist_np_cleanup.c lfds720_freelist_np_init.c lfds720_freelist_np_pop.c lfds720_freelist_np_push.c lfds720_freelist_np_query.c \
              lfds720_freelist_smrg_cleanup.c lfds720_freelist_smrg_init.c lfds720_freelist_smrg_pop.c lfds720_freelist_smrg_push.c lfds720_freelist_smrg_query.c lfds720_freelist_smrg_smr.c \
              lfds720_hash_n_cleanup.c lfds720_hash_n_get.c lfds720_hash_n_init.c lfds720_hash_n_insert.c lfds720_hash_n_iterate.c lfds720_hash_n_query.c \
              lfds720_list_nso_cleanup.c lfds720_list_nso_get.c lfds720_list_nso_init.c lfds720_list_nso_insert.c lfds720_list_nso_query.c \
              lfds720_list_nsu_cleanup.c lfds720_list_nsu_get.c lfds720_list_nsu_init.c lfds720_list_nsu_insert.c lfds720_list_nsu_query.c \
              lfds720_misc_internal_backoff_init.c lfds720_misc_globals.c lfds720_misc_query.c \
              lfds720_prng_init.c \
              lfds720_queue_bmm_cleanup.c lfds720_queue_bmm_dequeue.c lfds720_queue_bmm_enqueue.c lfds720_queue_bmm_init.c lfds720_queue_bmm_query.c \
              lfds720_queue_bss_cleanup.c lfds720_queue_bss_dequeue.c lfds720_queue_bss_enqueue.c lfds720_queue_bss_init.c lfds720_queue_bss_query.c \
              lfds720_queue_npumm_cleanup.c lfds720_queue_npumm_dequeue.c lfds720_queue_npumm_enqueue.c lfds720_queue_npumm_init.c lfds720_queue_npumm_query.c \
              lfds720_queue_npuss_cleanup.c lfds720_queue_npuss_dequeue.c lfds720_queue_npuss_enqueue.c lfds720_queue_npuss_flush.c lfds720_queue_npuss_init.c lfds720_queue_npuss_query.c \
              lfds720_queue_numm_cleanup.c lfds720_queue_numm_dequeue.c lfds720_queue_numm_enqueue.c lfds720_queue_numm_init.c lfds720_queue_numm_query.c \
              lfds720_queue_nuss_cleanup.c lfds720_queue_nuss_dequeue.c lfds720_queue_nuss_enqueue.c lfds720_queue_nuss_flush.c lfds720_queue_nuss_init.c lfds720_queue_nuss_query.c \
              lfds720_queue_umm_cleanup.c lfds720_queue_umm_dequeue.c lfds720_queue_umm_enqueue.c lfds720_queue_umm_init.c lfds720_queue_umm_query.c lfds720_queue_umm_smr.c \
              lfds720_ringbuffer_n_cleanup.c lfds720_ringbuffer_n_init.c lfds720_ringbuffer_n_query.c lfds720_ringbuffer_n_read.c lfds720_ringbuffer_n_write.c \
              lfds720_smrg_clean.c lfds720_smrg_flush.c lfds720_smrg_init_and_cleanup.c lfds720_smrg_query.c lfds720_smrg_register.c lfds720_smrg_release.c lfds720_smrg_submit.c \
              lfds720_smrhp_clean.c lfds720_smrhp_init_and_cleanup.c lfds720_smrhp_query.c lfds720_smrhp_register.c lfds720_smrhp_submit.c \
              lfds720_stack_n_cleanup.c lfds720_stack_n_init.c lfds720_stack_n_pop.c lfds720_stack_n_push.c lfds720_stack_n_query.c \
              lfds720_stack_np_cleanup.c lfds720_stack_np_init.c lfds720_stack_np_pop.c lfds720_stack_np_push.c lfds720_stack_np_query.c \
              stds_freelist_cleanup.c stds_freelist_init.c stds_freelist_push.c stds_freelist_query.c \
              stds_list_du_cleanup.c stds_list_du_init.c stds_list_du_insert.c stds_list_du_query.c stds_list_du_remove.c

OBJECTS    := $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SOURCES)))
SYSLIBS    := 

##### tools #####
DG                     := gcc
DGFLAGS_MANDATORY      := -MM
DGFLAGS_OPTIONAL       := -std=gnu89

CC                     := gcc
CFLAGS_MANDATORY       := -c -fno-strict-aliasing -Wno-pointer-to-int-cast
# CFLAGS_OPTIONAL        := -ffreestanding -nostdinc -std=gnu89 -Wall -Werror -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-uninitialized
CFLAGS_OPTIONAL        := -std=gnu89 -Wall -Werror -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-uninitialized
CFLAGS_MANDATORY_COV   := -O0 -ggdb -DCOVERAGE -fprofile-arcs -ftest-coverage
CFLAGS_MANDATORY_DBG   := -O0 -ggdb -D_DEBUG
CFLAGS_MANDATORY_PROF  := -O0 -ggdb -DPROF     -pg
CFLAGS_MANDATORY_REL   := -O3       -DNDEBUG
CFLAGS_MANDATORY_TSAN  := -O0 -ggdb -DTSAN     -fsanitize=thread -fPIC

AR                     := ar
ARFLAGS                :=
ARFLAGS_MANDATORY      := rcs
ARFLAGS_OPTIONAL       :=

LD                     := gcc
LDFLAGS_MANDATORY      := -shared -Wl,-soname,$(SONAME) -o $(SOPATHNAME)
LDFLAGS_OPTIONAL       := -nodefaultlibs -nostdlib -std=gnu89 -Wall -Werror
LDFLAGS_MANDATORY_COV  := -O0 -fprofile-arcs -ftest-coverage
LDFLAGS_MANDATORY_DBG  := -O0 -ggdb
LDFLAGS_MANDATORY_PROF := -O0 -pg
LDFLAGS_MANDATORY_REL  := -O3 -s
LDFLAGS_MANDATORY_TSAN := -O0 -fsanitize=thread -fPIC

##### defaults #####
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS := so_rel
endif

##### build variants #####
ifeq ($(findstring so,$(MAKECMDGOALS)),so)
  CFLAGS_MANDATORY += -fPIC
endif

ifeq ($(findstring cov,$(MAKECMDGOALS)),cov)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_COV)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_COV)
  SYSLIBS += -lgcov
endif

ifeq ($(findstring dbg,$(MAKECMDGOALS)),dbg)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_DBG)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_DBG)
endif

ifeq ($(findstring prof,$(MAKECMDGOALS)),prof)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_PROF)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_PROF)
endif

ifeq ($(findstring rel,$(MAKECMDGOALS)),rel)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_REL)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_REL)
endif

ifeq ($(findstring tsan,$(MAKECMDGOALS)),tsan)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_TSAN)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_TSAN)
endif

##### search paths #####
vpath %.c $(patsubst %,$(SRCDIR)/%:,$(SRCDIRS))

##### implicit rules #####
$(OBJDIR)/%.o : %.c
	$(DG) $(DGFLAGS_OPTIONAL) $(DGFLAGS) $(DGFLAGS_MANDATORY) $< >$(OBJDIR)/$*.d
	$(CC) $(CFLAGS_OPTIONAL) $(CFLAGS) $(CFLAGS_MANDATORY) -o $@ $<

##### explicit rules #####
$(ARPATHNAME) : $(OBJECTS)
	$(AR) $(ARFLAGS_OPTIONAL) $(ARFLAGS) $(ARFLAGS_MANDATORY) $(ARPATHNAME) $(OBJECTS)

$(SOPATHNAME) : $(OBJECTS)
	$(LD) $(LDFLAGS_OPTIONAL) $(LDFLAGS) $(LDFLAGS_MANDATORY) $(OBJECTS) -lgcov -lgcc -o $(SOPATHNAME)
	@ln -fs $(SOFILENAME) $(BINDIR)/$(SONAME)
	@ln -fs $(SOFILENAME) $(BINDIR)/$(SOBASENAME)

##### phony #####
.PHONY : clean ar_cov ar_dbg ar_prof ar_rel ar_tsan ar_vanilla ar_install ar_uninstall so_dbg so_prof so_rel so_tsan so_vanilla so_install so_uninstall

clean : 
	@rm -f $(BINDIR)/* $(OBJDIR)/*

ar_cov       : $(ARPATHNAME) # archive (.a), coverage
ar_dbg       : $(ARPATHNAME) # archive (.a), debug
ar_prof      : $(ARPATHNAME) # archive (.a), profiling
ar_rel       : $(ARPATHNAME) # archive (.a), release
ar_tsan      : $(ARPATHNAME) # archive (.a), thread sanitizer
ar_vanilla   : $(ARPATHNAME) # archive (.a), no specific-build arguments
ar_install   :
  # TRD : leading backslash to use command rather than alias
  #       as many Linux distros have a built-in alias to force
  #       a prompt ("y/n?") on file overwrite - silent and
  #       unexpected interference which breaks a makefile
	@mkdir -p $(INSLIBDIR)
	@\cp $(ARPATHNAME) $(INSLIBDIR)
	@mkdir -p $(INSINCDIR)
	@\cp -r $(INCDIR)/* $(INSINCDIR)
ar_uninstall :
	@rm    $(INSLIBDIR)/$(ARFILENAME)
	@rm -r $(INSINCDIR)/$(BINNAME)
	@rm -r $(INSINCDIR)/$(BINNAME).h

# TRD : so_cov currently disabled as it cannot work with -nostdlib -nodefaultlibs
# so_cov       : $(SOPATHNAME) # shared (.so), coverage
so_dbg       : $(SOPATHNAME) # shared (.so), debug
so_prof      : $(SOPATHNAME) # shared (.so), profiling
so_rel       : $(SOPATHNAME) # shared (.so), release
so_tsan      : $(SOPATHNAME) # shared (.so), thread sanitizer
so_vanilla   : $(SOPATHNAME) # shared (.so), no specific-build arguments
so_install   : 
	@mkdir -p $(INSINCDIR)
	@\cp $(SOPATHNAME) $(INSLIBDIR)
	@ldconfig -vn $(INSLIBDIR)
	@ln -s $(SONAME) $(INSLIBDIR)/$(SOBASENAME)
	@mkdir -p $(INSLIBDIR)
	@\cp -r $(INCDIR)/* $(INSINCDIR)
so_uninstall : 
	@rm -f $(INSLIBDIR)/$(SOFILENAME)
	@rm -f $(INSLIBDIR)/$(SOBASENAME)
	@rm -f $(INSLIBDIR)/$(SONAME)
	@rm -r $(INSINCDIR)/$(BINNAME)
	@rm -r $(INSINCDIR)/$(BINNAME).h

##### dependencies #####
-include $(DEPENDS)

