/***** includes *****/
#include "lfds720_queue_umm_internal.h"

/***** private prototypes *****/
void lfds720_queue_umm_internal_allocation_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                                   struct lfds720_smrhp_allocation_state *smrhpas,
                                                                   void *allocation,
                                                                   void *smrhp_user_state,
                                                                   void *smrhpas_user_state );





/****************************************************************************/
void lfds720_queue_umm_clean_dirty_element( struct lfds720_queue_umm_state *qummhps,
                                                  struct lfds720_queue_umm_element *qummhpe,
                                                  void *element_user_state,
                                                  void (*element_cleaned_callback)( struct lfds720_queue_umm_state *qummhps,
                                                                                    struct lfds720_queue_umm_element *qummhpe,
                                                                                    void *element_user_state,
                                                                                    struct lfds720_smrhp_per_thread_state *smrhpts ),
                                                  struct lfds720_smrhp_per_thread_state *smrhpts )
{
  LFDS720_PAL_ASSERT( qummhps != NULL );
  LFDS720_PAL_ASSERT( qummhpe != NULL );
  // TRD : element_user_state can be NULL
  LFDS720_PAL_ASSERT( element_cleaned_callback != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  qummhpe->qummhps = qummhps;
  qummhpe->element_cleaned_callback = element_cleaned_callback;

  lfds720_smrhp_submit_allocation_for_reclamation( qummhps->smrhps, smrhpts, &qummhpe->smrhpas, lfds720_queue_umm_internal_allocation_cleaned_callback, qummhpe, element_user_state );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_queue_umm_internal_allocation_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                                   struct lfds720_smrhp_allocation_state *smrhpas,
                                                                   void *allocation,
                                                                   void *smrhp_user_state,
                                                                   void *smrhpas_user_state )
{
  struct lfds720_queue_umm_element
    *qummhpe;

  LFDS720_PAL_ASSERT( smrhpts != NULL );
  LFDS720_PAL_ASSERT( smrhpas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smrhp_user_state == NULL );
  // TRD : smrhpas_user_state can be NULL

  qummhpe = (struct lfds720_queue_umm_element *) allocation;

  qummhpe->element_cleaned_callback( qummhpe->qummhps, qummhpe, smrhpas_user_state, smrhpts );

  return;
}

#pragma warning( default : 4100 )

