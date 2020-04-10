#include "stds_freelist_internal.h"





/****************************************************************************/
void stds_freelist_init( struct stds_freelist_state *fs, void *user_state )
{
  STDS_PAL_ASSERT( fs != NULL );
  // TRD : user_state can be NULL

  fs->top = NULL;

  fs->user_state = user_state;

  return;
}

