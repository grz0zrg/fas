/***** includes *****/
#include "lfds720_freelist_smrg_internal.h"





/****************************************************************************/
void lfds720_freelist_smrg_push_clean_element( struct lfds720_freelist_smrg_state *fsgs,
                                               struct lfds720_freelist_smrg_element *fsge,
                                               struct lfds720_smrg_thread_state *smrgts )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_freelist_smrg_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *new_top,
    *original_top;

  LFDS720_PAL_ASSERT( fsgs != NULL );
  LFDS720_PAL_ASSERT( fsge != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );

  LFDS720_SMRG_THREAD_BEGIN_LOCKFREE_OPERATIONS( *smrgts );

  new_top = fsge;

  original_top = fsgs->top;

  do
  {
    fsge->next = original_top;
    LFDS720_MISC_BARRIER_STORE;

    LFDS720_PAL_ATOMIC_CAS( fsgs->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( fsgs->push_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_SMRG_THREAD_END_LOCKFREE_OPERATIONS( *smrgts );

  LFDS720_BACKOFF_AUTOTUNE( fsgs->push_backoff, backoff_iteration );

  return;
}

