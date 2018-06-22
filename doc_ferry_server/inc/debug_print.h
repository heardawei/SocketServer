#ifndef DEBUG_PRINT_HHH
#define DEBUG_PRINT_HHH

#define DEBUG

#ifdef DEBUG

// #define DEBUG_PRINT(fmt, args...)\
//     do {\
//         fprintf(stderr, "DEBUG :%s:%d:%s(): " fmt,\
//                 __FILE__, __LINE__, __func__, ##args);\
//     } while(0)

#define debug_print(...) do{ fprintf(stderr, __VA_ARGS__); }while(0)

#else /* DEBUG */

// #define DEBUG_PRINT(fmt, args...) do{ }while(0)

#define debug_print(...) do{ }while(0)

#endif /* DEBUG */


#endif /* DEBUG_PRINT_HHH */