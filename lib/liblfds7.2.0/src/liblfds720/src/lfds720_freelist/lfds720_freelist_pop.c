/***** includes *****/
#include "lfds720_freelist_internal.h"


#include <stdio.h>
#include <stdlib.h>



/****************************************************************************/
int lfds720_freelist_pop_dirty_element( struct lfds720_freelist_state *fs,
                                        struct lfds720_freelist_element **fe,
                                        struct lfds720_smrhp_per_thread_state *smrhpts )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_freelist_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *new_top,
    *original_top;

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( fe != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  do
  {
    original_top = LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_FREELIST_HAZARD_POINTER_INDEX_HEAD_POINTER, fs->top );

    if( original_top == NULL )
    {
      *fe = NULL;
      LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_FREELIST_HAZARD_POINTER_INDEX_NEXT_POINTER );
      return 0;
    }

    new_top = LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_FREELIST_HAZARD_POINTER_INDEX_NEXT_POINTER, original_top->next );

    if( original_top == new_top )
    {
      puts( "kerunch" );
      exit( EXIT_FAILURE );
    }

    LFDS720_MISC_BARRIER_LOAD;

    LFDS720_PAL_ATOMIC_CAS( fs->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( fs->pop_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_FREELIST_HAZARD_POINTER_INDEX_HEAD_POINTER );
  LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_FREELIST_HAZARD_POINTER_INDEX_NEXT_POINTER );

  *fe = original_top;

    if( original_top == original_top->next )
    {
      puts( "pop boing" );
      exit( EXIT_FAILURE );
    }

  LFDS720_BACKOFF_AUTOTUNE( fs->pop_backoff, backoff_iteration );

  return 1;
}

