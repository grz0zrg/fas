/***** includes *****/
#include "lfds720_queue_npuss_internal.h"





/****************************************************************************/
void lfds720_queue_npuss_cleanup( struct lfds720_queue_npuss_state *qusss,
                                  void (*element_cleanup_callback)( struct lfds720_queue_npuss_state *qusss,
                                                                    struct lfds720_queue_npuss_element *qusse,
                                                                    enum lfds720_misc_flag dummy_element_flag ) )
{
  enum lfds720_misc_flag
    dummy_element_flag = LFDS720_MISC_FLAG_LOWERED;

  struct lfds720_queue_npuss_element
    *qusse;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : element_cleanup_callback can be NULL

  if( element_cleanup_callback != NULL )
  {
    qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );

    while( qusse->next != 0 )
    {
      if( qusss->dequeue_writer_writes == qusss->dequeue_reader_writes )
        dummy_element_flag = LFDS720_MISC_FLAG_RAISED;

      element_cleanup_callback( qusss, qusse, dummy_element_flag );
      qusss->dequeue_writer_writes = qusse->next;

      qusse = LFDS720_MISC_OFFSET_TO_POINTER( qusss, qusss->dequeue_writer_writes, struct lfds720_queue_npuss_element );
    }

    // TRD : now for the dummy element
    element_cleanup_callback( qusss, qusse, LFDS720_MISC_FLAG_LOWERED );
  }

  return;
}

