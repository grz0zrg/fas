/***** includes *****/
#include "lfds720_freelist_n_internal.h"





/****************************************************************************/
void lfds720_freelist_n_cleanup( struct lfds720_freelist_n_state *fs,
                               void (*element_cleanup_callback)(struct lfds720_freelist_n_state *fs, struct lfds720_freelist_n_element *fe) )
{
  struct lfds720_freelist_n_element
    *fe,
    *fe_temp;

  LFDS720_PAL_ASSERT( fs != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    fe = fs->top[LFDS720_MISC_POINTER];

    while( fe != NULL )
    {
      fe_temp = fe;
      fe = fe->next;

      element_cleanup_callback( fs, fe_temp );
    }
  }

  return;
}

