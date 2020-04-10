/***** includes *****/
#include "lfds720_list_nso_internal.h"





/****************************************************************************/
void lfds720_list_nso_cleanup( struct lfds720_list_nso_state *lasos,
                               void (*element_cleanup_callback)(struct lfds720_list_nso_state *lasos, struct lfds720_list_nso_element *lasoe) )
{
  struct lfds720_list_nso_element
    *lasoe,
    *temp;

  LFDS720_PAL_ASSERT( lasos != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback == NULL )
    return;

  lasoe = LFDS720_LIST_NSO_GET_START( *lasos );

  while( lasoe != NULL )
  {
    temp = lasoe;

    lasoe = LFDS720_LIST_NSO_GET_NEXT( *lasoe );

    element_cleanup_callback( lasos, temp );
  }

  return;
}

