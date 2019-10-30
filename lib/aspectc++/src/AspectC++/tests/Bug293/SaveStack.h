#ifndef SAVESTACK_H
#define SAVESTACK_H

/*
 * SaveStack.h
 * 
 * Helper Templates to "simulate" call to function matched by pointcut
 * saveStack template is overloaded with up to ten parameters.
 * 
 * TODO: find a way to do this more generic. Perhaps with gcc macro functions?
 * 
 */


#define SAVESTACK(var) __asm__ volatile ( "mov %%esp, %0" : "=m" (var) : : "memory" )
#define SAVEFRAME(var) var = __builtin_frame_address(0)
#define SAVESP SAVESTACK(frameAddress)

// 64bit architectures use 128bit rdtsc register.
// we just take the lower ones from rax and forget about rdx,
// because we don't expect longer functions to measure ;)
#ifdef __x86_64__
#define RDTSC(var) __asm__ volatile ("rdtsc" : "=a" (var) : : "memory" )
#else
#define RDTSC(var) __asm__ volatile ("rdtsc" : "=A" (var) : : "memory" )
#endif

    extern void *frameAddress;
 
    //void saveStack() { SAVESP; }
    
    template <typename P1>
    void saveStack(P1 a1) { SAVESP; }
    
    template <typename P1, typename P2>
    void saveStack(P1 a1, P2 a2) { SAVESP; }

    template <typename P1, typename P2, typename P3>
    void saveStack(P1 a1, P2 a2, P3 a3) { SAVESP; }

    template <typename P1, typename P2, typename P3, typename P4>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4) { SAVESP; }
    
    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5) { SAVESP; }

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6) { SAVESP; }

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7) { SAVESP; }

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8) { SAVESP; }

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, P9 a9) { SAVESP; }
    
    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
    void saveStack(P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, P9 a9, P10 a10) { SAVESP; }
    
  
    template<class JoinPoint, unsigned i>
    struct dispatch {
      static void doit(AC::Action &action) {
         UNSUPPORTED_NUMBER_OF_ARGUMENTS_EXPAND_MEASURE_ASPECT_AH;
      }
    };
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 0> {
      static void doit(AC::Action &action) { 
          SAVESP;
      }
    };
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 1> {
        static void doit(AC::Action &action) { 
          saveStack<
             typename JoinPoint::template Arg<0>::Type
           > (
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0]
          );
        }
    };
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 2> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type
           > (
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1]);
        }
    };
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 3> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type
           >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2]);
        }
    }; 
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 4> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type
            >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3]);
        }
    }; 
        
    template<class JoinPoint>
    struct dispatch<JoinPoint, 5> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type
            >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4]);
        }
    }; 
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 6> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type,
              typename JoinPoint::template Arg<5>::Type
            >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4],
            *(typename JoinPoint::template Arg<5>::ReferredType*)action._args[5]);
        }
    }; 
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 7> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type,
              typename JoinPoint::template Arg<5>::Type,
              typename JoinPoint::template Arg<6>::Type
            >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4],
            *(typename JoinPoint::template Arg<5>::ReferredType*)action._args[5],
            *(typename JoinPoint::template Arg<6>::ReferredType*)action._args[6]);
        }
    }; 
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 8> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type,
              typename JoinPoint::template Arg<5>::Type,
              typename JoinPoint::template Arg<6>::Type,
              typename JoinPoint::template Arg<7>::Type
           >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4],
            *(typename JoinPoint::template Arg<5>::ReferredType*)action._args[5],
            *(typename JoinPoint::template Arg<6>::ReferredType*)action._args[6],
            *(typename JoinPoint::template Arg<7>::ReferredType*)action._args[7]);
        }
    }; 
    
    template<class JoinPoint>
    struct dispatch<JoinPoint, 9> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type,
              typename JoinPoint::template Arg<5>::Type,
              typename JoinPoint::template Arg<6>::Type,
              typename JoinPoint::template Arg<7>::Type,
              typename JoinPoint::template Arg<8>::Type
           >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4],
            *(typename JoinPoint::template Arg<5>::ReferredType*)action._args[5],
            *(typename JoinPoint::template Arg<6>::ReferredType*)action._args[6],
            *(typename JoinPoint::template Arg<7>::ReferredType*)action._args[7],                        
            *(typename JoinPoint::template Arg<8>::ReferredType*)action._args[8]);
        }
    };         

    template<class JoinPoint>
    struct dispatch<JoinPoint, 10> {
        static void doit(AC::Action &action) {
           saveStack<
              typename JoinPoint::template Arg<0>::Type,
              typename JoinPoint::template Arg<1>::Type,
              typename JoinPoint::template Arg<2>::Type,
              typename JoinPoint::template Arg<3>::Type,
              typename JoinPoint::template Arg<4>::Type,
              typename JoinPoint::template Arg<5>::Type,
              typename JoinPoint::template Arg<6>::Type,
              typename JoinPoint::template Arg<7>::Type,
              typename JoinPoint::template Arg<8>::Type,
              typename JoinPoint::template Arg<9>::Type
           >(
            *(typename JoinPoint::template Arg<0>::ReferredType*)action._args[0],
            *(typename JoinPoint::template Arg<1>::ReferredType*)action._args[1],
            *(typename JoinPoint::template Arg<2>::ReferredType*)action._args[2],
            *(typename JoinPoint::template Arg<3>::ReferredType*)action._args[3],            
            *(typename JoinPoint::template Arg<4>::ReferredType*)action._args[4],
            *(typename JoinPoint::template Arg<5>::ReferredType*)action._args[5],
            *(typename JoinPoint::template Arg<6>::ReferredType*)action._args[6],
            *(typename JoinPoint::template Arg<7>::ReferredType*)action._args[7],                        
            *(typename JoinPoint::template Arg<8>::ReferredType*)action._args[8],
            *(typename JoinPoint::template Arg<9>::ReferredType*)action._args[9]);
        }
    };    
      
   
#endif
