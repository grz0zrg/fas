/***** defines *****/
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS  0x1
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE       0x2
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE             0x4
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW            0x8
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE                0x10
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED               0x20
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRING              0x40
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_COUNT                 7
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_MASK                  (LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRING)
#define LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_REDUCED_MASK          (                                                                                                                     LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED                                                 )

#define LFDS720_SMRG_SMRGS_FLAG_STATUS_SETTING   0x1
#define LFDS720_SMRG_SMRGS_FLAG_STATUS_CLEARING  0x2
#define LFDS720_SMRG_SMRGS_FLAG_STATUS_COUNT     2
#define LFDS720_SMRG_SMRGS_FLAG_STATUS_MASK      (LFDS720_SMRG_SMRGS_FLAG_STATUS_SETTING | LFDS720_SMRG_SMRGS_FLAG_STATUS_CLEARING)

#define LFDS720_SMRG_DEFAULT_NUMA_NODE_ID       0

/***** enums *****/
enum lfds720_smrg_query
{
  LFDS720_SMRG_QUERY_SINGLETHREADED_COUNT_THREAD_STATES,
  LFDS720_SMRG_QUERY_SINGLETHREADED_VALIDATE
};

enum lfds720_smrg_thread_query
{
  LFDS720_SMRG_THREAD_QUERY_SINGLETHREADED_VALIDATE
};

/***** incomplete types *****/
struct lfds720_smrg_thread_state;

/***** structs *****/
struct lfds720_smrg_thread_state
{
  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    generation_count_and_status_flags;

  lfds720_pal_uint_t LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    numa_node_id;

  struct lfds720_smrg_state
    *smrgs;

  struct lfds720_list_nsu_element
    asle;

  struct stds_list_du_state
    allocations;
};

struct lfds720_smrg_state
{
  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    generation_count_and_status_flags;

  struct lfds720_list_nsu_state LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    smrg_thread_states;

  void
    (*thread_state_deallocation_callback)( struct lfds720_smrg_thread_state *smrgts ),
    *smrg_user_state;
};

struct lfds720_smrg_thread_state_counts
{
  lfds720_pal_uint_t
    active_new_states,
    active_states,
    available_states,
    retired_states;
};

struct lfds720_smrg_allocation_state
{
  lfds720_pal_uint_t
    generation_count;

  struct stds_list_du_element
    stle;

  void
    (*allocation_cleaned_callback)( struct lfds720_smrg_thread_state *smrgts, struct lfds720_smrg_allocation_state *smrgas, void *value, void *smrg_user_state, void *allocation_user_state ),
    *allocation_user_state,
    *allocation;
};

struct lfds720_smrg_thread_validation_info
{
  enum lfds720_misc_flag
    check_expected_generation_count_flag,
    check_expected_expected_number_of_allocations_flag,
    check_expected_numa_node_id_flag,
    check_expected_status_flags_bitmask;

  lfds720_pal_uint_t
    expected_generation_count,
    expected_status_flags_bitmask,
    expected_numa_node_id;

  struct lfds720_misc_validation_info
    expected_number_of_allocations;
};

struct lfds720_smrg_validation_info
{
  enum lfds720_misc_flag
    check_expected_generation_count_flag,
    check_expected_smrgtsc_flag;

  lfds720_pal_uint_t
    check_expected_status_flags_bitmask,
    expected_generation_count,
    expected_status_flags_bitmask;

  struct lfds720_smrg_thread_state_counts
    expected_smrgtsc;
};

/***** public macros and prototypes *****/
void lfds720_smrg_init_smr( struct lfds720_smrg_state *smrgs, void *smrg_user_state );

void lfds720_smrg_cleanup( struct lfds720_smrg_state *smrgs, void (*thread_state_deallocation_callback)(struct lfds720_smrg_thread_state *smrgts) );

void lfds720_smrg_init_smrg_thread( struct lfds720_smrg_thread_state *smrgts, lfds720_pal_uint_t numa_node_id );

void lfds720_smrg_register_thread_using_new_smrg_thread_state( struct lfds720_smrg_state *smrgs,
                                                              struct lfds720_smrg_thread_state *smrgts );
  // TRD : threads must register with the SMR state before they can use the API; allocate *smrgts in the NUMA node of the CPU the thread will run on

void lfds720_smrg_deregister_thread( struct lfds720_smrg_state *smrgs, struct lfds720_smrg_thread_state *smrgts );
  // TRD : a deregistered thread's smrg_thread_state is never freed, but it is available for re-use, via the lfds720_smrg_register_thread_using_existing_available_smrg_thread_state() call

int lfds720_smrg_register_thread_using_existing_available_smrg_thread_state( struct lfds720_smrg_state *smrgs,
                                                                           lfds720_pal_uint_t numa_node_id,
                                                                           struct lfds720_smrg_thread_state **smrgts );
  // TRD : so when registering a new thread, call the re-use function; if it fails, then allocate a new smrg_thread_state and use that
  //       this function gives you back a pointer to the re-used smrg_thread_state, because you'll need it for other function calls later

