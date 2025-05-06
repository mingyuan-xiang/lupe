#ifndef INCLUDE_MSPBASE_MACRO_H
#define INCLUDE_MSPBASE_MACRO_H

#define __STICH(A, B) A##B
#define STICH(A, B) __STICH(A, B)
#define __STIC3(A, B, C) A##B##C
#define STIC3(A, B, C) __STIC3(A, B, C)

#define clear(reg, value) reg &= ~value

#endif /* INCLUDE_MSPBASE_MACRO_H */