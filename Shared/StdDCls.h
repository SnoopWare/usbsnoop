// stddcls.h -- Precompiled headers for WDM drivers
// Produced by Walt Oney's driver wizard

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable: 4201)
// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4200)
// warning C4127: conditional expression is constant
#pragma warning(disable: 4127)
// warning C4514: 'f' : unreferenced inline function has been removed
#pragma warning(disable: 4514)


#ifdef DRIVER

#ifdef __cplusplus
extern "C" {
#endif
	
#include <wdm.h>
	
#ifdef __cplusplus
}
#endif

#endif // DRIVER

// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4200)
#include <usbdi.h>


#define PAGEDCODE code_seg("page")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("init")

#define PAGEDDATA data_seg("page")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("init")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

// Override DDK definition of ASSERT so that debugger halts in the
// affected code and halts even in the unchecked OS

#if DBG && defined(_X86_)
#undef ASSERT
#define ASSERT(e) if(!(e)){DbgPrint("Assertion failure in "\
	__FILE__ ", line %d: " #e "\n", __LINE__);\
	_asm int 1\
}
#endif


#ifndef min
// the always-missing min() and max()
static inline long min(long a, long b)
{
    return (a < b) ? a : b;
}
#endif

#ifndef max
static inline long max(long a, long b)
{
    return (a > b) ? a : b;
}
#endif

