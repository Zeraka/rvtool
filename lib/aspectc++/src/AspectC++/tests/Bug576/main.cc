#include <stdio.h>

#define CYG_MACRO_START do {
#define CYG_MACRO_END   } while (0)

#define CYGNUM_HAL_ISR_MIN                       32
#define CYGNUM_HAL_ISR_MAX                       255
#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

typedef int cyg_uint32;
typedef cyg_uint32   cyg_vector_t;
typedef cyg_vector_t cyg_vector;
typedef unsigned int* CYG_ADDRESS;
typedef unsigned int CYG_ADDRWORD;

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
volatile CYG_ADDRESS  hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRWORD hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRESS  hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
//volatile CYG_ADDRESS  hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;


//---------------------------------------------------------------------------
// Default ISR

cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data) { return 42; }

typedef cyg_uint32 cyg_ISR(cyg_vector vector, CYG_ADDRWORD data);


#define HAL_DEFAULT_ISR hal_default_isr


#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
        ((_index_) = ((_vector_)-CYGNUM_HAL_ISR_MIN))

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)        \
    CYG_MACRO_START                                     \
    cyg_uint32 _index_;                                 \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);         \
                                                        \
    if (hal_interrupt_handlers[_index_]                 \
        ==(CYG_ADDRESS)HAL_DEFAULT_ISR)                 \
        (_state_) = 0;                                  \
    else                                                \
        (_state_) = 1;                                  \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )               \
    CYG_MACRO_START                                                             \
    cyg_uint32 _index_;                                                         \
    HAL_TRANSLATE_VECTOR((_vector_), _index_);                                  \
                                                                                \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR )       \
    {                                                                           \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)(_isr_);                 \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD)(_data_);                   \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)(_object_);               \
    }                                                                           \
    CYG_MACRO_END

class Cyg_Interrupt
{
    cyg_vector          vector;         // Interrupt vector

    cyg_ISR             *isr;           // Pointer to ISR

    CYG_ADDRWORD        data;           // Data pointer

public:
    void attach(void);
    Cyg_Interrupt() : vector(CYGNUM_HAL_ISR_MIN + 1), isr(0), data(0) {}
};


void
Cyg_Interrupt::attach(void)
{
        int in_use;


        HAL_INTERRUPT_IN_USE( vector, in_use );
        //CYG_ASSERT( 0 == in_use, "Interrupt vector not free.");

        HAL_INTERRUPT_ATTACH( vector, isr, data, this );
}

int main() {
  Cyg_Interrupt i;
  i.attach();
  return 0;
}

aspect Tracer {
  advice get("% ...::%") || set("% ...::%") : before() {
    printf("%s\n", JoinPoint::signature());
  }
};

