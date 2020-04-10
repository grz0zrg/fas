/***** includes *****/
#include "lfds720_stack_np_internal.h"





/****************************************************************************/
int lfds720_stack_np_threadsafe_pop( struct lfds720_stack_np_state *ss,
                                     struct lfds720_stack_np_element **se )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  ptrdiff_t LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    new_top[LFDS720_MISC_PAC_SIZE];

  ptrdiff_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    original_top[LFDS720_MISC_PAC_SIZE];

  LFDS720_PAL_ASSERT( ss != NULL );
  LFDS720_PAL_ASSERT( se != NULL );

  original_top[LFDS720_MISC_COUNTER] = ss->top[LFDS720_MISC_COUNTER];
  original_top[LFDS720_MISC_OFFSET]  = ss->top[LFDS720_MISC_OFFSET];

  do
  {
    if( original_top[LFDS720_MISC_OFFSET] == 0 )
    {
      *se = NULL;
      return 0;
    }

    new_top[LFDS720_MISC_COUNTER] = original_top[LFDS720_MISC_COUNTER] + 1;
    new_top[LFDS720_MISC_OFFSET]  = LFDS720_MISC_OFFSET_TO_POINTER( ss, original_top[LFDS720_MISC_OFFSET], struct lfds720_stack_np_element )->next;

    LFDS720_PAL_ATOMIC_DWCAS( ss->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( ss->pop_backoff, backoff_iteration );
  }
  while( result == 0 );

  // *se = (void *) ss + original_top[LFDS720_MISC_OFFSET];
  *se = LFDS720_MISC_OFFSET_TO_POINTER( ss, original_top[LFDS720_MISC_OFFSET], struct lfds720_stack_np_element );

  LFDS720_BACKOFF_AUTOTUNE( ss->pop_backoff, backoff_iteration );

  return 1;
}

