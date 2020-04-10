/***** enums *****/
enum lfds720_freelist_np_query
{
  LFDS720_FREELIST_NP_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_FREELIST_NP_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_freelist_np_element
{
  ptrdiff_t
    next;

  void
    *key,
    *value;
};

struct lfds720_freelist_np_state
{
  ptrdiff_t LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    volatile top[LFDS720_MISC_PAC_SIZE];

  void LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *user_state;

  struct lfds720_misc_backoff_state
    pop_backoff,
    push_backoff;
};

/***** public macros and prototypes *****/
void lfds720_freelist_np_init_valid_on_current_logical_core( struct lfds720_freelist_np_state *fs,
                                                             void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_freelist_np_cleanup( struct lfds720_freelist_np_state *fs,
                                  void (*element_cleanup_callback)(struct lfds720_freelist_np_state *fs, struct lfds720_freelist_np_element *fe) );

#define LFDS720_FREELIST_NP_GET_KEY_FROM_ELEMENT( freelist_np_element )             ( (freelist_np_element).key )
#define LFDS720_FREELIST_NP_SET_KEY_IN_ELEMENT( freelist_np_element, new_key )      ( (freelist_np_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_FREELIST_NP_GET_VALUE_FROM_ELEMENT( freelist_np_element )           ( (freelist_np_element).value )
#define LFDS720_FREELIST_NP_SET_VALUE_IN_ELEMENT( freelist_np_element, new_value )  ( (freelist_np_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_FREELIST_NP_GET_USER_STATE_FROM_STATE( freelist_np_state )          ( (freelist_np_state).user_state )

void lfds720_freelist_np_threadsafe_push( struct lfds720_freelist_np_state *fs,
                                          struct lfds720_freelist_np_element *fe );

int lfds720_freelist_np_threadsafe_pop( struct lfds720_freelist_np_state *fs,
                                        struct lfds720_freelist_np_element **fe );

void lfds720_freelist_np_query( struct lfds720_freelist_np_state *fs,
                                enum lfds720_freelist_np_query query_type,
                                void *query_input,
                                void *query_output );


