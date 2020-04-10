/***** includes *****/
#include "lfds720_stack_n_internal.h"





/****************************************************************************/
void lfds720_stack_n_cleanup( struct lfds720_stack_n_state *ss,
                            void (*element_cleanup_callback)(struct lfds720_stack_n_state *ss, struct lfds720_stack_n_element *se) )
{
  struct lfds720_stack_n_element
    *se,
    *se_temp;

  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    se = ss->top[LFDS720_MISC_POINTER];

    while( se != NULL )
    {
      se_temp = se;
      se = se->next;

      element_cleanup_callback( ss, se_temp );
    }
  }

  return;
}

