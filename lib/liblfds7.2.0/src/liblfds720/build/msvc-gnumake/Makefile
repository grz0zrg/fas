##### paths #####
BINDIR := ..\..\bin
INCDIR := ..\..\inc
OBJDIR := ..\..\obj
SRCDIR := ..\..\src

##### misc #####
QUIETLY := 1>nul 2>nul
NULL := 
SPACE := $(NULL) # TRD : with a trailing space

##### sources, objects and libraries #####
BINNAME    := liblfds720
LIB_BINARY := $(BINDIR)\$(BINNAME).lib
DLL_BINARY := $(BINDIR)\$(BINNAME).dll
SRCDIRS    := lfds720_btree_addonly_unbalanced                                                           \
              lfds720_freelist                                                                           \
              lfds720_freelist_safe_memory_reclamation_generational                                      \
              lfds720_freelist_safe_memory_reclamation_hazard_pointers                                   \
              lfds720_hash_nddonly                                                                       \
              lfds720_list_addonly_singlylinked_ordered                                                  \
              lfds720_list_addonly_singlylinked_unordered                                                \
              lfds720_misc                                                                               \
              lfds720_pseudo_random_number_generator                                                     \
              lfds720_queue_bounded_manyproducer_manyconsumer                                            \
              lfds720_queue_bounded_singleproducer_singleconsumer                                        \
              lfds720_queue_unbounded_manyproducer_manyconsumer                                          \
              lfds720_queue_unbounded_manyproducer_manyconsumer_safe_memory_reclamation_hazard_pointers  \
              lfds720_queue_unbounded_singleproducer_singleconsumer                                      \
              lfds720_ringbuffer_n                                                                         \
              lfds720_safe_memory_reclamation_generational                                               \
              lfds720_safe_memory_reclamation_hazard_pointers                                            \
              lfds720_stack                                                                              \
              lfds720_stack_safe_memory_reclamation_generational                                         \
              singlethreaded_data_structures_for_liblfds_internal_use/libstds/src/stds_freelist          \
              singlethreaded_data_structures_for_liblfds_internal_use/libstds/src/stds_list_du
SOURCES    := lfds720_btree_nu_cleanup.c lfds720_btree_nu_get.c lfds720_btree_nu_init.c lfds720_btree_nu_insert.c lfds720_btree_nu_query.c \
              lfds720_freelist_cleanup.c lfds720_freelist_init.c lfds720_freelist_pop.c lfds720_freelist_push.c lfds720_freelist_query.c \
              lfds720_freelist_smrg_cleanup.c lfds720_freelist_smrg_init.c lfds720_freelist_smrg_pop.c lfds720_freelist_smrg_push.c lfds720_freelist_smrg_query.c lfds720_freelist_smrg_smr.c \
              lfds720_freelist_smrhp_cleanup.c lfds720_freelist_smrhp_init.c lfds720_freelist_smrhp_pop.c lfds720_freelist_smrhp_push.c lfds720_freelist_smrhp_query.c lfds720_freelist_smrhp_smr.c \
              lfds720_hash_n_cleanup.c lfds720_hash_n_get.c lfds720_hash_n_init.c lfds720_hash_n_insert.c lfds720_hash_n_iterate.c lfds720_hash_n_query.c \
              lfds720_list_nso_cleanup.c lfds720_list_nso_get.c lfds720_list_nso_init.c lfds720_list_nso_insert.c lfds720_list_nso_query.c \
              lfds720_list_nsu_cleanup.c lfds720_list_nsu_get.c lfds720_list_nsu_init.c lfds720_list_nsu_insert.c lfds720_list_nsu_query.c \
              lfds720_misc_internal_backoff_init.c lfds720_misc_globals.c lfds720_misc_query.c \
              lfds720_prng_init.c \
              lfds720_queue_bmm_cleanup.c lfds720_queue_bmm_dequeue.c lfds720_queue_bmm_enqueue.c lfds720_queue_bmm_init.c lfds720_queue_bmm_query.c \
              lfds720_queue_bss_cleanup.c lfds720_queue_bss_dequeue.c lfds720_queue_bss_enqueue.c lfds720_queue_bss_init.c lfds720_queue_bss_query.c \
              lfds720_queue_numm_cleanup.c lfds720_queue_numm_dequeue.c lfds720_queue_numm_enqueue.c lfds720_queue_numm_init.c lfds720_queue_numm_query.c \
              lfds720_queue_numm_smrhp_cleanup.c lfds720_queue_numm_smrhp_dequeue.c lfds720_queue_numm_smrhp_enqueue.c lfds720_queue_numm_smrhp_init.c lfds720_queue_numm_smrhp_query.c lfds720_queue_numm_smrhp_smr.c \
              lfds720_queue_nuss_cleanup.c lfds720_queue_nuss_dequeue.c lfds720_queue_nuss_enqueue.c lfds720_queue_nuss_flush.c lfds720_queue_nuss_init.c lfds720_queue_nuss_query.c \
              lfds720_ringbuffer_n_cleanup.c lfds720_ringbuffer_n_init.c lfds720_ringbuffer_n_query.c lfds720_ringbuffer_n_read.c lfds720_ringbuffer_n_write.c \
              lfds720_smrg_clean.c lfds720_smrg_flush.c lfds720_smrg_init_and_cleanup.c lfds720_smrg_query.c lfds720_smrg_register.c lfds720_smrg_release.c lfds720_smrg_submit.c \
              lfds720_smrhp_clean.c lfds720_smrhp_init_and_cleanup.c lfds720_smrhp_register.c lfds720_smrhp_submit.c \
              lfds720_stack_cleanup.c lfds720_stack_init.c lfds720_stack_pop.c lfds720_stack_push.c lfds720_stack_query.c \
              lfds720_stack_smrg_cleanup.c lfds720_stack_smrg_init.c lfds720_stack_smrg_pop.c lfds720_stack_smrg_push.c lfds720_stack_smrg_query.c lfds720_stack_smrg_smr.c \
              stds_freelist_cleanup.c stds_freelist_init.c stds_freelist_push.c stds_freelist_query.c \
              stds_list_du_cleanup.c stds_list_du_init.c stds_list_du_insert.c stds_list_du_query.c stds_list_du_remove.c
