/***** includes *****/
#include "lfds720_freelist_smrg_internal.h"

/***** private prototypes *****/
void lfds720_freelist_smrg_internal_allocation_reclaimed_callback( struct lfds720_smrg_thread_state *smrgts,
                                                                   struct lfds720_smrg_allocation_state *smrgas,
                                                                   void *allocation,
                                                                   void *smrg_user_state,
                                                                   void *allocation_user_state );





/****************************************************************************/
void lfds720_freelist_smrg_clean_dirty_element( struct lfds720_freelist_smrg_state *fsgs,
                                                struct lfds720_freelist_smrg_element *fsge,
                                                void *element_user_state,
                                                void (*element_cleaned_callback)( struct lfds720_freelist_smrg_state *fsgs,
                                                                                  struct lfds720_freelist_smrg_element *fsge,
                                                                                  void *element_user_state,
                                                                                  struct lfds720_smrg_thread_state *smrgts ),
                                                struct lfds720_smrg_thread_state *smrgts )
{
  LFDS720_PAL_ASSERT( fsgs != NULL );
  LFDS720_PAL_ASSERT( fsge != NULL );
  // TRD : element_user_state can be NULL
  LFDS720_PAL_ASSERT( element_cleaned_callback != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );

  fsge->fsgs = fsgs;
  fsge->element_cleaned_callback = element_cleaned_callback;

  lfds720_smrg_submit_dirty_allocation( fsgs->smrgs, smrgts, &fsge->smrgas, lfds720_freelist_smrg_internal_allocation_reclaimed_callback, fsge, element_user_state );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_freelist_smrg_internal_allocation_reclaimed_callback( struct lfds720_smrg_thread_state *smrgts,
                                                                   struct lfds720_smrg_allocation_state *smrgas,
                                                                   void *allocation,
                                                                   void *smrg_user_state,
                                                                   void *smrgas_user_state )
{
  struct lfds720_freelist_smrg_element
    *fsge;

  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( smrgas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smrg_user_state == NULL );
  // TRD : smrgas_user_state can be NULL

  fsge = (struct lfds720_freelist_smrg_element *) allocation;

  fsge->element_cleaned_callback( fsge->fsgs, fsge, smrgas_user_state, smrgts );

  return;
}

#pragma warning( default : 4100 )

