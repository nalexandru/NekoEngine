#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

#include <stdint.h>

static inline bool
Sys_BigEndian(void)
{
	union
	{
		uint32_t i;
		char c[4];
	} bint = { 0x01020304 };

	return (bint.c[0] == 1); 
}

static inline int16_t
Sys_SwapInt16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

static inline int32_t
Sys_SwapInt32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

static inline int64_t
Sys_SwapInt64(int64_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

static inline uint16_t
Sys_SwapUint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

static inline uint32_t
Sys_SwapUint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

static inline uint64_t
Sys_SwapUint64(uint64_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

static inline float
Sys_SwapFloat(float val)
{
	uint32_t *ptr = (uint32_t *)&val;
	*ptr = Sys_SwapUint32(*ptr);
	return val;
}

#endif /* _SYS_ENDIAN_H_ */
