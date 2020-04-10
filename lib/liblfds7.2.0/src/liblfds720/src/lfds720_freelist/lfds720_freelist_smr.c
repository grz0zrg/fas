/***** includes *****/
#include "lfds720_freelist_internal.h"

/***** private prototypes *****/
void lfds720_freelist_internal_allocation_reclaimed_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state );





/****************************************************************************/
void lfds720_freelist_clean_dirty_element( struct lfds720_freelist_state *fs,
                                           struct lfds720_freelist_element *fe,
                                           void *element_user_state,
                                           void (*element_cleaned_callback)( struct lfds720_freelist_state *fs,
                                                                             struct lfds720_freelist_element *fe,
                                                                             void *element_user_state,
                                                                             struct lfds720_smrhp_per_thread_state *smrhpts ),
                                           struct lfds720_smrhp_per_thread_state *smrhpts )
{
  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( fe != NULL );
  // TRD : element_user_state can be NULL
  LFDS720_PAL_ASSERT( element_cleaned_callback != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  fe->fs = fs;
  fe->element_cleaned_callback = element_cleaned_callback;

  lfds720_smrhp_submit_allocation_for_reclamation( fs->smrhps, smrhpts, &fe->smrhpas, lfds720_freelist_internal_allocation_reclaimed_callback, fe, element_user_state );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_freelist_internal_allocation_reclaimed_callback( struct lfds720_smrhp_per_thread_state *smrhpts, 
                                                              struct lfds720_smrhp_allocation_state *smrhpas,
                                                              void *allocation,
                                                              void *smrhp_user_state,
                                                              void *smrhpas_user_state )
{
  struct lfds720_freelist_element
    *fe;

  LFDS720_PAL_ASSERT( smrhpts != NULL );
  LFDS720_PAL_ASSERT( smrhpas != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  LFDS720_PAL_ASSERT( smrhp_user_state == NULL );
  // TRD : smrhpas_user_state can be NULL

  fe = (struct lfds720_freelist_element *) allocation;

  fe->element_cleaned_callback( fe->fs, fe, smrhpas_user_state, smrhpts );

  return;
}

#pragma warning( default : 4100 )

