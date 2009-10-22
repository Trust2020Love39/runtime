#ifndef __UTILS_MONO_COMPILER_H__
#define __UTILS_MONO_COMPILER_H__

/*
 * This file includes macros used in the runtime to encapsulate different
 * compiler behaviours.
 */
#include <config.h>

#ifdef HAVE_KW_THREAD
#if HAVE_TLS_MODEL_ATTR

#if defined(__PIC__) && !defined(PIC)
/*
 * Must be compiling -fPIE, for executables.  Build PIC
 * but with initial-exec.
 * http://bugs.gentoo.org/show_bug.cgi?id=165547
 */
#define PIC
#define PIC_INITIAL_EXEC
#endif

/* 
 * Define this if you want a faster libmono, which cannot be loaded dynamically as a 
 * module.
 */
//#define PIC_INITIAL_EXEC

#if defined(PIC)

#ifdef PIC_INITIAL_EXEC
#define MONO_TLS_FAST __attribute__((tls_model("initial-exec")))
#else
#if defined (__powerpc__)
/* local dynamic requires a call to __tls_get_addr to look up the
   TLS block address via the Dynamic Thread Vector. In this case Thread
   Pointer relative offsets can't be used as this modules TLS was
   allocated separately (none contiguoiusly) from the initial TLS
   block.

   For now we will disable this. */
#define MONO_TLS_FAST
#else
#define MONO_TLS_FAST __attribute__((tls_model("local-dynamic")))
#endif
#endif

#else

#define MONO_TLS_FAST __attribute__((tls_model("local-exec")))

#endif

#else
#define MONO_TLS_FAST 
#endif

#if defined(__GNUC__) && defined(__i386__)
#if defined(PIC)
#define MONO_THREAD_VAR_OFFSET(var,offset) do { int tmp; __asm ("call 1f; 1: popl %0; addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %0; movl " #var "@gotntpoff(%0), %1" : "=r" (tmp), "=r" (offset)); } while (0)
#else
#define MONO_THREAD_VAR_OFFSET(var,offset) __asm ("movl $" #var "@ntpoff, %0" : "=r" (offset))
#endif
#elif defined(__x86_64__)
#if defined(PIC)
// This only works if libmono is linked into the application
#define MONO_THREAD_VAR_OFFSET(var,offset) do { guint64 foo;  __asm ("movq " #var "@GOTTPOFF(%%rip), %0" : "=r" (foo)); offset = foo; } while (0)
#else
#define MONO_THREAD_VAR_OFFSET(var,offset) do { guint64 foo;  __asm ("movq $" #var "@TPOFF, %0" : "=r" (foo)); offset = foo; } while (0)
#endif
#elif defined(__ia64__) && !defined(__INTEL_COMPILER)
#if defined(PIC)
#define MONO_THREAD_VAR_OFFSET(var,offset) __asm ("addl %0 = @ltoff(@tprel(" #var "#)), gp ;; ld8 %0 = [%0]\n" : "=r" (offset))
#else
#define MONO_THREAD_VAR_OFFSET(var,offset) __asm ("addl %0 = @tprel(" #var "#), r0 ;;\n" : "=r" (offset))
#endif
#elif defined(__arm__) && defined(__ARM_EABI__) && !defined(PIC)
#define MONO_THREAD_VAR_OFFSET(var,offset) __asm ("	ldr	%0, 1f; b 2f; 1: .word " #var "(tpoff); 2:" : "=r" (offset))
#elif defined(__mono_ppc__) && defined(__GNUC__)
#if defined(PIC)
#ifdef PIC_INITIAL_EXEC

#if defined(__mono_ppc64__)
#define MONO_THREAD_VAR_OFFSET(var,offset) \
	do { long off; \
	__asm (	"ld	%0," #var "@got@tprel(2)\n" \
	: "=r" (off)); \
	(offset) = off; } while (0)
#else
/* must be powerpc32 */
#define MONO_THREAD_VAR_OFFSET(var,offset) \
	__asm (	"lwz	%0," #var "@got@tprel(30)\n" \
	: "=r" (offset))
#endif

#else

/* local dynamic requires a call to __tls_get_addr to look up the
   TLS block address via the Dynamic Thread Vector. In this case Thread
   Pointer relative offsets can't be used as this modules TLS was
   allocated separately (none contiguoiusly) from the initial TLS
   block.

   For now we will disable this. */
#define MONO_THREAD_VAR_OFFSET(var,offset) (offset) = -1

#endif
#else
/* Must be local-exec TLS */
#define MONO_THREAD_VAR_OFFSET(var,offset) \
	__asm (	"lis	%0," #var "@tprel@ha\n" \
		"addi	%0,%0, " #var "@tprel@l\n" \
	: "=r" (offset))
#endif
#else
#define MONO_THREAD_VAR_OFFSET(var,offset) (offset) = -1
#endif

#if defined(PIC) && !defined(PIC_INITIAL_EXEC)
/* 
 * The above definitions do not seem to work if libmono is loaded dynamically as a module.
 * See bug #78767.
 */
#undef MONO_THREAD_VAR_OFFSET
#define MONO_THREAD_VAR_OFFSET(var,offset) (offset) = -1
#endif

#else /* no HAVE_KW_THREAD */

#define MONO_THREAD_VAR_OFFSET(var,offset) (offset) = -1

#endif

/* Deal with Microsoft C compiler differences */
#ifdef _MSC_VER

#include <float.h>
#define isnan(x)	_isnan(x)
#define trunc(x)	(((x) < 0) ? ceil((x)) : floor((x)))
#define isinf(x)	(_isnan(x) ? 0 : (_fpclass(x) == _FPCLASS_NINF) ? -1 : (_fpclass(x) == _FPCLASS_PINF) ? 1 : 0)
#define isnormal(x)	_finite(x)

#define popen		_popen
#define pclose		_pclose

#include <direct.h>
#define mkdir(x)	_mkdir(x)

/* GCC specific functions aren't available */
#define __builtin_return_address(x)	NULL

#define __func__ __FUNCTION__

#endif /* _MSC_VER */

#if !defined(PLATFORM_WIN32) && !defined(PLATFORM_SOLARIS) && HAVE_VISIBILITY_HIDDEN
#define MONO_INTERNAL __attribute__ ((visibility ("hidden")))
#else
#define MONO_INTERNAL 
#endif

#if HAVE_DEPRECATED
#define MONO_DEPRECATED __attribute__ ((deprecated))
#else
#define MONO_DEPRECATED 
#endif

#endif /* __UTILS_MONO_COMPILER_H__*/

