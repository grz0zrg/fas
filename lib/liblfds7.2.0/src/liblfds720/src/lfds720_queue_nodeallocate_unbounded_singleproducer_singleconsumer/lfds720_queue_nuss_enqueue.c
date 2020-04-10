/***** includes *****/
#include "lfds720_queue_nuss_internal.h"





/****************************************************************************/
void lfds720_queue_nuss_enqueue( struct lfds720_queue_nuss_state *qusss,
                                struct lfds720_queue_nuss_element *qusse )
{
  struct lfds720_queue_nuss_element
    *qusse_temp;

  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse != NULL );
  // TRD : position can be any value in its range

  // TRD : first, remove read elements (duplicate of lfds720_queue_nuss_flush_dequeued_elements())
  while( qusss->dequeue_writer_writes->next != NULL and qusss->dequeue_writer_writes != qusss->dequeue_reader_writes )
  {
    qusse_temp = qusss->dequeue_writer_writes;
    qusss->dequeue_writer_writes = qusss->dequeue_writer_writes->next;
    qusss->dequeued_element_callback( qusss, qusse_temp );
  }

  // TRD : second, enqueue the element
  qusse->next = NULL;
  LFDS720_MISC_BARRIER_STORE;
  qusss->enqueue_writer_writes->next = qusse;
  qusss->enqueue_writer_writes = qusse;

  return;
}





/****************************************************************************/
void lfds720_queue_nuss_enqueue_inside_dequeue_element_callback( struct lfds720_queue_nuss_state *qusss,
                                                                struct lfds720_queue_nuss_element *qusse )
{
  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse != NULL );
  // TRD : position can be any value in its range

  // TRD : enqueue the element
  qusse->next = NULL;
  LFDS720_MISC_BARRIER_STORE;
  qusss->enqueue_writer_writes->next = qusse;
  qusss->enqueue_writer_writes = qusse;

  return;
}

