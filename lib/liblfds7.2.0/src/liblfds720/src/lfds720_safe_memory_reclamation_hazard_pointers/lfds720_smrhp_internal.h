/***** the library wide include file *****/
#include "../liblfds720_internal.h"

/***** defines *****/

/***** private prototypes *****/
lfds720_pal_uint_t lfds720_smrhp_internal_reclaim_reclaimable_allocations_submitted_by_current_thread( struct lfds720_smrhp_state *smrhps,
                                                                                                       struct lfds720_smrhp_per_thread_state *smrhppts );

void lfds720_smrhp_internal_allocation_pending_reclamation_cleanup_function( struct stds_list_du_state *ldus, struct stds_list_du_element *ldue );

