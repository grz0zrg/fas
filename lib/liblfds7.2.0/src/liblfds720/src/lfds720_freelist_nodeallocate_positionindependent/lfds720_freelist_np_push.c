/***** includes *****/
#include "lfds720_freelist_np_internal.h"





/****************************************************************************/
void lfds720_freelist_np_threadsafe_push( struct lfds720_freelist_np_state *fs,
                                          struct lfds720_freelist_np_element *fe )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  ptrdiff_t LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    new_top[LFDS720_MISC_PAC_SIZE];

  ptrdiff_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    original_top[LFDS720_MISC_PAC_SIZE];

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( fe != NULL );

  new_top[LFDS720_MISC_OFFSET] = LFDS720_MISC_POINTER_TO_OFFSET( fs, fe );

  original_top[LFDS720_MISC_COUNTER] = fs->top[LFDS720_MISC_COUNTER];
  original_top[LFDS720_MISC_OFFSET]  = fs->top[LFDS720_MISC_OFFSET];

  do
  {
    fe->next = original_top[LFDS720_MISC_OFFSET];
    LFDS720_MISC_BARRIER_STORE;

    new_top[LFDS720_MISC_COUNTER] = original_top[LFDS720_MISC_COUNTER] + 1;
    LFDS720_PAL_ATOMIC_DWCAS( fs->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( fs->push_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_BACKOFF_AUTOTUNE( fs->push_backoff, backoff_iteration );

  return;
}

