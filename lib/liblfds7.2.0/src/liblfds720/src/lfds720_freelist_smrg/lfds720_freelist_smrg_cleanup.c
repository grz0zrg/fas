/***** includes *****/
#include "lfds720_freelist_smrg_internal.h"

/***** private prototypes *****/
void lfds720_freelist_smrg_internal_state_cleaned_callback( struct lfds720_smrg_thread_state *smrgts, 
                                                            struct lfds720_smrg_allocation_state *smrgas,
                                                            void *allocation,
                                                            void *smr_user_state,
                                                            void *allocation_user_state );





/****************************************************************************/
void lfds720_freelist_smrg_cleanup( struct lfds720_freelist_smrg_state *fsgs,
                                    void (*element_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs, struct lfds720_freelist_smrg_element *fsge ),
                                    void (*state_cleanup_callback)( struct lfds720_freelist_smrg_state *fsgs ),
                                    struct lfds720_smrg_thread_state *smrgts )
{
  LFDS720_PAL_ASSERT( fsgs != NULL );
  // TRD : element_cleanup_callback can be NULL
  // TRD : state_cleanup_callback can be NULL
  LFDS720_PAL_ASSERT( smrgts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  fsgs->state_cleanup_callback = state_cleanup_callback;
  fsgs->element_cleanup_callback = element_cleanup_callback;

  // TRD : submit the freelist SMR state for cleanup - when it emerges, we're free to deallocate the whole list
  lfds720_smrg_submit_dirty_allocation( fsgs->smrgs, smrgts, &fsgs->smrgas, lfds720_freelist_smrg_internal_state_cleaned_callback, fsgs, NULL );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_freelist_smrg_internal_state_cleaned_callback( struct lfds720_smrg_thread_state *smrgts, 
                                                            struct lfds720_smrg_allocation_state *smrgas,
                                                            void *allocation,
                                                            void *smr_user_state,
                                                            void *allocation_user_state )
{
  struct lfds720_freelist_smrg_element
    *fsge,
    *fsge_temp;

  struct lfds720_freelist_smrg_state
    *fsgs;

  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( smrgas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smr_user_state == NULL );
  LFDS720_PAL_ASSERT( allocation_user_state == NULL );

  fsgs = (struct lfds720_freelist_smrg_state *) allocation;

  if( fsgs->element_cleanup_callback != NULL )
  {
    fsge = fsgs->top;

    while( fsge != NULL )
    {
      fsge_temp = fsge;
      fsge = fsge->next;

      fsgs->element_cleanup_callback( fsgs, fsge_temp );
    }
  }

  if( fsgs->state_cleanup_callback != NULL )
    fsgs->state_cleanup_callback( fsgs );

  return;
}

#pragma warning( default : 4100 )

