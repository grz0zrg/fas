/***** includes *****/
#include "lfds720_queue_npuss_internal.h"





/****************************************************************************/
int lfds720_queue_npuss_dequeue( struct lfds720_queue_npuss_state *qusss,
                                 void **key,
                                 void **value )
{
  struct lfds720_queue_npuss_element
    *qusse,
    *qusse_next;

  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : key can be NULL
  // TRD : value can be NULL

  qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_reader_writes, struct lfds720_queue_npuss_element );

  if( qusse != NULL )
  {
    LFDS720_MISC_BARRIER_LOAD;

    qusse_next = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusse->next, struct lfds720_queue_npuss_element );

    if( key != NULL )
      *key = qusse_next->key;

    if( value != NULL )
      *value = qusse_next->value;

    qusss->dequeue_reader_writes = qusse->next;

    return 1;
  }

  return 0;
}

