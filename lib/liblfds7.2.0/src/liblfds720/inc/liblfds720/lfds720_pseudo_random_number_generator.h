/***** defines *****/
#define LFDS720_PRNG_MAX  ( (lfds720_pal_uint_t) -1 )

#if( LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 4 )
  #define LFDS720_PRNG_SEED                            0x0a34655dUL
  #define LFDS720_PRNG_SPLITMIX_MAGIC_RATIO            0x9E3779B9UL
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_ONE     16
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_TWO     13
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_THREE   16
  #define LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_ONE  0x85ebca6bUL
  #define LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_TWO  0xc2b2ae35UL
#endif

#if( LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 8 )
  #define LFDS720_PRNG_SEED                            0x0a34655d34c092feULL
  #define LFDS720_PRNG_SPLITMIX_MAGIC_RATIO            0x9E3779B97F4A7C15ULL
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_ONE     30
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_TWO     27
  #define LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_THREE   31
  #define LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_ONE  0xBF58476D1CE4E5B9ULL
  #define LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_TWO  0x94D049BB133111EBULL
#endif

/***** structs *****/
struct lfds720_prng_state
{
  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    entropy;
};

struct lfds720_prng_st_state
{
  lfds720_pal_uint_t
    entropy;
};

/***** public macros and prototypes *****/
void lfds720_prng_init_valid_on_current_logical_core( struct lfds720_prng_state *ps, lfds720_pal_uint_t seed );
void lfds720_prng_st_init( struct lfds720_prng_st_state *psts, lfds720_pal_uint_t seed );

// TRD : struct lfds720_prng_state prng_state, lfds720_pal_uint_t random_value
#define LFDS720_PRNG_GENERATE( prng_state, random_value )                                                                 \
{                                                                                                                         \
  LFDS720_PAL_ATOMIC_ADD( (prng_state).entropy, LFDS720_PRNG_SPLITMIX_MAGIC_RATIO, (random_value), lfds720_pal_uint_t );  \
  LFDS720_PRNG_ST_MIXING_FUNCTION( random_value );                                                                        \
}

// TRD : struct lfds720_prng_state prng_st_state, lfds720_pal_uint_t random_value
#define LFDS720_PRNG_ST_GENERATE( prng_st_state, random_value )                       \
{                                                                                     \
  (random_value) = ( (prng_st_state).entropy += LFDS720_PRNG_SPLITMIX_MAGIC_RATIO );  \
  LFDS720_PRNG_ST_MIXING_FUNCTION( random_value );                                    \
}

// TRD : lfds720_pal_uint_t random_value
#define LFDS720_PRNG_ST_MIXING_FUNCTION( random_value )                                                                                            \
{                                                                                                                                                  \
  (random_value) = ((random_value) ^ ((random_value) >> LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_ONE)) * LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_ONE;  \
  (random_value) = ((random_value) ^ ((random_value) >> LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_TWO)) * LFDS720_PRNG_SPLITMIX_MULTIPLY_CONSTANT_TWO;  \
  (random_value) = (random_value ^ (random_value >> LFDS720_PRNG_SPLITMIX_SHIFT_CONSTANT_THREE));                                                  \
}

/***** notes *****/

/* TRD : the seed is from an on-line hardware RNG, using atmospheric noise
         the URL below will generate another 16 random hex digits (e.g. a 64-bit number) and is
         the RNG used to generate the number above (0x0a34655d34c092fe)

         http://www.random.org/integers/?num=16&min=0&max=15&col=1&base=16&format=plain&rnd=new

         the 32 bit seed is the upper half of the 64 bit seed

         the "SplitMix" PRNG is from from Sebastiano vigna's site, CC0 license, http://xorshift.di.unimi.it/splitmix64.c
         the 64-bit constants come directly from the source, the 32-bt constants are in fact the 32-bit murmurhash3 constants
*/


