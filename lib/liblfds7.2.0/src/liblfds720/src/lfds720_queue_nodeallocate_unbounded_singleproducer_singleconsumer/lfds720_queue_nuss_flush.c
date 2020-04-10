/***** includes *****/
#include "lfds720_queue_nuss_internal.h"





/****************************************************************************/
void lfds720_queue_nuss_flush_dequeued_elements( struct lfds720_queue_nuss_state *qusss )
{
  struct lfds720_queue_nuss_element
    *qusse;

  LFDS720_PAL_ASSERT( qusss != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  while( qusss->dequeue_writer_writes->next != NULL and qusss->dequeue_writer_writes != qusss->dequeue_reader_writes )
  {
    qusse = qusss->dequeue_writer_writes;
    qusss->dequeue_writer_writes = qusss->dequeue_writer_writes->next;
    qusss->dequeued_element_callback( qusss, qusse );
  }

  return;
}

