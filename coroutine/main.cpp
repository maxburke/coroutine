// main.cpp / 2012 Max Burke / Public Domain

#include "coroutine.h"
#include <stdio.h>

int test1(void *unused)
{
    int i;
    for (i = 0; i < 10; ++i)
    {
        printf("Coroutine 1: %d\n", i);
        coroutine_yield();
    }

    return 0;
}

int test2(void *unused)
{
    int i;
    for (i = 5; i > 0; --i)
    {
        printf("    Coroutine 2: %d\n", i);
        coroutine_yield();
    }

    return 0;
}

int main(void)
{
    struct coroutine *co = coroutine_create(test1, NULL);
    struct coroutine *co2 = coroutine_create(test2, NULL);
    enum coroutine_status status;
    enum coroutine_status status2;

    for (;;)
    {
        status = coroutine_resume(co);
        status2 = coroutine_resume(co2);

        if (status != COROUTINE_STATUS_FINISHED || status2 != COROUTINE_STATUS_FINISHED)
            printf(" Coroutine yielded!\n");
        else
            break;
    }

    printf("done!\n");
    coroutine_destroy(co);

    return 0;
}
