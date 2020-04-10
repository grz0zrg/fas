/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"





/****************************************************************************/
void lfds720_list_so_cleanup( struct lfds720_list_so_state *lsos,
                              void (*element_cleanup_callback)(struct lfds720_list_so_state *lsos, struct lfds720_list_so_element *lsoe) )
{
  struct lfds720_list_so_element
    *lsoe,
    *temp;

  LFDS720_PAL_ASSERT( lsos != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback == NULL )
  {
    lsoe = LFDS720_LIST_SO_GET_START( *lsos );

    while( lsoe != NULL )
    {
      temp = lsoe;
      lsoe = LFDS720_LIST_SO_GET_NEXT( *lsoe );
      element_cleanup_callback( lsos, temp );
    }
  }

  return;
}

