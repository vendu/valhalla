#ifndef __V0_VM_TYPES_H__
#define __V0_VM_TYPES_H__

#include <v0/vm/conf.h>
#include <stdint.h>

typedef int32_t    v0reg_t;     	// signed register value
typedef uint32_t   v0ureg_t;    	// unsigned register value
typedef int64_t    v0wide_t;    	// signed wide-register value
typedef uint64_t   v0uwide_t;   	// unsigned wide-register value
typedef v0ureg     v0adr_t;     	// machine memory address
typedef char      *v0ptr8_t;    	// 8-bit pointer
typedef uint8_t   *v0ptru8_t;   	// 8-bit unsigned pointer
typedef void      *v0ptr_t;     	// generic pointer
typedef intptr_t   v0memofs_t;  	// signed pointer value
typedef uintptr_t  v0memadr_t;          // unsigned pointer value

typedef uint32_t   v0trapdesc_t;        // interrupt/exception/abort/fault spec

typedef uint32_t   v0pagedesc_t;        // [virtual] memory page descriptor

#endif /* __V0_VM_TYPES_H__ */

