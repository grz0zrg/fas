/***** includes *****/
#include "lfds720_queue_umm_internal.h"

/***** private prototypes *****/
void lfds720_queue_umm_internal_state_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state );





/****************************************************************************/
void lfds720_queue_umm_cleanup( struct lfds720_queue_umm_state *qummhps,
                                      void (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps, struct lfds720_queue_umm_element *qummhpe, void *element_user_state, struct lfds720_smrhp_per_thread_state *smrhpts, enum lfds720_misc_flag dummy_element_flag ),
                                      void (*state_cleanup_callback)( struct lfds720_queue_umm_state *qummhps, void *element_user_state, struct lfds720_smrhp_per_thread_state *smrhpts ),
                                      struct lfds720_smrhp_per_thread_state *smrhpts )
{
  LFDS720_PAL_ASSERT( qummhps != NULL );
  // TRD : element_cleanup_callback can be NULL
  // TRD : state_cleanup_callback can be NULL
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  qummhps->state_cleanup_callback = state_cleanup_callback;
  qummhps->element_cleaned_callback = element_cleaned_callback;

  // TRD : submit the freelist SMR state for cleanup - when it emerges, we're free to deallocate the whole list
  lfds720_smrhp_submit_allocation_for_reclamation( qummhps->smrhps, smrhpts, &qummhps->smrhpas, lfds720_queue_umm_internal_state_cleaned_callback, qummhps, NULL );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_queue_umm_internal_state_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state )
{
  struct lfds720_queue_umm_element
    *qe;

  struct lfds720_queue_umm_state
    *qummhps;

  void
    *value;

  LFDS720_PAL_ASSERT( smrhpts != NULL );
  LFDS720_PAL_ASSERT( smrhpas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smrhp_user_state == NULL );
  LFDS720_PAL_ASSERT( smrhpas_user_state == NULL );

  qummhps = (struct lfds720_queue_umm_state *) allocation;

  if( qummhps->element_cleaned_callback != NULL )
  {
    while( qummhps->dequeue != qummhps->enqueue )
    {
      // TRD : trailing dummy element
      value = qummhps->dequeue->next->value;

      // TRD : user is given back *an* element, but not the one his user data was in
      qe = qummhps->dequeue;

      // TRD : remove the element from queue
      qummhps->dequeue = qummhps->dequeue->next;

      qummhps->element_cleaned_callback( qummhps, qe, smrhpas_user_state, smrhpts, LFDS720_MISC_FLAG_LOWERED );
    }

    // TRD : finally, the dummy element
    qummhps->element_cleaned_callback( qummhps, qummhps->dequeue, smrhpas_user_state, smrhpts, LFDS720_MISC_FLAG_RAISED );
  }

  if( qummhps->state_cleanup_callback != NULL )
    qummhps->state_cleanup_callback( qummhps, smrhpas_user_state, smrhpts );

  return;
}

#pragma warning( default : 4100 )

