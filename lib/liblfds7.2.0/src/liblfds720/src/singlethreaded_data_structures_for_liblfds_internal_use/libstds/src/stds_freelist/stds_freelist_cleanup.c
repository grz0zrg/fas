#include "stds_freelist_internal.h"





/****************************************************************************/
void stds_freelist_cleanup( struct stds_freelist_state *fs,
                            void (*element_pop_callback)(struct stds_freelist_state *fs, struct stds_freelist_element *fe) )
{
  struct stds_freelist_element
    *fe = NULL;

  STDS_PAL_ASSERT( fs != NULL );
  // TRD : element_pop_callback can be any value in its range

  if( element_pop_callback != NULL )
  {
    STDS_FREELIST_POP( *fs, fe );

    while( fe != NULL )
    {
      element_pop_callback( fs, fe );
      STDS_FREELIST_POP( *fs, fe );
    }
  }

  return;
}

