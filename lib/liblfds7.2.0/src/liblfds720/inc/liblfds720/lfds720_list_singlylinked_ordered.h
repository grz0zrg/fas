/***** defines *****/
#define LFDS720_LIST_SO_DELETE_BIT                                 0x1
#define LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER  0
#define LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER    1
#define LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_STORE_POINTER   2

/***** enums *****/
enum lfds720_list_so_query
{
  LFDS720_LIST_SO_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_LIST_SO_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_list_so_cursor
{
  struct lfds720_smrhp_per_thread_state
    smrhpts;
};

enum lfds720_list_so_existing_key
{
  LFDS720_LIST_SO_EXISTING_KEY_OVERWRITE,
  LFDS720_LIST_SO_EXISTING_KEY_FAIL
};

enum lfds720_list_so_insert_result
{
  LFDS720_LIST_SO_INSERT_RESULT_FAILURE_EXISTING_KEY,
  LFDS720_LIST_SO_INSERT_RESULT_SUCCESS_OVERWRITE,
  LFDS720_LIST_SO_INSERT_RESULT_SUCCESS
};

enum lfds720_list_so_remove_result
{
  LFDS720_LIST_SO_REMOVE_RESULT_FAILURE_NOT_FOUND,
  LFDS720_LIST_SO_REMOVE_RESULT_SUCCESS
};

struct lfds720_list_so_element
{
  struct lfds720_list_so_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile next;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile value;

  void
    *key;
};

struct lfds720_list_so_state
{
  struct lfds720_list_so_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile start;

  struct lfds720_list_so_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    dummy_element;

  enum lfds720_list_so_existing_key
    existing_key;

  int
    (*key_compare_function)( void const *new_key, void const *existing_key );

  struct lfds720_smrhp_state
    *smrhps;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    insert_backoff;
};

/***** public macros and prototypes *****/
void lfds720_list_nso_init_valid_on_current_logical_core( struct lfds720_list_nso_state *lasos,
                                                          int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                          enum lfds720_list_so_existing_key existing_key,
                                                          struct lfds720_smrhp_state *smrhps,
                                                          void *user_state );

void lfds720_list_so_cleanup( struct lfds720_list_so_state *lsos,
                              void (*element_cleanup_callback)(struct lfds720_list_so_state *lsos, struct lfds720_list_so_element *lsoe) );

#define LFDS720_LIST_SO_GET_KEY_FROM_ELEMENT( list_so_element )             ( (list_so_element).key )
#define LFDS720_LIST_SO_SET_KEY_IN_ELEMENT( list_so_element, new_key )      ( (list_so_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_LIST_SO_GET_VALUE_FROM_ELEMENT( list_so_element )           ( LFDS720_MISC_BARRIER_LOAD, (list_so_element).value )
#define LFDS720_LIST_SO_SET_VALUE_IN_ELEMENT( list_so_element, new_value )  { LFDS720_PAL_ATOMIC_SET( &(list_so_element).value, (void *) (lfds720_pal_uint_t) (new_value) ); }
#define LFDS720_LIST_SO_GET_USER_STATE_FROM_STATE( list_so_state )          ( (list_so_state).user_state )

enum lfds720_list_so_insert_result lfds720_list_so_insert( struct lfds720_list_so_state *lsos,
                                                           struct lfds720_list_so_element *lsoe,
                                                           struct lfds720_list_so_element *existing_lsoe,
                                                           struct lfds720_smrhp_per_thread_state *smrhppts );

struct lfds720_list_so_element *lfds720_list_so_get_start( struct lfds720_list_so_state *lsos,
                                                           struct lfds720_smrhp_per_thread_state *smrhppts );

struct lfds720_list_so_element *lfds720_list_so_get_next( struct lfds720_list_so_state *lsos,
                                                          struct lfds720_list_so_element *lsoe;
                                                          struct lfds720_smrhp_per_thread_state *smrhppts );

enum lfds720_list_so_remove_result lfds720_list_so_remove( struct lfds720_list_so_state *lsos,
                                                           void *key,
                                                           struct lfds720_smrhp_per_thread_state *smrhppts );


