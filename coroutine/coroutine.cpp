#include <malloc.h>
#include "coroutine.h"

struct coroutine *coroutine_create(coroutine_function function, void *parameter)
{
    // Co-routine objects are 4kb in size and 4kb aligned to make some address calculation
    // easy later without having to carry around pointers to the objects that manage our 
    // co-routine state.
    struct coroutine *co = (struct coroutine *)_aligned_malloc(sizeof(struct coroutine), 4096);
    co->header.return_value = 0;
    co->header.status = COROUTINE_STATUS_NEW;
    co->header.function = function;
    co->header.parameter = parameter;

    return co;
}

void coroutine_yield()
{
    struct coroutine *co;
    register unsigned int stack;
    __asm mov stack, esp

    // Aligning the co-routine object on a 4k boundary (and being 4k sized) makes it easy to
    // calculate the address of the co-routine object. By rounding the stack pointer down to 
    // the next 4kb boundary below we get the address of the coroutine object.
    co = (struct coroutine *)(stack & ~4095);
    if (!setjmp(co->header.coroutine_context))
    {
        co->header.status = COROUTINE_STATUS_YIELDED;
        longjmp(co->header.calling_context, 1);
    }

    co->header.status = COROUTINE_STATUS_EXECUTING;
}

enum coroutine_status coroutine_resume(struct coroutine *co)
{
    if (co->header.status == COROUTINE_STATUS_FINISHED
            || co->header.status == COROUTINE_STATUS_EXECUTING)
        return co->header.status;

    switch (setjmp(co->header.calling_context))
    {
        case 0:
        {
            // New co-routines require setting up our new "stack" in the 4kb block we've allocated.
            // The co-routine is called like a normal function and when it returns it will have
            // finished successfully.
            if (co->header.status == COROUTINE_STATUS_NEW)
            {
                register unsigned int newStack = (unsigned int)&co->header;
                __asm mov esp, newStack
                co->header.status = COROUTINE_STATUS_EXECUTING;
                co->header.return_value = co->header.function(co->header.parameter);
                co->header.status = COROUTINE_STATUS_FINISHED;

                // Restoring our stack pointer is done by longjmp'ing back to our calling context.
                // This saves us from having to properly save and restore the stack pointer and,
                // since we're carrying around that state anyways, we might as well use it.
                longjmp(co->header.calling_context, 1);
            }
            else
            {
                // If we're not a new invocation then we're going to resume our co-routine by
                // longjmp'ing back to where we were.
                longjmp(co->header.coroutine_context, 1);
            }
        }
        default:
            break;
    }

    return co->header.status;
}

void coroutine_destroy(struct coroutine *co)
{
    _aligned_free(co);
}

