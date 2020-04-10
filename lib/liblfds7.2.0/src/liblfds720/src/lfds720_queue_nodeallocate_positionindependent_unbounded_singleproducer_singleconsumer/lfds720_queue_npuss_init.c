/***** includes *****/
#include "lfds720_queue_npuss_internal.h"





/****************************************************************************/
void lfds720_queue_npuss_init_valid_on_current_logical_core( struct lfds720_queue_npuss_state *qusss,
                                                             struct lfds720_queue_npuss_element *qusse_dummy_element,
                                                             void (*dequeued_element_callback)(struct lfds720_queue_npuss_state *qusss, struct lfds720_queue_npuss_element *qusse),
                                                             void *user_state )
{
  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse_dummy_element != NULL );
  LFDS720_PAL_ASSERT( dequeued_element_callback != NULL );
  // TRD : user_state can be NULL

  qusse_dummy_element->next = 0;

  qusss->enqueue_writer_writes = LFDS720_MISC_POINTER_TO_OFFSET( qusss, qusse_dummy_element );
  qusss->dequeue_writer_writes = LFDS720_MISC_POINTER_TO_OFFSET( qusss, qusse_dummy_element );
  qusss->dequeue_reader_writes = LFDS720_MISC_POINTER_TO_OFFSET( qusss, qusse_dummy_element );

  qusss->dequeued_element_callback = dequeued_element_callback;
  qusss->user_state = user_state;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

