/***** the library wide include file *****/
#include "../liblfds720_internal.h"

/***** defines *****/
// TRD : since both smrgs and smrgts store the same generation count, we have to use the larger of the two flags sets in both cases
#define LFDS720_SMRG_GET_GENERATION( generation_and_status_flags )             ((generation_and_status_flags) >> LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_COUNT)
#define LFDS720_SMRG_GET_FLAGS( generation_and_status_flags )                  ((generation_and_status_flags) &  LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_MASK)
#define LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( generation, status_flags )  ( ((generation) << LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_COUNT) | (status_flags) )

/***** private prototypes *****/
void lfds720_smrg_internal_thread_state_cleanup_function( struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue );

void lfds720_smrg_internal_allocation_cleanup_function( struct stds_list_du_state *ldus, struct stds_list_du_element *ldue );

