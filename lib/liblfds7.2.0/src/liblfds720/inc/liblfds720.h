#ifndef LIBLFDS720_H

  /***** defines *****/
  #define LIBLFDS720_H

  /***** pragmas on *****/
  #pragma warning( push )
  #pragma warning( disable : 4324 )                                                        // TRD : 4324 disables MSVC warnings for structure alignment padding due to alignment specifiers
  #pragma prefast( disable : 28113 28182 28183, "blah" )

  /***** includes *****/
  #include "liblfds720/lfds720_porting_abstraction_layer_processor.h"
  #include "liblfds720/lfds720_porting_abstraction_layer_operating_system.h"
  #include "liblfds720/lfds720_porting_abstraction_layer_compiler.h"

  #include "liblfds720/lfds720_pseudo_random_number_generator.h"                           // TRD : needed by misc
  #include "liblfds720/lfds720_misc.h"                                                     // TRD : needed by all data structures

  #include "liblfds720/singlethreaded_data_structures_for_liblfds_internal_use/libstds.h"  // TRD : needed by SMRs
  #include "liblfds720/lfds720_list_nodelete_singlylinked_unordered.h"
  #include "liblfds720/lfds720_safe_memory_reclamation_generational.h"                     // TRD : needed by all SMRG data structures
  #include "liblfds720/lfds720_safe_memory_reclamation_hazard_pointers.h"                  // TRD : needed by all SMRHP data structures

  #include "liblfds720/lfds720_btree_nodelete_unbalanced.h"                                // TRD : needed by hash_nodelete
  #include "liblfds720/lfds720_freelist.h"
  #include "liblfds720/lfds720_freelist_nodeallocate.h"
  #include "liblfds720/lfds720_freelist_nodeallocate_positionindependent.h"
  #include "liblfds720/lfds720_freelist_smrg.h"
  #include "liblfds720/lfds720_hash_nodelete.h"
  #include "liblfds720/lfds720_list_nodelete_singlylinked_ordered.h"
  // #include "liblfds720/lfds720_list_singlylinked_ordered.h"
  #include "liblfds720/lfds720_queue_bounded_manyproducer_manyconsumer.h"
  #include "liblfds720/lfds720_queue_bounded_singleproducer_singleconsumer.h"
  #include "liblfds720/lfds720_queue_nodeallocate_positionindependent_unbounded_manyproducer_manyconsumer.h"
  #include "liblfds720/lfds720_queue_nodeallocate_positionindependent_unbounded_singleproducer_singleconsumer.h"
  #include "liblfds720/lfds720_queue_nodeallocate_unbounded_manyproducer_manyconsumer.h"
  #include "liblfds720/lfds720_queue_nodeallocate_unbounded_singleproducer_singleconsumer.h"
  #include "liblfds720/lfds720_queue_unbounded_manyproducer_manyconsumer.h"
  #include "liblfds720/lfds720_ringbuffer_nodeallocate.h"
  #include "liblfds720/lfds720_stack_nodeallocate.h"
  #include "liblfds720/lfds720_stack_nodeallocate_positionindependent.h"

  /***** pragmas off *****/
  #pragma warning( pop )

#endif

