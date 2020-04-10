/***** defines *****/
#define and &&
#define or  ||

#define STDS_MISC_DELIBERATELY_CRASH  { char *c = 0; *c = 0; }

/***** enums *****/
enum stds_misc_flag
{
  STDS_MISC_FLAG_LOWERED,
  STDS_MISC_FLAG_RAISED
};

enum stds_misc_validity
{
  STDS_MISC_VALIDITY_UNKNOWN,
  STDS_MISC_VALIDITY_VALID,
  STDS_MISC_VALIDITY_INVALID_LOOP,
  STDS_MISC_VALIDITY_INVALID_MISSING_ELEMENTS,
  STDS_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS,
  STDS_MISC_VALIDITY_INVALID_TEST_DATA,
  STDS_MISC_VALIDITY_INVALID_ORDER,
  STDS_MISC_VALIDITY_INVALID_TREE
};

/***** structures *****/
struct stds_misc_validation_info
{
  stds_pal_uint_t
    min_elements,
    max_elements;
};

/***** macros *****/
#define stds_misc_offsetof( structure, member )  ( (stds_pal_uint_t) &((structure *) NULL)->member )


