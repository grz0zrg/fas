/***** includes *****/
#include "lfds720_queue_npuss_internal.h"





/****************************************************************************/
void lfds720_queue_npuss_flush_dequeued_elements( struct lfds720_queue_npuss_state *qusss )
{
  struct lfds720_queue_npuss_element
    *qusse;

  LFDS720_PAL_ASSERT( qusss != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );

  while( qusse->next != 0 and qusss->dequeue_writer_writes != qusss->dequeue_reader_writes )
  {
    qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );
    qusss->dequeue_writer_writes = qusse->next;
    qusss->dequeued_element_callback( qusss, qusse );
  }

  return;
}

