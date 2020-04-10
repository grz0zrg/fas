/***** includes *****/
#include "lfds720_freelist_internal.h"


#include <stdio.h>
#include <stdlib.h>


/****************************************************************************/
void lfds720_freelist_push_clean_element( struct lfds720_freelist_state *fs,
                                          struct lfds720_freelist_element *fe,
                                          struct lfds720_smrhp_per_thread_state *smrhpts )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_freelist_element LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    *new_top, // TRD : function arguments are not suitably aligned on all platforms, so we need a local
    *original_top;

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( fe != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  new_top = fe;

  original_top = fs->top;

  do
  {
    fe->next = original_top;

    if( fe == original_top )
    {
      puts( "boing" );
      exit( EXIT_FAILURE );
    }

    // TRD : the next pointer in the element must be published to other threads *before* the element itself becomes visible
    LFDS720_MISC_BARRIER_STORE;

    // TRD : original top is set to the original value of fs->top, whether or not the function succeeds
    LFDS720_PAL_ATOMIC_CAS( fs->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( fs->push_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_BACKOFF_AUTOTUNE( fs->push_backoff, backoff_iteration );

  return;
}

