/* TRD : the internal single-threaded library includes this file
         so it does not have to have the user repeat the work
         so this file has to be protected from double inclusion
         or the "more than one amtching layer" error kicks in
*/

#ifndef LFDS720_PAL_PROCESSOR

  /****************************************************************************/
  #if( defined _MSC_VER && defined _M_IX86 )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                     lfds720_pal_int_t;
    typedef int long unsigned                            lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                 "x86"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    8

    // TRD : 32-bit Intel varies from 32 to 128 bytes cache line length
    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        128
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  128

  #endif





  /****************************************************************************/
  #if( defined _MSC_VER && (defined _M_X64 || defined _M_AMD64) )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                                 lfds720_pal_int_t;
    typedef int long long unsigned                        lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "x86_64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    16

    // TRD : Intel bring over two cache lines at once, always, unless disabled in BIOS
    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  128

  #endif





  /****************************************************************************/
  #if( defined _MSC_VER && defined _M_IA64 )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                                 lfds720_pal_int_t;
    typedef int long long unsigned                        lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "ia64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    16

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  64

  #endif





  /****************************************************************************/
  #if( defined _MSC_VER && defined _M_ARM )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                      lfds720_pal_int_t;
    typedef int long unsigned                             lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "arm"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    8

    /* TRD : ARM is LL/SC and uses a reservation granule of 8 to 2048 bytes
             so the isolation value used here is worst-case - be sure to set
             this correctly, otherwise structures are painfully large

             the test application has an argument, "-e", which attempts to
             determine the ERG length
    */

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  2048

  #endif
  
  
  
  
  
  /****************************************************************************/
  #if( defined __GNUC__ && defined __arm__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                lfds720_pal_int_t;
    typedef int long unsigned                       lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "arm"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        8

    /* TRD : ARM32 is LL/SC and uses a reservation granule of 2 to 512 words,
             where a word is 32 bits, so 8 to 2048 bytes

             the ERG value used here is worst-case - be sure to set
             this correctly, otherwise structures are painfully large

             the test application has an argument, "-e", which attempts to
             determine the ERG length
    */

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   2048

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __aarch64__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                           lfds720_pal_int_t;
    typedef int long long unsigned                  lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "aarch64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        16

    /* TRD : ARM64 is LL/SC and uses a reservation granule of 2 to 512 words,
             where a word is 32 bits, so 8 to 2048 bytes, same as ARM32

             the ERG value used here is worst-case - be sure to set
             this correctly, otherwise structures are painfully large

             the test application has an argument, "-e", which attempts to
             determine the ERG length
    */

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   2048

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __i386__ )

    /* TRD : i386 has no atomic operations
             i486 has everything but not DWCAS
             i586 onwards has DWCAS
             however, the GCC predefines make it impossible to differentiate between many 32-bit platforms
             so here we only check for i386, which is always defined on 32-bit
             and in the compiler abstraction layer we use the __GCC_HAVE_SYNC_COMPARE_AND_SWAP_N defines
    */

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                lfds720_pal_int_t;
    typedef int long unsigned                       lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "x86"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        8

    // TRD : 32-bit Intel varies from 32 to 128 bytes cache line length
    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  128
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   128

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __x86_64__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                           lfds720_pal_int_t;
    typedef int long long unsigned                  lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "x86_64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        16

    // TRD : Intel bring over two cache lines at once, always, unless disabled in BIOS
    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   128

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __alpha__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                lfds720_pal_int_t;
    typedef int long unsigned                       lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "alpha"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        16

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   64

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __ia64__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                           lfds720_pal_int_t;
    typedef int long long unsigned                  lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "ia64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        16

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  64
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   64

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __mips__ && __mips != 64 )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                      lfds720_pal_int_t;
    typedef int long unsigned                             lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "mips"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    8

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  32

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __mips__ && __mips == 64 )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                                 lfds720_pal_int_t;
    typedef int long long unsigned                        lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "mips64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    16

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        128
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  128

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __powerpc__ && !defined __powerpc64__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                      lfds720_pal_int_t;
    typedef int long unsigned                             lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "powerpc"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    8

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        64
    // TRD : this value is not very certain
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  128

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __powerpc64__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                                 lfds720_pal_int_t;
    typedef int long long unsigned                        lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "powerpc64"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES    8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES    16

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES        128
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES  128

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __sparc__ && !defined __sparc_v9__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                       lfds720_pal_int_t;
    typedef int long unsigned                              lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                   "sparc"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES     4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES     8

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES         32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   32

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __sparc__ && defined __sparc_v9__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long long                                 lfds720_pal_int_t;
    typedef int long long unsigned                        lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING                  "sparcv9"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES     8
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES     16

    // TRD : yes, really
    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES         32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   32

  #endif





  /****************************************************************************/
  #if( defined __GNUC__ && defined __m68k__ )

    #ifdef LFDS720_PAL_PROCESSOR
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_processor.h".
    #endif

    #define LFDS720_PAL_PROCESSOR

    typedef int long                                lfds720_pal_int_t;
    typedef int long unsigned                       lfds720_pal_uint_t;

    #define LFDS720_PAL_PROCESSOR_STRING            "m68k"

    #define LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES        4
    #define LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES        8

    #define LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES  32
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES   32

  #endif





  /****************************************************************************/
  #if( !defined LFDS720_PAL_PROCESSOR )

    #error No matching porting abstraction layer in "lfds720_porting_abstraction_layer_processor.h".

  #endif

#endif

