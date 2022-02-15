#ifndef BOVIS_GLOBALS_INCLUDED
#define BOVIS_GLOBALS_INCLUDED

extern uint8_t running;
extern uint64_t *ret_ptr;
extern uint64_t *stack_ptr;
extern uint64_t *buf_ptr;
extern uint64_t *int_ptr;
extern FILE *fd_output;
extern int func_line_start;
extern char filename[128];

extern char buffer_input[1024];
extern pthread_mutex_t mutex_buffer;
extern pthread_cond_t cond_buffer;
extern short input_requested;
extern short input_received;
extern short update_mem;


#endif
