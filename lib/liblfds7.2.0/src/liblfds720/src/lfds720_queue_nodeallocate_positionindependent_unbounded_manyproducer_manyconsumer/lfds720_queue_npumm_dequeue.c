/***** includes *****/
#include "lfds720_queue_npumm_internal.h"





/****************************************************************************/
int lfds720_queue_npumm_dequeue( struct lfds720_queue_npumm_state *qumms,
                                 struct lfds720_queue_npumm_element **qumme )
{
  char unsigned
    result = 0;

  enum lfds720_misc_flag
    backoff_flag = LFDS720_MISC_FLAG_RAISED,
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  enum lfds720_queue_npumm_queue_state
    state = LFDS720_QUEUE_NPUMM_QUEUE_STATE_UNKNOWN;

  int
    rv = 1;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  ptrdiff_t LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    dequeue[LFDS720_MISC_PAC_SIZE],
    enqueue[LFDS720_MISC_PAC_SIZE],
    next[LFDS720_MISC_PAC_SIZE];

  struct lfds720_queue_npumm_element
    *enqueue_pointer,
    *next_pointer;

  void
    *key = NULL,
    *value = NULL;

  LFDS720_PAL_ASSERT( qumms != NULL );
  LFDS720_PAL_ASSERT( qumme != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  do
  {
    /* TRD : note here the deviation from the white paper
             in the white paper, next is loaded from dequeue, not from qumms->dequeue
             what concerns me is that between the load of dequeue and the load of
             enqueue->next, the element can be dequeued by another thread *and freed*

             by ordering the loads (load barriers), and loading both from qumms,
             the following if(), which checks dequeue is still the same as qumms->enqueue
             still continues to ensure next belongs to enqueue, while avoiding the
             problem with free
    */

    dequeue[LFDS720_MISC_COUNTER] = qumms->dequeue[LFDS720_MISC_COUNTER];
    dequeue[LFDS720_MISC_OFFSET]  = qumms->dequeue[LFDS720_MISC_OFFSET];

    LFDS720_MISC_BARRIER_LOAD;

    enqueue[LFDS720_MISC_COUNTER] = qumms->enqueue[LFDS720_MISC_COUNTER];
    enqueue[LFDS720_MISC_OFFSET]  = qumms->enqueue[LFDS720_MISC_OFFSET];

    enqueue_pointer = LFDS720_MISC_OFFSET_TO_POINTER( qumms, enqueue[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );

    next[LFDS720_MISC_COUNTER] = enqueue_pointer->next[LFDS720_MISC_COUNTER];
    next[LFDS720_MISC_OFFSET]  = enqueue_pointer->next[LFDS720_MISC_OFFSET];

    LFDS720_MISC_BARRIER_LOAD;

    if( qumms->dequeue[LFDS720_MISC_COUNTER] == dequeue[LFDS720_MISC_COUNTER] and qumms->dequeue[LFDS720_MISC_OFFSET] == dequeue[LFDS720_MISC_OFFSET] )
    {
      if( enqueue[LFDS720_MISC_OFFSET] == dequeue[LFDS720_MISC_OFFSET] and next[LFDS720_MISC_OFFSET] == 0 )
        state = LFDS720_QUEUE_NPUMM_QUEUE_STATE_EMPTY;

      if( enqueue[LFDS720_MISC_OFFSET] == dequeue[LFDS720_MISC_OFFSET] and next[LFDS720_MISC_OFFSET] != 0 )
        state = LFDS720_QUEUE_NPUMM_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE;

      if( enqueue[LFDS720_MISC_OFFSET] != dequeue[LFDS720_MISC_OFFSET] )
        state = LFDS720_QUEUE_NPUMM_QUEUE_STATE_ATTEMPT_DEQUEUE;

      switch( state )
      {
        case LFDS720_QUEUE_NPUMM_QUEUE_STATE_UNKNOWN:
          // TRD : eliminates compiler warning
        break;

        case LFDS720_QUEUE_NPUMM_QUEUE_STATE_EMPTY:
          rv = 0;
          *qumme = NULL;
          result = 0;
          backoff_flag = LFDS720_MISC_FLAG_LOWERED;
          finished_flag = LFDS720_MISC_FLAG_RAISED;
        break;

        case LFDS720_QUEUE_NPUMM_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE:
          next[LFDS720_MISC_COUNTER] = enqueue[LFDS720_MISC_COUNTER] + 1;
          LFDS720_PAL_ATOMIC_DWCAS( qumms->enqueue, enqueue, next, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
          // TRD : in fact if result is 1 (successful) I think we can now simply drop down into the dequeue attempt
        break;

        case LFDS720_QUEUE_NPUMM_QUEUE_STATE_ATTEMPT_DEQUEUE:
          next_pointer = LFDS720_MISC_OFFSET_TO_POINTER( qumms, next[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );

          key   = next_pointer->key;
          value = next_pointer->value;

          next[LFDS720_MISC_COUNTER] = qumms->dequeue[LFDS720_MISC_COUNTER] + 1;
          LFDS720_PAL_ATOMIC_DWCAS( qumms->dequeue, dequeue, next, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

          if( result == 1 )
          {
            backoff_flag = LFDS720_MISC_FLAG_LOWERED;
            finished_flag = LFDS720_MISC_FLAG_RAISED;
          }
        break;
      }
    }
    else
      backoff_flag = LFDS720_MISC_FLAG_RAISED;

    if( backoff_flag == LFDS720_MISC_FLAG_RAISED )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qumms->dequeue_backoff, backoff_iteration );
  }
  while( finished_flag == LFDS720_MISC_FLAG_LOWERED );

  if( result == 1 )
  {
    *qumme = LFDS720_MISC_OFFSET_TO_POINTER( qumms, dequeue[LFDS720_MISC_OFFSET], struct lfds720_queue_npumm_element );
    (*qumme)->key = key;
    (*qumme)->value = value;
  }

  LFDS720_BACKOFF_AUTOTUNE( qumms->dequeue_backoff, backoff_iteration );

  return rv;
}

