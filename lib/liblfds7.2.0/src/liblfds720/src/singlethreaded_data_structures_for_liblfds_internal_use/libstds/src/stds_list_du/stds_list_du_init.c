#include "stds_list_du_internal.h"





/****************************************************************************/
void stds_list_du_init( struct stds_list_du_state *ldus, void *user_state )
{
  STDS_PAL_ASSERT( ldus != NULL );
  // TRD : user_state can be NULL

  ldus->number_elements = 0;
  ldus->start = NULL;
  ldus->end = NULL;
  ldus->user_state = user_state;

  return;
}