/*
#define LFDS720_SMRG_THREAD_BEGIN_LOCKFREE_OPERATIONS( pointer_to_smrgts )  {                                                                                                                        \
                                                                              (pointer_to_smrgts)->generation_count_and_status_flags |= LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS;  \
                                                                              LFDS720_MISC_BARRIER_STORE;                                                                                            \
                                                                              lfds720_misc_force_store();                                                                                            \
                                                                            }
*/

#define LFDS720_SMRG_THREAD_BEGIN_LOCKFREE_OPERATIONS( smrgts )                                                                                                                                                             \
{                                                                                                                                                                                                                           \
  char unsigned                                                                                                                                                                                                             \
    result;                                                                                                                                                                                                                 \
                                                                                                                                                                                                                            \
  lfds720_pal_uint_t                                                                                                                                                                                                        \
    original_destination;                                                                                                                                                                                                   \
                                                                                                                                                                                                                            \
  do                                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                         \
    original_destination = (smrgts).generation_count_and_status_flags;                                                                                                                                                      \
    LFDS720_PAL_ATOMIC_CAS( (smrgts).generation_count_and_status_flags, original_destination, original_destination | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS, LFDS720_MISC_CAS_STRENGTH_WEAK, result );  \
  }                                                                                                                                                                                                                         \
  while( result == 0 );                                                                                                                                                                                                     \
}

/*
#define LFDS720_SMRG_THREAD_END_LOCKFREE_OPERATIONS( pointer_to_smrgts )    {                                                                                                                                                                                                                                             \
                                                                              (pointer_to_smrgts)->generation_count_and_status_flags = (((pointer_to_smrgts)->generation_count_and_status_flags | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE) & ~LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS);  \
                                                                              LFDS720_MISC_BARRIER_STORE;                                                                                                                                                                                                                 \
                                                                            }
*/

#define LFDS720_SMRG_THREAD_END_LOCKFREE_OPERATIONS( smrgts )                                                                                                                                                                                                                            \
{                                                                                                                                                                                                                                                                                        \
  char unsigned                                                                                                                                                                                                                                                                          \
    result;                                                                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                                                                         \
  lfds720_pal_uint_t                                                                                                                                                                                                                                                                     \
    original_destination;                                                                                                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                         \
  do                                                                                                                                                                                                                                                                                     \
  {                                                                                                                                                                                                                                                                                      \
    original_destination = (smrgts).generation_count_and_status_flags;                                                                                                                                                                                                                   \
    LFDS720_PAL_ATOMIC_CAS( (smrgts).generation_count_and_status_flags, original_destination, ((original_destination | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE) & ~LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS), LFDS720_MISC_CAS_STRENGTH_WEAK, result );  \
  }                                                                                                                                                                                                                                                                                      \
  while( result == 0 );                                                                                                                                                                                                                                                                  \
}

void lfds720_smrg_submit_dirty_allocation( struct lfds720_smrg_state *smrgs,
                                           struct lfds720_smrg_thread_state *smrgts,
                                           struct lfds720_smrg_allocation_state *smrgas,
                                           void (*allocation_cleaned_callback)( struct lfds720_smrg_thread_state *smrgts, struct lfds720_smrg_allocation_state *smrgas, void *allocation, void *smrg_user_state, void *smrgas_user_state ),
                                           void *allocation,
                                           void *smrgas_user_state );
  // TRD : a dirty allocation is any allocation which a lock-free data structure could access

void lfds720_smrg_clean_eligible_dirty_allocations_for_all_threads( struct lfds720_smrg_state *smrgs, enum lfds720_misc_flag *generation_flag, enum lfds720_misc_flag *cleaning_flag );
  /* TRD : a dirty allocation can be cleaned if, after its submission, all threads registered with the SMR state
           have either called LFDS720_SMRG_THREAD_END_LOCKFREE_OPERATIONS twice (yes, twice!) or have not called LFDS720_SMRG_THREAD_BEGIN_LOCKFREE_OPERATIONS at all
           returns 0 if it is known no elements were cleaned, 1 if it is known one or more elemnts were cleaned
  */

lfds720_pal_uint_t lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( struct lfds720_smrg_state *smrgs, struct lfds720_smrg_thread_state *smrgts );
  /* TRD : although cleaning is performed for all threads at once (we end up incrementing a generation counter, if it's safe to do so),
           actually getting lfds720_smrg_release_all_clean_allocations_submitted_by_current_thread() called occurs on a per-thread basis (because
           submitted elements are stored in a single-threaded list per thread - that's part of what lfds720_smrg_thread_state is for)
           returns count of released elements (which of course can be zero)
  */

void lfds720_smrg_flush( struct lfds720_smrg_state *smrgs );
  // TRD : this is a non-thread-safe utility function needed by the test application; users NEVER need call this functions

void lfds720_smrg_query( struct lfds720_smrg_state *smrgs, enum lfds720_smrg_query query_type, void *query_input, void *query_output );
void lfds720_smrg_thread_query( struct lfds720_smrg_thread_state *smrgts, enum lfds720_smrg_thread_query query_type, void *query_input, void *query_output );

