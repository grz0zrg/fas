/***** includes *****/
#include "lfds720_queue_npuss_internal.h"





/****************************************************************************/
void lfds720_queue_npuss_enqueue( struct lfds720_queue_npuss_state *qusss,
                                  struct lfds720_queue_npuss_element *qusse )
{
  ptrdiff_t
    qusse_offset;

  struct lfds720_queue_npuss_element
    *qusse_temp;

  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse != NULL );
  // TRD : position can be any value in its range

  // TRD : first, remove read elements (duplicate of lfds720_queue_npuss_flush_dequeued_elements())
  qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );

  while( qusse->next != 0 and qusss->dequeue_writer_writes != qusss->dequeue_reader_writes )
  {
    qusse_temp = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );
    qusss->dequeue_writer_writes = qusse_temp->next;
    qusss->dequeued_element_callback( qusss, qusse_temp );
  }

  // TRD : second, enqueue the element
  qusse->next = 0;
  LFDS720_MISC_BARRIER_STORE;
  qusse_offset = LFDS720_MISC_POINTER_TO_OFFSET( qusss, qusse );
  LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->enqueue_writer_writes, struct lfds720_queue_npuss_element )->next = qusse_offset;
  qusss->enqueue_writer_writes = qusse_offset;

  return;
}





/****************************************************************************/
void lfds720_queue_npuss_enqueue_inside_dequeue_element_callback( struct lfds720_queue_npuss_state *qusss,
                                                                 struct lfds720_queue_npuss_element *qusse )
{
  ptrdiff_t
    qusse_offset;

  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse != NULL );
  // TRD : position can be any value in its range

  // TRD : enqueue the element
  qusse->next = 0;
  LFDS720_MISC_BARRIER_STORE;
  qusse_offset = LFDS720_MISC_POINTER_TO_OFFSET( qusss, qusse );
  LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->enqueue_writer_writes, struct lfds720_queue_npuss_element )->next = qusse_offset;
  qusss->enqueue_writer_writes = qusse_offset;

  return;
}

