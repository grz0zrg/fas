/***** includes *****/
#include "lfds720_freelist_n_internal.h"





/****************************************************************************/
int lfds720_freelist_n_threadsafe_pop( struct lfds720_freelist_n_state *fs,
                                       struct lfds720_freelist_n_per_thread_state *fpts,
                                       struct lfds720_freelist_n_element **fe )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE,
    elimination_array_index,
    loop;

  struct lfds720_freelist_n_element LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    *new_top[LFDS720_MISC_PAC_SIZE],
    *volatile original_top[LFDS720_MISC_PAC_SIZE];

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( (fs->elimination_array != NULL and fpts != NULL) or
                      (fs->elimination_array == NULL and fpts == NULL) );
  LFDS720_PAL_ASSERT( fe != NULL );

  if( fs->elimination_array != NULL )
  {
    elimination_array_index = ( (fpts->elimination_array_round_robin_pop_counter++) & (fs->elimination_array_number_of_lines-1) );

    *fe = NULL;

    // TRD : full scan of one cache line, max pointers per cache line
    for( loop = 0 ; loop < LFDS720_FREELIST_N_ELIMINATION_ARRAY_USED_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS ; loop++ )
      if( fs->elimination_array[elimination_array_index][loop] != NULL )
      {
        LFDS720_PAL_ATOMIC_EXCHANGE( fs->elimination_array[elimination_array_index][loop], *fe, struct lfds720_freelist_n_element * );
        if( *fe != NULL )
        {
          fpts->elimination_array_round_robin_push_counter = fpts->elimination_array_round_robin_pop_counter;
          return 1;
        }
      }
  }

  original_top[LFDS720_MISC_COUNTER] = fs->top[LFDS720_MISC_COUNTER];
  original_top[LFDS720_MISC_POINTER] = fs->top[LFDS720_MISC_POINTER];

  do
  {
    if( original_top[LFDS720_MISC_POINTER] == NULL )
    {
      *fe = NULL;
      return 0;
    }

    new_top[LFDS720_MISC_COUNTER] = original_top[LFDS720_MISC_COUNTER] + 1;
    new_top[LFDS720_MISC_POINTER] = original_top[LFDS720_MISC_POINTER]->next;

    LFDS720_PAL_ATOMIC_DWCAS( fs->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( fs->pop_backoff, backoff_iteration );
  }
  while( result == 0 );

  *fe = original_top[LFDS720_MISC_POINTER];

  LFDS720_BACKOFF_AUTOTUNE( fs->pop_backoff, backoff_iteration );

  return 1;
}

