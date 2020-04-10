/***** includes *****/
#include "lfds720_queue_nuss_internal.h"





/****************************************************************************/
void lfds720_queue_nuss_init_valid_on_current_logical_core( struct lfds720_queue_nuss_state *qusss,
                                                            struct lfds720_queue_nuss_element *qusse_dummy_element,
                                                            void (*dequeued_element_callback)(struct lfds720_queue_nuss_state *qusss, struct lfds720_queue_nuss_element *qusse),
                                                            void *user_state )
{
  LFDS720_PAL_ASSERT( qusss != NULL );
  LFDS720_PAL_ASSERT( qusse_dummy_element != NULL );
  LFDS720_PAL_ASSERT( dequeued_element_callback != NULL );
  // TRD : user_state can be NULL

  qusse_dummy_element->next = NULL;

  qusss->enqueue_writer_writes = qusse_dummy_element;
  qusss->dequeue_writer_writes = qusse_dummy_element;
  qusss->dequeue_reader_writes = qusse_dummy_element;

  qusss->dequeued_element_callback = dequeued_element_callback;
  qusss->user_state = user_state;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

