#ifndef ARGUMENTSIZECALCULATOR_H
#define ARGUMENTSIZECALCULATOR_H

/*
 * Template Metaprogram to calculate the size of the arguments
 * of a given JoinPoint on the stack
 * 
 * uses the JoinPoint API
 */

template <typename T>
struct TypedSizeCalculator {
    enum { RESULT = sizeof(T) };
};

template <typename T>
struct TypedSizeCalculator<T&> {
    enum  { RESULT = sizeof(T*) };
};

template <typename T> 
class ArgumentSizeCalculator {
 // Workaround for http://aspectc.org/bugzilla/show_bug.cgi?id=236
#ifndef __puma
      private:
      template <typename TJP, int i> struct Sum_N {
        enum { RESULT = TypedSizeCalculator<typename TJP::template Arg<i-1>::Type>::RESULT + Sum_N<TJP, i-1>::RESULT };
      };
            
      template <typename TJP> struct Sum_N <TJP, 0> {
        enum { RESULT = 0};
      };
   
      public:
        enum { RES = Sum_N<T, T::ARGS>::RESULT };
#else 
     enum { RES = 42 };
#endif        
};

#endif
