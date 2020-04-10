/***** includes *****/
#include "lfds720_freelist_n_internal.h"





/****************************************************************************/
void lfds720_freelist_n_add_optional_elimination_array_to_improve_performance( struct lfds720_freelist_n_state *fs, 
                                                                               void *elimination_array,
                                                                               size_t elimination_array_size_in_bytes )
{
  lfds720_pal_uint_t
    loop,
    subloop;

  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( elimination_array != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) elimination_array % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( elimination_array_size_in_bytes >= LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES );

  fs->elimination_array = elimination_array;
  fs->elimination_array_number_of_lines = elimination_array_size_in_bytes / LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES;

  for( loop = 0 ; loop < fs->elimination_array_number_of_lines ; loop++ )
    for( subloop = 0 ; subloop < LFDS720_FREELIST_N_ELIMINATION_ARRAY_USED_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS ; subloop++ )
      fs->elimination_array[loop][subloop] = NULL;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

