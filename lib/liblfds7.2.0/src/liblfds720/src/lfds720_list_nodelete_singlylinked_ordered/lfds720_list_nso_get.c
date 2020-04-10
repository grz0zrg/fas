/***** includes *****/
#include "lfds720_list_nso_internal.h"





/****************************************************************************/
int lfds720_list_nso_get_by_key( struct lfds720_list_nso_state *lasos,
                                 void *key,
                                 struct lfds720_list_nso_element **lasoe )
{
  int
    cr = !0,
    rv = 1;

  LFDS720_PAL_ASSERT( lasos != NULL );
  // TRD : key can be NULL
  LFDS720_PAL_ASSERT( lasoe != NULL );

  while( cr != 0 and LFDS720_LIST_NSO_GET_START_AND_THEN_NEXT(*lasos, *lasoe) )
    cr = lasos->key_compare_function( key, (*lasoe)->key );

  if( *lasoe == NULL )
    rv = 0;

  return rv;
}

