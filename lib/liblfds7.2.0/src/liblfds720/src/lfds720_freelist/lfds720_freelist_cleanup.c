/***** includes *****/
#include "lfds720_freelist_internal.h"

/***** private prototypes *****/
void lfds720_freelist_internal_state_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state );





/****************************************************************************/
void lfds720_freelist_cleanup( struct lfds720_freelist_state *fs,
                                void *element_user_state,
                                void (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                                                  struct lfds720_freelist_element *fe,
                                                                  void *element_user_state,
                                                                  struct lfds720_smrhp_per_thread_state *smrhpts ),
                                void (*state_cleanup_callback)( struct lfds720_freelist_state *fs,
                                                                void *element_user_state,
                                                                struct lfds720_smrhp_per_thread_state *smrhpts ),
                                struct lfds720_smrhp_per_thread_state *smrhpts )
{
  LFDS720_PAL_ASSERT( fs != NULL );
  // TRD : element_user_state can be NULL
  // TRD : element_cleanup_callback can be NULL
  // TRD : state_cleanup_callback can be NULL
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  fs->state_cleanup_callback = state_cleanup_callback;
  fs->element_cleaned_callback = element_cleaned_callback;

  // TRD : submit the freelist SMR state for cleanup - when it emerges, we're free to deallocate the whole list
  lfds720_smrhp_submit_allocation_for_reclamation( fs->smrhps, smrhpts, &fs->smrhpas, lfds720_freelist_internal_state_cleaned_callback, fs, element_user_state );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_freelist_internal_state_cleaned_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state )
{
  struct lfds720_freelist_element
    *fe,
    *fe_temp;

  struct lfds720_freelist_state
    *fs;

  LFDS720_PAL_ASSERT( smrhpts != NULL );
  LFDS720_PAL_ASSERT( smrhpas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smrhp_user_state == NULL );
  LFDS720_PAL_ASSERT( smrhpas_user_state == NULL );

  fs = (struct lfds720_freelist_state *) allocation;

  if( fs->element_cleaned_callback != NULL )
  {
    fe = fs->top;

    while( fe != NULL )
    {
      fe_temp = fe;
      fe = fe->next;

      fs->element_cleaned_callback( fs, fe_temp, smrhpas_user_state, smrhpts );
    }
  }

  if( fs->state_cleanup_callback != NULL )
    fs->state_cleanup_callback( fs, smrhpas_user_state, smrhpts );

  return;
}

#pragma warning( default : 4100 )

