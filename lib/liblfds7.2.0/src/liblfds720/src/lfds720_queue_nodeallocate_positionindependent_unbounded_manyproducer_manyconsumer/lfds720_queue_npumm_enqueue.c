/***** includes *****/
#include "lfds720_queue_npumm_internal.h"





/****************************************************************************/
void lfds720_queue_npumm_enqueue( struct lfds720_queue_npumm_state *qumms,
                                  struct lfds720_queue_npumm_element *qumme )
{
  char unsigned
    result = 0;

  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  ptrdiff_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    enqueue[LFDS720_MISC_PAC_SIZE],
    next[LFDS720_MISC_PAC_SIZE];

  ptrdiff_t LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    new_enqueue[LFDS720_MISC_PAC_SIZE];

  struct lfds720_queue_npumm_element
    *enqueue_pointer;

  LFDS720_PAL_ASSERT( qumms != NULL );
  LFDS720_PAL_ASSERT( qumme != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) qumme->next % LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES == 0 );

  qumme->next[LFDS720_MISC_OFFSET] = 0;
  LFDS720_PAL_ATOMIC_ADD( qumms->aba_counter, 1, qumme->next[LFDS720_MISC_COUNTER], ptrdiff_t );
  LFDS720_MISC_BARRIER_STORE;

  new_enqueue[LFDS720_MISC_OFFSET] = LFDS720_MISC_POINTER_TO_OFFSET( qumms, qumme );

  LFDS720_MISC_BARRIER_LOAD;

  do
  {
    /* TRD : note here the deviation from the white paper
             in the white paper, next is loaded from enqueue, not from qumms->enqueue
             what concerns me is that between the load of enqueue and the load of
             enqueue->next, the element can be dequeued by another thread *and freed*

             by ordering the loads (load barriers), and loading both from qumms,
             the following if(), which checks enqueue is still the same as qumms->enqueue
             still continues to ensure next belongs to enqueue, while avoiding the
             problem with free
    */

    enqueue[LFDS720_MISC_COUNTER] = qumms->enqueue[LFDS720_MISC_COUNTER];
    enqueue[LFDS720_MISC_OFFSET]  = qumms->enqueue[LFDS720_MISC_OFFSET];

    enqueue_pointer = LFDS720_MISC_OFFSET_TO_POINTER( qumms, enqueue[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );

    LFDS720_MISC_BARRIER_LOAD;

    next[LFDS720_MISC_COUNTER] = enqueue_pointer->next[LFDS720_MISC_COUNTER];
    next[LFDS720_MISC_OFFSET]  = enqueue_pointer->next[LFDS720_MISC_OFFSET];

    LFDS720_MISC_BARRIER_LOAD;

    if( qumms->enqueue[LFDS720_MISC_COUNTER] == enqueue[LFDS720_MISC_COUNTER] and qumms->enqueue[LFDS720_MISC_OFFSET] == enqueue[LFDS720_MISC_OFFSET] )
    {
      if( next[LFDS720_MISC_OFFSET] == 0 )
      {
        new_enqueue[LFDS720_MISC_COUNTER] = next[LFDS720_MISC_COUNTER] + 1;
        LFDS720_PAL_ATOMIC_DWCAS( enqueue_pointer->next, next, new_enqueue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
        if( result == 1 )
          finished_flag = LFDS720_MISC_FLAG_RAISED;
      }
      else
      {
        next[LFDS720_MISC_COUNTER] = enqueue[LFDS720_MISC_COUNTER] + 1;
        // TRD : strictly, this is a weak CAS, but we do an extra iteration of the main loop on a fake failure, so we set it to be strong
        LFDS720_PAL_ATOMIC_DWCAS( qumms->enqueue, enqueue, next, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
      }
    }
    else
      result = 0;

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qumms->enqueue_backoff, backoff_iteration );
  }
  while( finished_flag == LFDS720_MISC_FLAG_LOWERED );

  // TRD : move enqueue along; only a weak CAS as the dequeue will solve this if it's out of place
  new_enqueue[LFDS720_MISC_COUNTER] = enqueue[LFDS720_MISC_COUNTER] + 1;
  LFDS720_PAL_ATOMIC_DWCAS( qumms->enqueue, enqueue, new_enqueue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

  if( result == 0 )
    LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qumms->enqueue_backoff, backoff_iteration );

  LFDS720_BACKOFF_AUTOTUNE( qumms->enqueue_backoff, backoff_iteration );

  return;
}

