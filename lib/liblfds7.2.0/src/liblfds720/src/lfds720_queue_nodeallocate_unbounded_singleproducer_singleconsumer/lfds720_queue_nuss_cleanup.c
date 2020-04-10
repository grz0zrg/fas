/***** includes *****/
#include "lfds720_queue_nuss_internal.h"





/****************************************************************************/
void lfds720_queue_nuss_cleanup( struct lfds720_queue_nuss_state *qusss,
                                void (*element_cleanup_callback)( struct lfds720_queue_nuss_state *qusss,
                                                                  struct lfds720_queue_nuss_element *qusse,
                                                                  enum lfds720_misc_flag dummy_element_flag ) )
{
  enum lfds720_misc_flag
    dummy_element_flag = LFDS720_MISC_FLAG_LOWERED;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : element_cleanup_callback can be NULL

  if( element_cleanup_callback != NULL )
  {
    while( qusss->dequeue_writer_writes->next != NULL )
    {
      if( qusss->dequeue_writer_writes == qusss->dequeue_reader_writes )
        dummy_element_flag = LFDS720_MISC_FLAG_RAISED;

      element_cleanup_callback( qusss, qusss->dequeue_writer_writes, dummy_element_flag );
      qusss->dequeue_writer_writes = qusss->dequeue_writer_writes->next;
    }

    // TRD : now for the dummy element
    element_cleanup_callback( qusss, qusss->dequeue_writer_writes, LFDS720_MISC_FLAG_LOWERED );
  }

  return;
}

