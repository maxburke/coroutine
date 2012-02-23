#include <setjmp.h>

enum coroutine_status
{
    COROUTINE_STATUS_NEW,                           //< New co-routines have not been run yet and do not have any context data set.
    COROUTINE_STATUS_EXECUTING,                     //< Diagnostic state to prevent resuming a co-routine while it is running.
    COROUTINE_STATUS_YIELDED,                       //< Yielded co-routines are currently in-active but can be resumed.
    COROUTINE_STATUS_FINISHED                       //< Finished co-routines have returned and cannot be resumed.
};

//< Coroutine functions take a context pointer and can return an integer, not unlike most thread entry points.
typedef int (*coroutine_function)(void *);          

struct coroutine_header
{
    // The two separate contexts used are the coroutine context and the calling context. The co-routine context
    // is saved when the co-routine is yielded and restored when it is resumed. Conversely, the calling context
    // is saved when the co-routine is resumed and restored when it is yielded. This permits jumping over
    // code that is "executing" while preserving the state of the stack/continuation.
    jmp_buf calling_context;
    jmp_buf coroutine_context;
    int return_value;
    coroutine_status status;
    coroutine_function function;
    void *parameter;
};

struct coroutine
{
    // As the stack grows down from high memory addresses it is placed first in the co-routine structure.
    // If our code tramples beyond the stack boundaries at least we won't destroy the co-routine data :).
    char stack[4096 - sizeof(coroutine_header)];
    coroutine_header header;
};

coroutine *coroutine_create(coroutine_function function, void *parameter);
void coroutine_yield();
coroutine_status coroutine_resume(coroutine *co);
void coroutine_destroy(coroutine *co);

