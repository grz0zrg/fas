/***** includes *****/
#include "lfds720_list_nsu_internal.h"





/****************************************************************************/
int lfds720_list_nsu_get_by_key( struct lfds720_list_nsu_state *lasus,
                                 int (*key_compare_function)(void const *new_key, void const *existing_key),
                                 void *key, 
                                 struct lfds720_list_nsu_element **lasue )
{
  int
    cr = !0,
    rv = 1;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( key_compare_function != NULL );
  // TRD : key can be NULL
  LFDS720_PAL_ASSERT( lasue != NULL );

  *lasue = NULL;

  while( cr != 0 and LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(*lasus, *lasue) )
    cr = key_compare_function( key, (*lasue)->key );

  if( *lasue == NULL )
    rv = 0;

  return rv;
}