OBJECTS    := $(patsubst %.c,$(OBJDIR)/%.obj,$(notdir $(SOURCES)))
SYSLIBS    := kernel32.lib

##### default paths fix up #####
INCDIRS := $(patsubst %,%;,$(INCDIR))
INCLUDE += $(subst $(SPACE),,$(INCDIRS))

##### tools #####
CC                    := cl
CFLAGS_MANDATORY      := /c "/Fd$(BINDIR)\$(BINNAME).pdb" /wd 4068
CFLAGS_OPTIONAL       := /DWIN32_LEAN_AND_MEAN /DUNICODE /D_UNICODE /nologo /W4 /WX
CFLAGS_MANDATORY_DBG  := /Od /Gm /Zi /D_DEBUG
CFLAGS_MANDATORY_REL  := /Ox /DNDEBUG

AR                    := lib
ARFLAGS               :=
ARFLAGS_MANDATORY     := /subsystem:console
ARFLAGS_OPTIONAL      := /nologo /wx /verbose

LD                    := link
LDFLAGS_MANDATORY     := /def:$(BINNAME).def /dll /nodefaultlib /subsystem:console
LDFLAGS_OPTIONAL      := /nologo /nxcompat /wx
LDFLAGS_MANDATORY_DBG := /debug "/pdb:$(BINDIR)\$(BINNAME).pdb"
LDFLAGS_MANDATORY_REL := /incremental:no

##### defaults #####
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS := dll_rel
endif

##### variants #####
ifeq ($(MAKECMDGOALS),lib_dbg)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_DBG)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_DBG)
  CLIB              := libcmtd.lib
endif

ifeq ($(MAKECMDGOALS),lib_rel)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_REL)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_REL)
  CLIB              := libcmt.lib
endif

ifeq ($(MAKECMDGOALS),dll_dbg)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_DBG)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_DBG)
  CLIB              := msvcrtd.lib
endif

ifeq ($(MAKECMDGOALS),dll_rel)
  CFLAGS_MANDATORY  += $(CFLAGS_MANDATORY_REL)
  LDFLAGS_MANDATORY += $(LDFLAGS_MANDATORY_REL)
  CLIB              := msvcrt.lib
endif

##### search paths #####
vpath %.c $(patsubst %,$(SRCDIR)/%;,$(SRCDIRS))

##### implicit rules #####
$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS_OPTIONAL) $(CFLAGS) $(CFLAGS_MANDATORY) "/Fo$@" $<

##### explicit rules #####
$(LIB_BINARY) : $(OBJECTS)
	$(AR) $(ARFLAGS_OPTIONAL) $(ARFLAGS) $(ARFLAGS_MANDATORY) $(OBJECTS) /out:$(LIB_BINARY)

$(DLL_BINARY) : $(OBJECTS)
	$(LD) $(LDFLAGS_OPTIONAL) $(LDFLAGS) $(LDFLAGS_MANDATORY) $(CLIB) $(SYSLIBS) $(OBJECTS) /out:$(DLL_BINARY)

##### phony #####
.PHONY : clean lib_rel lib_dbg dll_rel dll_dbg

clean : 
	@erase /Q $(BINDIR)\$(BINNAME).* $(OBJDIR)\*.obj $(QUIETLY)

dll_rel : $(DLL_BINARY)
dll_dbg : $(DLL_BINARY)

lib_rel : $(LIB_BINARY)
lib_dbg : $(LIB_BINARY)

##### notes #####
# /wd 4068 : turn off "unknown pragma" warning

