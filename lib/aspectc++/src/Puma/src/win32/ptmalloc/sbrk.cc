
#include <windows.h>
#include <errno.h>
#include <assert.h>
//#include <iostream>


// also needed by pt_malloc
extern "C" unsigned int getpagesize () {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize;
}

class VirtualHeap {
   
    enum { MINPAGES=128 };
   
	char*  nextPage; // next page to commit
	DWORD  pageSize; // size of one page
	DWORD  maxPages; // maximal allocatable pages
	DWORD  gotPages; // pages already allocated
	DWORD  freeSize; // free space to next page
	public:
	VirtualHeap () {}

    void* reserve () {
		DWORD dPages = 1;
		void* base = 0;
		base = VirtualAlloc(0,pageSize*maxPages,MEM_RESERVE,PAGE_NOACCESS);
		while( base == 0 && maxPages ) {
			maxPages -= dPages;
			dPages *= 2;
			base = VirtualAlloc(0,pageSize*maxPages,MEM_RESERVE,PAGE_NOACCESS);
		}
		return base;
	}
   
	void init () {
		gotPages = freeSize = 0;
		pageSize = getpagesize();
		// try to get 1.5 GByte virtual address space
		maxPages = (1500UL*1024UL*1024UL)/pageSize;
	   
		// reserve pages in virtual address space
		nextPage = (char*)reserve();

//		std::cout << "PageSize: " << pageSize << "  maxPages: " << maxPages << std::endl;
//		std::cout << "BaseAddress: " << (void*)nextPage << std::endl;
		assert(nextPage);
	}

private:   
	DWORD numPagesFor(size_t n) {
		DWORD num = ((DWORD)n + pageSize - 1) / pageSize;
		return num < MINPAGES ? MINPAGES : num;
	}

	DWORD alloc (DWORD n) {
		DWORD numPages = numPagesFor(n);
//		std::cout << "alloc " << (DWORD)n << "  Pages: " << numPages << std::endl;

		if( gotPages + numPages > maxPages ) {
//			std::cout << "ERROR: no more  Pages" << std::endl;
			return 0;
		}
		
		DWORD size = numPages*pageSize;
		void* tmp = VirtualAlloc(nextPage,size,MEM_COMMIT,PAGE_READWRITE);
		if( tmp == 0 ) {
//			std::cout << "ERROR: failed to get new virtual address space" << std::endl;
			return 0;			
		}

		freeSize = size;
		nextPage += size;
		gotPages += numPages;

		return size;
	}

public:
	// extend the commited area by n byte
	void* extend (DWORD n) {
//		std::cout << "extend " << n << std::endl;
		void* res = nextPage - freeSize;
		if( freeSize < n ) {
			n -= freeSize;
			freeSize = alloc(n);
		}
		if( freeSize >= n ) {
			freeSize -= n;
//			std::cout << "Success " << res << std::endl;
			return res;
		}
		return (void*)-1;
	}

	void* shrink (DWORD n) {
		assert( n < ((gotPages*pageSize)-freeSize) );

		void* res = nextPage - freeSize;
//	  	std::cout << "shrink " << n << std::endl;
		freeSize += n;
		return res;
	}
};

static int is_init = 0;
static VirtualHeap  __vHeap;

extern "C" void* sbrk (int n) {
	void* res = 0;

	if( is_init == 0 ) {
		is_init = 1;
		__vHeap.init();
	}

	if( n > 0 ) {
		res = __vHeap.extend((DWORD)n);
	}
	else {
		res = __vHeap.shrink((DWORD)(-n));
	}

	if( res == 0 )
		errno = ENOMEM;

	return res;
}
