/***** includes *****/
#include "lfds720_queue_umm_internal.h"





/****************************************************************************/
void lfds720_queue_umm_enqueue( struct lfds720_queue_umm_state *qummhps,
                                      struct lfds720_queue_umm_element *qummhpe,
                                      struct lfds720_smrhp_per_thread_state *smrhpts )
{
  char unsigned
    result = 0;

  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_queue_umm_element LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    *volatile enqueue,
    *new_enqueue,
    *volatile next;

  LFDS720_PAL_ASSERT( qummhps != NULL );
  LFDS720_PAL_ASSERT( qummhpe != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) qummhpe->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  qummhpe->next = NULL;
  LFDS720_MISC_BARRIER_STORE;

  new_enqueue = qummhpe;

  LFDS720_MISC_BARRIER_LOAD;

  do
  {
    /* TRD : note here the deviation from the white paper
             in the white paper, next is loaded from enqueue, not from qummhps->enqueue
             what concerns me is that between the load of enqueue and the load of
             enqueue->next, the element can be dequeued by another thread *and freed*

             by ordering the loads (load barriers), and loading both from qummhps,
             the following if(), which checks enqueue is still the same as qummhps->enqueue
             still continues to ensure next belongs to enqueue, while avoiding the
             problem with free
    */

    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_POINTER, qummhps->enqueue );

    enqueue = LFDS720_SMRHP_GET_HAZARD_POINTER( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_POINTER );

    LFDS720_MISC_BARRIER_LOAD;

    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_NEXT_POINTER, enqueue->next );

    next = LFDS720_SMRHP_GET_HAZARD_POINTER( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_NEXT_POINTER );

    LFDS720_MISC_BARRIER_LOAD;

    if( qummhps->enqueue == enqueue )
    {
      if( next == NULL )
      {
        LFDS720_PAL_ATOMIC_CAS( enqueue->next, next, new_enqueue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
        if( result == 1 )
          finished_flag = LFDS720_MISC_FLAG_RAISED;
      }
      else
      {
        // TRD : strictly, this is a weak CAS, but we do an extra iteration of the main loop on a fake failure, so we set it to be strong
        LFDS720_PAL_ATOMIC_CAS( qummhps->enqueue, enqueue, next, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
      }
    }
    else
      result = 0;

    LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_NEXT_POINTER );

    LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( *smrhpts, LFDS720_QUEUE_UMM_HAZARD_POINTER_INDEX_ENQUEUE_POINTER );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qummhps->enqueue_backoff, backoff_iteration );
  }
  while( finished_flag == LFDS720_MISC_FLAG_LOWERED );

  // TRD : move enqueue along; only a weak CAS as the dequeue will solve this if it's out of place
  LFDS720_PAL_ATOMIC_CAS( qummhps->enqueue, enqueue, new_enqueue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

  if( result == 0 )
    LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( qummhps->enqueue_backoff, backoff_iteration );

  LFDS720_BACKOFF_AUTOTUNE( qummhps->enqueue_backoff, backoff_iteration );

  return;
}

