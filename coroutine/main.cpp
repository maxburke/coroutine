// main.cpp / 2012 Max Burke / Public Domain

#include "coroutine.h"
#include <stdio.h>

int test1(void *)
{
    for (int i = 0; i < 10; ++i)
    {
        printf("Coroutine 1: %d\n", i);
        coroutine_yield();
    }

    return 0;
}

int test2(void *)
{
    for (int i = 5; i > 0; --i)
    {
        printf("    Coroutine 2: %d\n", i);
        coroutine_yield();
    }

    return 0;
}

int main(void)
{
    coroutine *co = coroutine_create(test1, NULL);
    coroutine *co2 = coroutine_create(test2, NULL);
    coroutine_status status;

    while (coroutine_resume(co) != COROUTINE_STATUS_FINISHED
            || coroutine_resume(co2) != COROUTINE_STATUS_FINISHED)
    {
        printf(" Coroutine yielded!\n");
    }

    printf("done!\n");
    coroutine_destroy(co);

    return 0;
}
