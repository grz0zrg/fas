/***** includes *****/
#include "lfds720_queue_nuss_internal.h"





/****************************************************************************/
int lfds720_queue_nuss_dequeue( struct lfds720_queue_nuss_state *qusss,
                               void **key,
                               void **value )
{
  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL

  if( qusss->dequeue_reader_writes->next != NULL )
  {
    LFDS720_MISC_BARRIER_LOAD;

    if( key != NULL )
      *key = qusss->dequeue_reader_writes->next->key;

    if( value != NULL )
      *value = qusss->dequeue_reader_writes->next->value;

    qusss->dequeue_reader_writes = qusss->dequeue_reader_writes->next;

    return 1;
  }

  return 0;
}

