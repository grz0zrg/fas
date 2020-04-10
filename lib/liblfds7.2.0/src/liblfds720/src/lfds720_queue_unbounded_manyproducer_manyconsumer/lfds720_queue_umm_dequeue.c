/***** includes *****/
#include "lfds720_queue_umm_internal.h"





/****************************************************************************/
int lfds720_queue_umm_dequeue( struct lfds720_queue_umm_state *qummhps,
                                     struct lfds720_queue_umm_element **qummhpe,
                                     struct lfds720_smrhp_per_thread_state *smrhpts )
{
  char unsigned
    result = 0;

  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  enum lfds720_queue_umm_queue_state
    state = LFDS720_QUEUE_UMM_QUEUE_STATE_UNKNOWN;

  int
    rv = 1;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_queue_umm_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *dequeue,
    *enqueue,
    *next;

  void
    *key = NULL,
    *value = NULL;

  LFDS720_PAL_ASSERT( qummhps != NULL );
  LFDS720_PAL_ASSERT( qummhpe != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  do
  {
    /* TRD : note here the deviation from the white paper
             in the white paper, next is loaded from dequeue, not from qummhps->dequeue
             what concerns me is that between the load of dequeue and the load of
             enqueue->next, the element can be dequeued by another thread *and freed*

             by ordering the loads (load barriers), and loading both from qummhps,
             the following if(), which checks dequeue is still the same as qummhps->enqueue
             still continues to ensure next belongs to enqueue, while avoiding the
             problem with free
    */

    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_POINTER, qummhps->dequeue );

    dequeue = LFDS720_SMRHP_GET_HAZARD_POINTER( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_POINTER );

    LFDS720_MISC_BARRIER_LOAD;

    enqueue = qummhps->enqueue;

    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_NEXT_POINTER, dequeue->next );

    next = LFDS720_SMRHP_GET_HAZARD_POINTER( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_NEXT_POINTER );

    LFDS720_MISC_BARRIER_LOAD;

    if( qummhps->dequeue == dequeue )
    {
      if( enqueue == dequeue and next == NULL )
        state = LFDS720_QUEUE_UMM_QUEUE_STATE_EMPTY;

      if( enqueue == dequeue and next != NULL )
        state = LFDS720_QUEUE_UMM_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE;

      if( enqueue != dequeue )
        state = LFDS720_QUEUE_UMM_QUEUE_STATE_ATTEMPT_DEQUEUE;

      switch( state )
      {
        case LFDS720_QUEUE_UMM_QUEUE_STATE_UNKNOWN:
          // TRD : eliminates compiler warning
        break;

        case LFDS720_QUEUE_UMM_QUEUE_STATE_EMPTY:
          rv = 0;
          *qummhpe = NULL;
          result = 1;
          finished_flag = LFDS720_MISC_FLAG_RAISED;
        break;

        case LFDS720_QUEUE_UMM_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE:
          LFDS720_PAL_ATOMIC_CAS( qummhps->enqueue, enqueue, next, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
          // TRD : in fact if result is 1 (successful) I think we can now simply drop down into the dequeue attempt
        break;

        case LFDS720_QUEUE_UMM_QUEUE_STATE_ATTEMPT_DEQUEUE:
          key = next->key;
          value = next->value;

          LFDS720_PAL_ATOMIC_CAS( qummhps->dequeue, dequeue, next, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

          if( result == 1 )
            finished_flag = LFDS720_MISC_FLAG_RAISED;
        break;
      }
    }
    else
      result = 0;

    LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_NEXT_POINTER );

    LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_DEQUEUE_POINTER );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qummhps->dequeue_backoff, backoff_iteration );
  }
  while( finished_flag == LFDS720_MISC_FLAG_LOWERED );

  if( result == 1 )
  {
    *qummhpe = dequeue;
    (*qummhpe)->key = key;
    (*qummhpe)->value = value;
  }

  LFDS720_BACKOFF_AUTOTUNE( qummhps->dequeue_backoff, backoff_iteration );

  return rv;
}

