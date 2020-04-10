/***** includes *****/
#include "lfds720_queue_npumm_internal.h"





/****************************************************************************/
void lfds720_queue_npumm_cleanup( struct lfds720_queue_npumm_state *qumms,
                                  void (*element_cleanup_callback)(struct lfds720_queue_npumm_state *qumms, struct lfds720_queue_npumm_element *qumme, enum lfds720_misc_flag dummy_element_flag) )
{
  struct lfds720_queue_npumm_element
    *qumme,
    *qumme_next;

  void
    *value;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    while( qumms->dequeue[LFDS720_MISC_OFFSET] != qumms->enqueue[LFDS720_MISC_OFFSET] )
    {
      // TRD : user is given back *an* element, but not the one his user data was in
      qumme = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumms->dequeue[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );

      // TRD : trailing dummy element, so the first real value is in the next element
      qumme_next = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumme->next[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );
      value = qumme_next->value;

      // TRD : remove the element from queue
      qumms->dequeue[LFDS720_MISC_OFFSET] = qumme->next[LFDS720_MISC_OFFSET];

      // TRD : write value into the qumme we're going to give the user
      qumme->value = value;

      element_cleanup_callback( qumms, qumme, LFDS720_MISC_FLAG_LOWERED );
    }

    // TRD : and now the final element
    element_cleanup_callback( qumms, qumme, LFDS720_MISC_FLAG_RAISED );
  }

  return;
}

