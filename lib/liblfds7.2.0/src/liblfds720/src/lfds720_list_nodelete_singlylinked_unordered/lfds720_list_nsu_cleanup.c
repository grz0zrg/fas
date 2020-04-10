/***** includes *****/
#include "lfds720_list_nsu_internal.h"





/****************************************************************************/
void lfds720_list_nsu_cleanup( struct lfds720_list_nsu_state *lasus,
                               void (*element_cleanup_callback)(struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue) )
{
  struct lfds720_list_nsu_element
    *lasue,
    *temp;

  LFDS720_PAL_ASSERT( lasus != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback == NULL )
    return;

  lasue = LFDS720_LIST_NSU_GET_START( *lasus );

  while( lasue != NULL )
  {
    temp = lasue;

    lasue = LFDS720_LIST_NSU_GET_NEXT( *lasue );

    element_cleanup_callback( lasus, temp );
  }

  return;
}

