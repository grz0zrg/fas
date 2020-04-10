/***** includes *****/
#include "lfds720_stack_np_internal.h"





/****************************************************************************/
void lfds720_stack_np_cleanup( struct lfds720_stack_np_state *ss,
                                  void (*element_cleanup_callback)(struct lfds720_stack_np_state *ss, struct lfds720_stack_np_element *se) )
{
  ptrdiff_t
    se;

  struct lfds720_stack_np_element
    *se_temp;

  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    se = ss->top[LFDS720_MISC_OFFSET];

    while( se != 0 )
    {
      se_temp = LFDS720_MISC_OFFSET_TO_POINTER( ss, se, struct lfds720_stack_np_element )
;
      se = se_temp->next;

      element_cleanup_callback( ss, se_temp );
    }
  }

  return;
}

