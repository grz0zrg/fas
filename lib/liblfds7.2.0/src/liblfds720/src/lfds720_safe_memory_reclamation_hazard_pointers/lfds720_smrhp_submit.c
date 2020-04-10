/***** includes *****/
#include "lfds720_smrhp_internal.h"


#include <stdio.h>
#include <stdlib.h>



/****************************************************************************/
void lfds720_smrhp_submit_allocation_for_reclamation( struct lfds720_smrhp_state *smrhps,
                                                      struct lfds720_smrhp_per_thread_state *smrhppts,
                                                      struct lfds720_smrhp_allocation_state *smrhpas,
                                                      void (*allocation_reclaimed_callback)( struct lfds720_smrhp_per_thread_state *smrhppts, struct lfds720_smrhp_allocation_state *smrhpas, void *allocation, void *smrhps_user_state, void *smrhpas_user_state ),
                                                      void *allocation,
                                                      void *smrhpas_user_state )
{
  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  LFDS720_PAL_ASSERT( smrhppts != NULL );
  LFDS720_PAL_ASSERT( smrhpas != NULL );
  LFDS720_PAL_ASSERT( allocation_reclaimed_callback != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  // TRD : smrhpas_user_state can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  /* TRD : we're handed a dirty allocation
           place it into the thread-local reuse candidates list
  */

  if( allocation == ((struct lfds720_freelist_element *) allocation)->next )
  {
    printf( "submit loop = %p, next = %p\n", (void *) allocation, ((struct lfds720_freelist_element *) allocation)->next );
    exit( EXIT_FAILURE );
  }

  smrhpas->pointed_at_flag = LFDS720_MISC_FLAG_LOWERED;
  smrhpas->allocation = allocation;
  smrhpas->smrhpas_user_state = smrhpas_user_state;
  smrhpas->allocation_reclaimed_callback = allocation_reclaimed_callback;
  smrhpas->count++;

  if( smrhpas->count != 1 )
  {
    printf( "allocation = %p, next = %p, %d\n", (void *) allocation, ((struct lfds720_freelist_element *) allocation)->next, (int) smrhpas->count );
    exit( EXIT_FAILURE );
  }

  // TRD : needs to be linked to the end of the list, so we reuse in the order of arrival
  STDS_LIST_DU_SET_VALUE_IN_ELEMENT( smrhpas->ldue, smrhpas );
  stds_list_du_insert_at_end( &smrhppts->list_of_allocations_pending_reclamation, &smrhpas->ldue );

  return;
}

