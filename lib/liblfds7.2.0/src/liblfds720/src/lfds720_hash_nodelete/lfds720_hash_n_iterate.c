/***** includes *****/
#include "lfds720_hash_n_internal.h"





/****************************************************************************/
void lfds720_hash_n_iterate_init( struct lfds720_hash_n_state *has,
                                  struct lfds720_hash_n_iterate *hai )
{
  LFDS720_PAL_ASSERT( has != NULL );
  LFDS720_PAL_ASSERT( hai != NULL );

  hai->baus = has->baus_array;
  hai->baus_end = has->baus_array + has->array_size;
  hai->baue = NULL;

  return;
}





/****************************************************************************/
int lfds720_hash_n_iterate( struct lfds720_hash_n_iterate *hai,
                            struct lfds720_hash_n_element **hae )
{
  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  int
    rv = 0;

  LFDS720_PAL_ASSERT( hai != NULL );
  LFDS720_PAL_ASSERT( hae != NULL );

  while( finished_flag == LFDS720_MISC_FLAG_LOWERED )
  {
    lfds720_btree_nu_get_by_absolute_position_and_then_by_relative_position( hai->baus, &hai->baue, LFDS720_BTREE_NU_ABSOLUTE_POSITION_SMALLEST_IN_TREE, LFDS720_BTREE_NU_RELATIVE_POSITION_NEXT_LARGER_ELEMENT_IN_ENTIRE_TREE );

    if( hai->baue != NULL )
    {
      *hae = LFDS720_BTREE_NU_GET_VALUE_FROM_ELEMENT( *hai->baue );
      finished_flag = LFDS720_MISC_FLAG_RAISED;
      rv = 1;
    }

    if( hai->baue == NULL )
      if( ++hai->baus == hai->baus_end )
      {
        *hae = NULL;
        finished_flag = LFDS720_MISC_FLAG_RAISED;
      }
  }

  return rv;
}

