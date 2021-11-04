/* Minimal stack implementation to store the directory heirarchies when traversing through directories */

#ifndef __STACK_C
#define __STACK_C

#include <stdio.h>
#include <stdint.h>

char    stack[8][256];    /* Maximum 8 levels deep and maximum 255 characters allowed for the directory name at each level */
int8_t top;


static void stack_init()
{
    memset(stack, 0, sizeof(stack));
    top = -1;
}

static void stack_push(char * str, size_t len)
{
    ++top;
    strncpy(stack[top], str, len);
}


static void stack_pop()   /* Clears the top of the stack */
{
    memset(stack[top], 0, sizeof(stack[top]));
    top--;
}


inline static int8_t stack_top()    /* Returns the index of (not the object at) the top of the stack */
{
    return top;
}


inline static char * stack_peek(int8_t i)
{
    return stack[i];
}


#endif

  
