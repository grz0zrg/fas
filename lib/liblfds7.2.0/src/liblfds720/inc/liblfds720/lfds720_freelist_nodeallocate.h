/***** defines *****/
#define LFDS720_FREELIST_N_ELIMINATION_ARRAY_ACTUAL_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS  ( LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES / sizeof(struct lfds720_freelist_n_element *) )
#define LFDS720_FREELIST_N_ELIMINATION_ARRAY_USED_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS    ( LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES       / sizeof(struct lfds720_freelist_n_element *) )

/***** enums *****/
enum lfds720_freelist_n_query
{
  LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT_INCLUDING_ELIMINATION_ARRAY,
  LFDS720_FREELIST_N_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_freelist_n_element
{
  struct lfds720_freelist_n_element
    *next;

  void
    *key,
    *value;
};

struct lfds720_freelist_n_per_thread_state
{
  lfds720_pal_uint_t
    elimination_array_round_robin_push_counter,
    elimination_array_round_robin_pop_counter;
};

struct lfds720_freelist_n_state
{
  struct lfds720_freelist_n_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile top[LFDS720_MISC_PAC_SIZE];

  struct lfds720_freelist_n_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    * volatile (*elimination_array)[LFDS720_FREELIST_N_ELIMINATION_ARRAY_ACTUAL_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS];

  lfds720_pal_uint_t LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    elimination_array_number_of_lines;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    pop_backoff,
    push_backoff;
};

/***** public macros and prototypes *****/
void lfds720_freelist_n_init_valid_on_current_logical_core( struct lfds720_freelist_n_state *fs,
                                                            void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

#define LFDS720_FREELIST_N_GET_ELIMINATION_SIZE_IN_BYTES( number_of_threads )              ( number_of_threads * LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES )
#define LFDS720_FREELIST_N_ELIMINATION_ALIGNMENT_IN_BYTES                                  LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES
#define LFDS720_FREELIST_N_GET_ELIMINATION_SIZE_IN_FREELIST_ELEMENTS( number_of_threads )  ( number_of_threads * LFDS720_FREELIST_N_ELIMINATION_ARRAY_USED_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS )
void lfds720_freelist_n_add_optional_elimination_array_to_improve_performance( struct lfds720_freelist_n_state *fs, 
                                                                               void *elimination_array,
                                                                               size_t elimination_array_size_in_bytes );

void lfds720_freelist_n_cleanup( struct lfds720_freelist_n_state *fs,
                                 void (*element_cleanup_callback)(struct lfds720_freelist_n_state *fs, struct lfds720_freelist_n_element *fe) );

#define LFDS720_FREELIST_N_GET_KEY_FROM_ELEMENT( freelist_n_element )             ( (freelist_n_element).key )
#define LFDS720_FREELIST_N_SET_KEY_IN_ELEMENT( freelist_n_element, new_key )      ( (freelist_n_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT( freelist_n_element )           ( (freelist_n_element).value )
#define LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT( freelist_n_element, new_value )  ( (freelist_n_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_FREELIST_N_GET_USER_STATE_FROM_STATE( freelist_n_state )          ( (freelist_n_state).user_state )

void lfds720_freelist_n_threadsafe_push( struct lfds720_freelist_n_state *fs,
                                         struct lfds720_freelist_n_per_thread_state *fpts,
                                         struct lfds720_freelist_n_element *fe );

int lfds720_freelist_n_threadsafe_pop( struct lfds720_freelist_n_state *fs,
                                       struct lfds720_freelist_n_per_thread_state *fpts,
                                       struct lfds720_freelist_n_element **fe );

void lfds720_freelist_n_query( struct lfds720_freelist_n_state *fs,
                               enum lfds720_freelist_n_query query_type,
                               void *query_input,
                               void *query_output );


