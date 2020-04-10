/***** defines *****/
#define LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS  3
  /* TRD : freelist    : 2
           list (so)   : 3 (yup, three)
           queue (umm) : 2
           stack       : 2
  */

#define LFDS720_SMRHP_DEFAULT_NUMA_NODE_ID  0

/***** enums *****/
enum lfds720_smrhppts_state
{
  LFDS720_SMRHPPTS_STATE_AVAILABLE,
  LFDS720_SMRHPPTS_STATE_ACTIVE,
  LFDS720_SMRHPPTS_STATE_RETIRED,
  LFDS720_SMRHPPTS_STATE_RETIRING
};

enum lfds720_smrhp_query
{
  LFDS720_SMRHP_QUERY_SINGLETHREADED_COUNT_THREAD_STATES,
  LFDS720_SMRHP_QUERY_SINGLETHREADED_VALIDATE
};

enum lfds720_smrhp_thread_query
{
  LFDS720_SMRHP_THREAD_QUERY_SINGLETHREADED_VALIDATE
};

/***** incomplete types *****/
struct lfds720_smrhp_per_thread_state;

/***** structs *****/
struct lfds720_smrhp_state
{
  struct lfds720_list_nsu_state LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    list_of_smrhp_per_thread_states;

  void
    (*per_thread_state_deallocation_callback)( struct lfds720_smrhp_per_thread_state *smrhppts ),
    *smrhps_user_state;

  int unsigned
    state_has_been_initialized_safety_check_bitpattern;
};

struct lfds720_smrhp_per_thread_state
{
  enum lfds720_smrhppts_state volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    status;

  lfds720_pal_uint_t LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    numa_node_id;

  struct lfds720_smrhp_state
    *smrhps;

  struct stds_list_du_state
    list_of_allocations_pending_reclamation;

  struct lfds720_list_nsu_element
    asle;

  void
    * volatile hazard_pointers[LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS];

  void
    *smrhppts_user_state;
};

struct lfds720_smrhp_allocation_state
{
  int
    count;

  enum lfds720_misc_flag
    pointed_at_flag;

  struct stds_list_du_element
    ldue;

  void
    (*allocation_reclaimed_callback)( struct lfds720_smrhp_per_thread_state *smrhppts, struct lfds720_smrhp_allocation_state *smrhpas, void *allocation, void *smrhps_user_state, void *smrhpas_user_state ),
    *smrhpas_user_state,
    *allocation;
};

struct lfds720_smrhp_per_thread_state_counts
{
  lfds720_pal_uint_t
    active_states,
    available_states,
    retired_states;
};

struct lfds720_smrhp_validation_info
{
  enum lfds720_misc_flag
    check_expected_smrhptsc_flag;

  struct lfds720_smrhp_per_thread_state_counts
    expected_smrhptsc;
};

struct lfds720_smrhp_thread_validation_info
{
  enum lfds720_misc_flag
    check_expected_status_flag,
    check_expected_numa_node_id_flag,
    check_expected_expected_number_of_pending_reclamations_flag,
    check_expected_hazard_pointers_flag;

  enum lfds720_smrhppts_state
    expected_status;

  lfds720_pal_uint_t
    expected_numa_node_id;

  struct lfds720_misc_validation_info
    expected_number_of_pending_reclamations;

  void
    *expected_hazard_pointers[LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS];
};

/***** public macros and prototypes *****/
void lfds720_smrhp_init_state( struct lfds720_smrhp_state *smrhps, void *smrhps_user_state );

void lfds720_smrhp_init_per_thread_state( struct lfds720_smrhp_per_thread_state *smrhppts, lfds720_pal_uint_t numa_node_id );

void lfds720_smrhp_register_thread_using_new_smrhp_per_thread_state( struct lfds720_smrhp_state *smrhps,
                                                                     struct lfds720_smrhp_per_thread_state *smrhppts );

void lfds720_smrhp_deregister_thread( struct lfds720_smrhp_state *smrhps,
                                      struct lfds720_smrhp_per_thread_state *smrhppts );

int lfds720_smrhp_register_thread_using_existing_available_smrhp_per_thread_state( struct lfds720_smrhp_state *smrhps,
                                                                                   lfds720_pal_uint_t numa_node_id,
                                                                                   struct lfds720_smrhp_per_thread_state **smrhppts );

void lfds720_smrhp_cleanup( struct lfds720_smrhp_state *smrhps,
                            void (*per_thread_state_deallocation_callback)(struct lfds720_smrhp_per_thread_state *smrhppts) );

/*
#define LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhppts, hp_number, allocation )  {                                                                  \
                                                                                          do                                                               \
                                                                                          {                                                                \
                                                                                            (smrhppts).hazard_pointers[hp_number] = (allocation);          \
                                                                                            LFDS720_MISC_BARRIER_LOAD;                                     \
                                                                                          }                                                                \
                                                                                          while( (smrhppts).hazard_pointers[hp_number] != (allocation) );  \
                                                                                          LFDS720_MISC_BARRIER_STORE;                                      \
                                                                                        }

#define LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhppts, hp_number, allocation )  {                                                        \
                                                                                          (smrhppts).hazard_pointers[hp_number] = (allocation);  \
                                                                                          LFDS720_MISC_BARRIER_FULL;                             \
                                                                                        }

*/

#define LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhppts, hp_number, allocation )  (smrhppts).hazard_pointers[hp_number] = (allocation)

#define LFDS720_SMRHP_GET_HAZARD_POINTER( smrhppts, hp_number )                         (smrhppts).hazard_pointers[hp_number]

/*

#define LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhppts, hp_number )                {                                                \
                                                                                          LFDS720_MISC_BARRIER_FULL;                     \
                                                                                          (smrhppts).hazard_pointers[hp_number] = NULL;  \
                                                                                        }

*/

#define LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhppts, hp_number )                (smrhppts).hazard_pointers[hp_number] = NULL

void lfds720_smrhp_submit_allocation_for_reclamation( struct lfds720_smrhp_state *smrhps,
                                                      struct lfds720_smrhp_per_thread_state *smrhppts,
                                                      struct lfds720_smrhp_allocation_state *smrhpas,
                                                      void (*allocation_reclaimed_callback)( struct lfds720_smrhp_per_thread_state *smrhppts, struct lfds720_smrhp_allocation_state *smrhpas, void *allocation, void *smrhps_user_state, void *smrhpas_user_state ),
                                                      void *allocation,
                                                      void *smrhpas_user_state );

lfds720_pal_uint_t lfds720_smrhp_reclaim_reclaimable_allocations_submitted_by_current_thread( struct lfds720_smrhp_state *smrhps,
                                                                                              struct lfds720_smrhp_per_thread_state *smrhppts );
  /* TRD : an allocation submitted for reclaimation can only be reclaimed if no thread has an outstanding LFDS720_SMRHP_MAKE_POINTER_UNRECLAIMABLE() issued on that allocation
           reclamation typically has the user free()ing the allocation, but not necessary - it might be returned to a freelist, say
           so the verb "reclame" must be used, not "deallocate"
           threads *never* actually themselves call free() or push allocations back to a freelist - they must and must only call lfds720_smrhp_submit_allocation_for_reclamation()
  */

void lfds720_smrhp_query( struct lfds720_smrhp_state *smrhps, enum lfds720_smrhp_query query_type, void *query_input, void *query_output );

void lfds720_smrhp_thread_query( struct lfds720_smrhp_per_thread_state *smrhpts, enum lfds720_smrhp_thread_query query_type, void *query_input, void *query_output );
