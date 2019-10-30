#pragma once

class Mem
{
public:
  static void memset_mwords(void *dst, const unsigned long val, unsigned long nr_of_mwords); //FIXME: const triggers crash

};

inline void
Mem::memset_mwords(void *dst, unsigned long value, unsigned long n)
{
  unsigned dummy1, dummy2;
#if 0
  asm volatile ("cld					\n\t"
		"repz stosl               \n\t"
		: "=c"(dummy1), "=D"(dummy2)
		: "a"(value), "c"(n), "D"(dst)
		: "memory");
#endif
}
