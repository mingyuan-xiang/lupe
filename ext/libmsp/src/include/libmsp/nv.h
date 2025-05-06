#ifndef INCLUDE_MSPBASE_NV_H
#define INCLUDE_MSPBASE_NV_H

#define __nv __attribute__((section(".persistent")))
#define __hinv __attribute__((section(".upper.persistent")))

#define __ro_nv __attribute__((section(".rodata")))
#define __ro_hinv __attribute__((section(".upper.rodata")))
#endif /* INCLUDE_MSPBASE_NV_H */