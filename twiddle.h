#ifndef _TWIDDLE_H_
#define _TWIDDLE_H_

#include <stdio.h>

static void
twiddle(void)
{
    static int t=-1;

    printf("%c\b", "|/-\\"[(++t)&3]);
    fflush(stdout);
}

#endif // !_TWIDDLE_H_
