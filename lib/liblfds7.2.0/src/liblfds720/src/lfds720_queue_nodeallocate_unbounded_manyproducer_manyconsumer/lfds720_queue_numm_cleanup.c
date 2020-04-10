/***** includes *****/
#include "lfds720_queue_numm_internal.h"





/****************************************************************************/
void lfds720_queue_numm_cleanup( struct lfds720_queue_numm_state *qumms,
                                void (*element_cleanup_callback)(struct lfds720_queue_numm_state *qumms, struct lfds720_queue_numm_element *qumme, enum lfds720_misc_flag dummy_element_flag) )
{
  struct lfds720_queue_numm_element
    *qumme;

  void
    *value;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : element_cleanup_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( element_cleanup_callback != NULL )
  {
    while( qumms->dequeue[LFDS720_MISC_POINTER] != qumms->enqueue[LFDS720_MISC_POINTER] )
    {
      // TRD : trailing dummy element, so the first real value is in the next element
      value = qumms->dequeue[LFDS720_MISC_POINTER]->next[LFDS720_MISC_POINTER]->value;

      // TRD : user is given back *an* element, but not the one his user data was in
      qumme = qumms->dequeue[LFDS720_MISC_POINTER];

      // TRD : remove the element from queue
      qumms->dequeue[LFDS720_MISC_POINTER] = qumms->dequeue[LFDS720_MISC_POINTER]->next[LFDS720_MISC_POINTER];

      // TRD : write value into the qumme we're going to give the user
      qumme->value = value;

      element_cleanup_callback( qumms, qumme, LFDS720_MISC_FLAG_LOWERED );
    }

    // TRD : and now the final element
    element_cleanup_callback( qumms, (struct lfds720_queue_numm_element *) qumms->dequeue[LFDS720_MISC_POINTER], LFDS720_MISC_FLAG_RAISED );
  }

  return;
}

