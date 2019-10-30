#ifndef DOMEASUREMENT_H
#define DOMEASUREMENT_H



typedef unsigned long long int cycle_t;
extern void *frameAddress;
extern void *stackBeforeAction; 
    
    cycle_t doMeasurement(AC::Action &action) {
        /* pseudocode:
        cycle_t start, current;    
        RDTSC(start);
        SAVESTACK(stackBeforeAction);
        action.trigger();
        RDTSC(current);
        return current - start;
        */

        cycle_t result;
        AC::Action *a = &action;
            
        __asm__ volatile (              
            "mov    %2, %%ecx\n"  // push argument (action == this) on the stack
	    "push   %%ecx\n" 
            "movl    %%esp, %0 \n"
	    "cpuid  \n"
            "rdtsc  \n"
            "mov    %%eax, %%esi\n"	   
	    "cpuid  \n"
	    "call   *%%edi \n"
	    "cpuid \n"
            "rdtsc \n"
            "sub    %%esi,%%eax\n" // current - start
	    "xorl   %%edx, %%edx\n"
            "pop    %%ecx\n"
              
            : "=m" (stackBeforeAction), "=A"(result) 
            : "m" ( a ), "D" (a->_wrapper)
            : "%ebx", "%ecx", "%esi" // add "%ecx" in case of problems
         );
        return result;
    }  
#endif
