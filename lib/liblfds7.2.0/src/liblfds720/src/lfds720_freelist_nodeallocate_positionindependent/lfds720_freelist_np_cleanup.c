/***** includes *****/
#include "lfds720_freelist_np_internal.h"





/****************************************************************************/
void lfds720_freelist_np_cleanup( struct lfds720_freelist_np_state *fs,
                                  void (*element_cleanup_callback)(struct lfds720_freelist_np_state *fs, struct lfds720_freelist_np_element *fe) )
{
  ptrdiff_t
    fe;

  struct lfds720_freelist_np_element
    *fe_temp;

  LFDS720_PAL_ASSERT( fs != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    fe = fs->top[LFDS720_MISC_OFFSET];

    while( fe != 0 )
    {
      fe_temp = LFDS720_MISC_OFFSET_TO_POINTER( fs, fe, struct lfds720_freelist_np_element )
;
      fe = fe_temp->next;

      element_cleanup_callback( fs, fe_temp );
    }
  }

  return;
}

