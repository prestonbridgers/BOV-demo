#ifndef BOVIS_GLOBALS_INCLUDED
#define BOVIS_GLOBALS_INCLUDED

extern uint8_t running;
extern uint64_t *ret_ptr;
extern uint64_t *stack_ptr;
extern uint64_t *buf_ptr;
extern FILE *fd_output;
extern int func_line_start;
extern char filename[128];

#endif
