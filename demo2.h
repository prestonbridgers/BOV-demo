#ifndef DEMO1_H_INCLUDED
#define DEMO1_H_INCLUDED

void    bad_func(char *str);
void    demo1();
int     demo_setup(void(**demo_func)(void), char *filename);

#endif
