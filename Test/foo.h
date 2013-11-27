#ifndef _FOO_H
#define _FOO_H
typedef void( *func_type )();

extern void(*get(void))(void);
extern void set(func_type func);
#endif
