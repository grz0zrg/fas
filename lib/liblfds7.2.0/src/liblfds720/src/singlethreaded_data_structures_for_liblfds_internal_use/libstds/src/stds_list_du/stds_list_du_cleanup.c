#include "stds_list_du_internal.h"





/****************************************************************************/
void stds_list_du_cleanup( struct stds_list_du_state *ldus,
                           void (*element_cleanup_callback)(struct stds_list_du_state *ldus, struct stds_list_du_element *ldue) )
{
  struct stds_list_du_element
    *ldue;

  STDS_PAL_ASSERT( ldus != NULL );
  // TRD : element_cleanup_callback can be NULL

  if( element_cleanup_callback != NULL )
  {
    ldue = STDS_LIST_DU_GET_START( *ldus );

    while( ldue != NULL )
    {
      stds_list_du_remove_element( ldus, ldue );
      ldue = STDS_LIST_DU_GET_START( *ldus );
    }
  }

  return;
}

