/***** enums *****/
enum lfds720_list_nso_existing_key
{
  LFDS720_LIST_NSO_EXISTING_KEY_OVERWRITE,
  LFDS720_LIST_NSO_EXISTING_KEY_FAIL
};

enum lfds720_list_nso_insert_result
{
  LFDS720_LIST_NSO_INSERT_RESULT_FAILURE_EXISTING_KEY,
  LFDS720_LIST_NSO_INSERT_RESULT_SUCCESS_OVERWRITE,
  LFDS720_LIST_NSO_INSERT_RESULT_SUCCESS
};

enum lfds720_list_nso_query
{
  LFDS720_LIST_NSO_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_LIST_NSO_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_list_nso_element
{
  struct lfds720_list_nso_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile next;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile value;

  void
    *key;
};

struct lfds720_list_nso_state
{
  struct lfds720_list_nso_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    dummy_element;

  struct lfds720_list_nso_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *start;

  int
    (*key_compare_function)( void const *new_key, void const *existing_key );

  enum lfds720_list_nso_existing_key
    existing_key;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    insert_backoff;
};

/***** public macros and prototypes *****/
void lfds720_list_nso_init_valid_on_current_logical_core( struct lfds720_list_nso_state *lasos,
                                                          int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                          enum lfds720_list_nso_existing_key existing_key,
                                                          void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_list_nso_cleanup( struct lfds720_list_nso_state *lasos,
                               void (*element_cleanup_callback)(struct lfds720_list_nso_state *lasos, struct lfds720_list_nso_element *lasoe) );

#define LFDS720_LIST_NSO_GET_START( list_nso_state )                                             ( LFDS720_MISC_BARRIER_LOAD, (list_nso_state).start->next )
#define LFDS720_LIST_NSO_GET_NEXT( list_nso_element )                                            ( LFDS720_MISC_BARRIER_LOAD, (list_nso_element).next )
#define LFDS720_LIST_NSO_GET_START_AND_THEN_NEXT( list_nso_state, pointer_to_list_nso_element )  ( (pointer_to_list_nso_element) == NULL ? ( (pointer_to_list_nso_element) = LFDS720_LIST_NSO_GET_START(list_nso_state) ) : ( (pointer_to_list_nso_element) = LFDS720_LIST_NSO_GET_NEXT(*(pointer_to_list_nso_element)) ) )
#define LFDS720_LIST_NSO_GET_KEY_FROM_ELEMENT( list_nso_element )                                ( (list_nso_element).key )
#define LFDS720_LIST_NSO_SET_KEY_IN_ELEMENT( list_nso_element, new_key )                         ( (list_nso_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_LIST_NSO_GET_VALUE_FROM_ELEMENT( list_nso_element )                              ( LFDS720_MISC_BARRIER_LOAD, (list_nso_element).value )
#define LFDS720_LIST_NSO_SET_VALUE_IN_ELEMENT( list_nso_element, new_value )                     { LFDS720_PAL_ATOMIC_SET( (list_nso_element).value, (void *) (lfds720_pal_uint_t) (new_value) ); }
#define LFDS720_LIST_NSO_GET_USER_STATE_FROM_STATE( list_nso_state )                             ( (list_nso_state).user_state )

enum lfds720_list_nso_insert_result lfds720_list_nso_insert( struct lfds720_list_nso_state *lasos,
                                                             struct lfds720_list_nso_element *lasoe,
                                                             struct lfds720_list_nso_element **existing_lasoe );

int lfds720_list_nso_get_by_key( struct lfds720_list_nso_state *lasos,
                                 void *key,
                                 struct lfds720_list_nso_element **lasoe );

void lfds720_list_nso_query( struct lfds720_list_nso_state *lasos,
                             enum lfds720_list_nso_query query_type,
                             void *query_input,
                             void *query_output );

