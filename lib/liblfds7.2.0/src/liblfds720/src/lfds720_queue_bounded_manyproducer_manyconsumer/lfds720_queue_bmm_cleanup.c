/***** includes *****/
#include "lfds720_queue_bmm_internal.h"





/****************************************************************************/
void lfds720_queue_bmm_cleanup( struct lfds720_queue_bmm_state *qbmms,
                                void (*element_cleanup_callback)(struct lfds720_queue_bmm_state *qbmms, void *key, void *value) )
{
  void
    *key,
    *value;

  LFDS720_PAL_ASSERT( qbmms != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
    while( lfds720_queue_bmm_dequeue(qbmms,&key,&value) )
      element_cleanup_callback( qbmms, key, value );

  return;
}

