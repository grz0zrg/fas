/***** enums *****/
enum lfds720_btree_nu_absolute_position
{
  LFDS720_BTREE_NU_ABSOLUTE_POSITION_ROOT,
  LFDS720_BTREE_NU_ABSOLUTE_POSITION_SMALLEST_IN_TREE,
  LFDS720_BTREE_NU_ABSOLUTE_POSITION_LARGEST_IN_TREE
}; 

enum lfds720_btree_nu_existing_key
{
  LFDS720_BTREE_NU_EXISTING_KEY_OVERWRITE,
  LFDS720_BTREE_NU_EXISTING_KEY_FAIL
};

enum lfds720_btree_nu_insert_result
{
  LFDS720_BTREE_NU_INSERT_RESULT_FAILURE_EXISTING_KEY,
  LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS_OVERWRITE,
  LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS
};

enum lfds720_btree_nu_query
{
  LFDS720_BTREE_NU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_BTREE_NU_QUERY_SINGLETHREADED_VALIDATE
};

enum lfds720_btree_nu_relative_position
{
  LFDS720_BTREE_NU_RELATIVE_POSITION_UP,
  LFDS720_BTREE_NU_RELATIVE_POSITION_LEFT,
  LFDS720_BTREE_NU_RELATIVE_POSITION_RIGHT,
  LFDS720_BTREE_NU_RELATIVE_POSITION_SMALLEST_ELEMENT_BELOW_CURRENT_ELEMENT,
  LFDS720_BTREE_NU_RELATIVE_POSITION_LARGEST_ELEMENT_BELOW_CURRENT_ELEMENT,
  LFDS720_BTREE_NU_RELATIVE_POSITION_NEXT_SMALLER_ELEMENT_IN_ENTIRE_TREE,
  LFDS720_BTREE_NU_RELATIVE_POSITION_NEXT_LARGER_ELEMENT_IN_ENTIRE_TREE
};

/***** structs *****/
struct lfds720_btree_nu_element
{
  /* TRD : we are add-only, so these elements are only written once
           as such, the write is wholly negligible
           we are only concerned with getting as many structs in one cache line as we can
  */

  struct lfds720_btree_nu_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile left,
    *volatile right,
    *volatile up;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile value;

  void
    *key;
};

struct lfds720_btree_nu_state
{
  struct lfds720_btree_nu_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile root;

  int
    (*key_compare_function)( void const *new_key, void const *existing_key );

  enum lfds720_btree_nu_existing_key 
    existing_key;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    insert_backoff;
};

/***** public macros and prototypes *****/
void lfds720_btree_nu_init_valid_on_current_logical_core( struct lfds720_btree_nu_state *baus,
                                                           int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                           enum lfds720_btree_nu_existing_key existing_key,
                                                           void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_btree_nu_cleanup( struct lfds720_btree_nu_state *baus,
                               void (*element_cleanup_callback)(struct lfds720_btree_nu_state *baus, struct lfds720_btree_nu_element *baue) );

#define LFDS720_BTREE_NU_GET_KEY_FROM_ELEMENT( btree_nu_element )             ( (btree_nu_element).key )
#define LFDS720_BTREE_NU_SET_KEY_IN_ELEMENT( btree_nu_element, new_key )      ( (btree_nu_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_BTREE_NU_GET_VALUE_FROM_ELEMENT( btree_nu_element )           ( LFDS720_MISC_BARRIER_LOAD, (btree_nu_element).value )
#define LFDS720_BTREE_NU_SET_VALUE_IN_ELEMENT( btree_nu_element, new_value )  { LFDS720_PAL_ATOMIC_SET( (btree_nu_element).value, (void *) (lfds720_pal_uint_t) (new_value) ); }
#define LFDS720_BTREE_NU_GET_USER_STATE_FROM_STATE( btree_nu_state )          ( (btree_nu_state).user_state )

enum lfds720_btree_nu_insert_result lfds720_btree_nu_insert( struct lfds720_btree_nu_state *baus,
                                                             struct lfds720_btree_nu_element *baue,
                                                             struct lfds720_btree_nu_element **existing_baue );
  // TRD : if an insert collides with an existing key and existing_baue is non-NULL, existing_baue is set to the existing element

int lfds720_btree_nu_get_by_key( struct lfds720_btree_nu_state *baus, 
                                 int (*key_compare_function)(void const *new_key, void const *existing_key),
                                 void *key,
                                 struct lfds720_btree_nu_element **baue );

int lfds720_btree_nu_get_by_absolute_position_and_then_by_relative_position( struct lfds720_btree_nu_state *baus,
                                                                             struct lfds720_btree_nu_element **baue,
                                                                             enum lfds720_btree_nu_absolute_position absolute_position,
                                                                             enum lfds720_btree_nu_relative_position relative_position );
  // TRD : if *baue is NULL, we get the element at absolute_position, otherwise we move from *baue according to relative_position

int lfds720_btree_nu_get_by_absolute_position( struct lfds720_btree_nu_state *baus,
                                               struct lfds720_btree_nu_element **baue,
                                               enum lfds720_btree_nu_absolute_position absolute_position );

int lfds720_btree_nu_get_by_relative_position( struct lfds720_btree_nu_element **baue,
                                               enum lfds720_btree_nu_relative_position relative_position );

void lfds720_btree_nu_query( struct lfds720_btree_nu_state *baus,
                             enum lfds720_btree_nu_query query_type,
                             void *query_input,
                             void *query_output );

