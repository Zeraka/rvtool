// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#include "NamespaceAC.h"
#include "CFlow.h"
#include "version.h"

// ACModel library includes
#include "ACModel/Elements.h"

// C++ includes
#include <sstream>

using std::ostringstream;
using std::endl;


string NamespaceAC::def( ACConfig &config ) {
  string acdef;
  ostringstream jptype_ids;

  if( config.data_joinpoints() )
    jptype_ids
      << "GET = " << JPT_Get << ", "
      << "SET = " << JPT_Set << ", "
      << "REF = " << JPT_Ref << ", ";
  if( config.builtin_operators() )
    jptype_ids
       << "BUILTIN = " << JPT_Builtin << ", ";
  jptype_ids
    << "CALL = " << JPT_Call << ", "
    << "EXECUTION = " << JPT_Execution << ", "
    << "CONSTRUCTION = " << JPT_Construction << ", "
    << "DESTRUCTION = " << JPT_Destruction;

  acdef += 
    "\n"
    "#ifndef __ac_h_\n"
    "#define __ac_h_\n"
    "#ifdef __cplusplus\n"
    "namespace AC {\n"
    "  typedef const char* Type;\n"
    "  enum JPType { " + jptype_ids.str() + " };\n"
    "  enum Protection { PROT_NONE, PROT_PRIVATE, PROT_PROTECTED, PROT_PUBLIC };\n"
    "  enum Specifiers { SPEC_NONE = 0x0 , SPEC_STATIC = 0x1, SPEC_MUTABLE = 0x2, SPEC_VIRTUAL = 0x4 };\n"
    "  struct Action {\n"
    "    void **_args; void *_result;";
  acdef += " void *_entity;";
  if( config.data_joinpoints() )
    acdef += " void *_array; void **_indices;";
  acdef += " void *_target; void *_that; void *_fptr;\n"
    "    void (*_wrapper)(Action &);\n"
    "    inline void trigger () { _wrapper (*this); }\n"
    "  };\n"
    "  struct AnyResultBuffer {};\n"
    "  template <typename T> struct ResultBuffer : public AnyResultBuffer {\n"
    "    struct { char _array[sizeof (T)]; } _data;\n"
    "    ~ResultBuffer () { ((T&)_data).T::~T(); }\n"
    "    operator T& () const { return (T&)_data; }\n"
    "  };\n"
    "  template <typename T, typename N> struct TL {\n"
    "    typedef T type; typedef N next; enum { ARGS = next::ARGS + 1 };\n"
    "  };\n"
    "  struct TLE { enum { ARGS = 0 }; };\n"
    "  template <typename T> struct Referred { typedef T type; };\n"
    "  template <typename T> struct Referred<T &> { typedef T type; };\n"
    "  template <typename TL, int I> struct Arg {\n"
    "    typedef typename Arg<typename TL::next, I - 1>::Type Type;\n"
    "    typedef typename Referred<Type>::type ReferredType;\n"
    "  };\n"
    "  template <typename TL> struct Arg<TL, 0> {\n"
    "    typedef typename TL::type Type;\n"
    "    typedef typename Referred<Type>::type ReferredType;\n"
    "  };\n";
  if( config.builtin_operators() )
    acdef +=
    "  template <typename T, bool Qual> struct ConstQualify {};\n"
    "  template <typename T> struct ConstQualify<T, false> { typedef T type; };\n"
    "  template <typename T> struct ConstQualify<T, true> { typedef const T type; };\n"
    "  template <typename T> struct ConstQualify<const T, true> { typedef const T type; };\n"
    "  template <typename TL, unsigned int FC, int I> struct ArgCQ {\n"
    "    typedef typename ConstQualify<typename Arg<TL, I>::Type, ( FC >> I ) & 1>::type Type;\n"
    "    typedef typename Referred<Type>::type ReferredType;\n"
    "  };\n";
  acdef +=
    "  template <typename T> int ttest(...);\n"
    "  template <typename T> char ttest(typename T::__TI const volatile *);\n"
    "  template<typename T> struct HasTypeInfo {\n" 
    "    enum { RET=((sizeof(ttest<T>(0))==1)?1:0) };\n"
    "  };\n"
    "  template<typename T, int HAVE = HasTypeInfo<T>::RET> struct TypeInfo {\n"
    "    enum { AVAILABLE = 0, BASECLASSES = 0, MEMBERS = 0, FUNCTIONS = 0,\n"
    "           CONSTRUCTORS = 0, DESTRUCTORS = 0 };\n"
    "    typedef T That;\n"
    "  };\n"
    "  template<typename T> struct TypeInfo<T, 1> : T::__TI {\n"
    "    enum { AVAILABLE = 1 };\n"
    "  };\n"
    "  template<typename T> struct RT {};\n"
    "  template<typename T> RT<T> rt_deduce (const T&) { return RT<T>(); }\n"
    "  struct Cnv { template<typename T> operator RT<T>() const { return RT<T>(); }};\n"
    "  #define __AC_TYPEOF(expr) (1?AC::Cnv():AC::rt_deduce(expr))\n";

  // class(es) needed to implement CFlows
  acdef += CFlow::ac_namespace_contributions ();

#ifdef FRONTEND_CLANG
  // Clang Variant only features

  // definitions for packed Entity handling
  if( config.data_joinpoints() )
    acdef +=
    "  template <typename T, typename E, typename O> inline E T::* mptr_castT( E O::*mptr ) { return static_cast< E T::* >( mptr ); }\n"
    "  template <typename A> struct SpliceT {};\n"
    "  template <typename B, unsigned int D> struct SpliceT< B[D] > {\n"
    "    typedef B Base;\n"
    "    enum { Dim = D };\n"
    "  };\n"
    "  template <unsigned int TSize, typename TIdx, typename N> struct DIL : N {\n"
    "    typedef N Next;\n"
    "    enum { Count = Next::Count + 1 };\n"
    "    enum { Size = TSize };\n"
    "    typedef TIdx Idx;\n"
    "    Idx idx;\n"
    "    template <typename B> struct Entity { typedef typename SpliceT< typename Next::template Entity<B>::Type >::Base Type; };\n"
    "    template <typename E> struct Array { typedef typename Next::template Array<E>::Type Type[Size]; };\n"
    "    template <typename E> inline E& apply( typename Array<E>::Type &base ) { return Next::template apply<E>( base[idx] ); }\n"
    "    template <typename A> struct Append { typedef DIL<Size, Idx, typename Next::template Append<A>::NewList > NewList; };\n"
    "    template <typename ONext> inline void copyTo( DIL<Size,Idx,ONext> *target ) {\n"
    "      target->idx = idx;\n"
    "      Next::copyTo( static_cast<ONext *>( target ) );\n"
    "    };\n"
    "  };\n"
    "  struct DILE {\n"
    "    enum { Count = 0 };\n"
    "    template <typename B> struct Entity { typedef B Type; };\n"
    "    template <typename E> struct Array { typedef E Type; };\n"
    "    template <typename E> inline E& apply( typename Array<E>::Type &base ) { return base; }\n"
    "    template <typename A> struct Append { typedef A NewList; };\n"
    "    template <typename O> inline void copyTo( O * ) {};\n"
    "  };\n"
    "  template <typename L, unsigned int D> struct DIL_Op {\n"
    "    enum { Size = DIL_Op<typename L::Next, D - 1>::Size };\n"
    "    typedef typename DIL_Op<typename L::Next, D - 1>::Idx Idx;\n"
    "    inline static Idx &idx( L *di ) { return DIL_Op<typename L::Next, D - 1>::idx( di ); }\n"
    "  };\n"
    "  template <typename L> struct DIL_Op<L,0> {\n"
    "    enum  { Size = L::Size };\n"
    "    typedef typename L::Idx Idx;\n"
    "    inline static Idx &idx( L *di ) { return di->idx; }\n"
    "  };\n"
    "  template <typename TEntity> struct BI_Entity {\n"
    "    typedef TEntity Entity;\n"
    "    Entity &_ent;\n"
    "    inline BI_Entity( Entity &ent ) : _ent( ent ) {}\n"
    "    inline Entity &entity() { return _ent; }\n"
    "  };\n"
    "  template <typename TTarget, typename TEntity> struct BI_Member {\n"
    "    typedef TEntity Entity;\n"
    "    typedef TTarget Target;\n"
    "    typedef Entity Target::*Member;\n"
    "    Target &_target;\n"
    "    Member _member;\n"
    "    inline BI_Member( Target &target, Member mem ) : _target( target ), _member( mem ) {}\n"
    "    inline Entity &entity() { return _target.*_member; }\n"
    "  };\n"
    "  template <typename TTarget, typename TEntity> struct BI_Static {\n"
    "    typedef TEntity Entity;\n"
    "    typedef TTarget Target;\n"
    "    Entity &_ent;\n"
    "    Target &_target;\n"
    "    inline BI_Static( Target &target, Entity &ent ) : _ent( ent ), _target( target ) {}\n"
    "    inline Entity &entity() { return _ent; }\n"
    "  };\n"
    "  template <typename TBI, typename TDI> struct PackedEntity {\n"
    "    typedef PackedEntity<TBI, TDI> SELF;\n"
    "    typedef TBI BI;\n"
    "    typedef TDI DI;\n"
    "    BI _base;\n"
    "    DI _indices;\n"
    "    inline PackedEntity( BI base ) : _base( base ) {}\n"
    "    typedef typename BI::Entity Base;\n"
    "    typedef typename DI::template Entity<Base>::Type Entity;\n"
    "    inline Base &base() { return _base.entity(); };\n"
    "    inline Entity &entity() { return _indices.template apply<Entity>( base() ); }\n"
    "    enum { DIMS = DI::Count };\n"
    "    template <unsigned int D> struct Dim : DIL_Op<DI,D> {};\n"
    "    template <unsigned int D> inline typename Dim<D>::Idx &idx() { return Dim<D>::idx( &_indices ); }\n"
    "    template<typename NextIdx> struct Extend {\n"
    "      typedef typename SpliceT<Entity>::Base NewEnt;\n"
    "      enum { NextDim = SpliceT<Entity>::Dim };\n"
    "      typedef typename DI::template Append< DIL< NextDim, NextIdx, DILE > >::NewList NewDI;\n"
    "      typedef PackedEntity< BI, NewDI > NewPacked;\n"
    "    };\n"
    "    template<typename TIdxN> inline typename Extend<TIdxN>::NewPacked operator []( TIdxN idxN ) {\n"
    "      typedef typename Extend<TIdxN>::NewPacked __PE;\n"
    "      __PE res( _base );\n"
    "      _indices.copyTo( &res._indices );\n"
    "      __PE::template Dim<__PE::DIMS - 1>::idx( &res._indices ) = idxN;\n"
    "      return res;\n"
    "    }\n"
    "    inline SELF operator =( Entity arg ) { entity() = arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator +=( ARG arg ) { entity() += arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator -=( ARG arg ) { entity() -= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator *=( ARG arg ) { entity() *= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator /=( ARG arg ) { entity() /= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator %=( ARG arg ) { entity() %= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator |=( ARG arg ) { entity() |= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator ^=( ARG arg ) { entity() ^= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator &=( ARG arg ) { entity() &= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator <<=( ARG arg ) { entity() <<= arg; return *this; }\n"
    "    template<typename ARG> inline SELF operator >>=( ARG arg ) { entity() >>= arg; return *this; }\n"
    "    inline SELF operator ++() { ++entity(); return *this; }\n"
    "    inline SELF operator --() { --entity(); return *this; }\n"
    "  };\n"
    "  template <typename TEntity>\n"
    "    inline PackedEntity< BI_Entity<TEntity>, DILE > PEInit( TEntity &ent ) {\n"
    "      return PackedEntity< BI_Entity<TEntity>, DILE >( BI_Entity<TEntity>( ent ) );\n"
    "    }\n"
    "  template <typename TTarget, typename TEntity>\n"
    "    inline PackedEntity<BI_Member<TTarget, TEntity>, DILE> PEInit( const TTarget &target, TEntity TTarget::*member ) {\n"
    "      return PackedEntity< BI_Member<TTarget, TEntity>, DILE >( BI_Member<TTarget, TEntity>( const_cast<TTarget &>( target ), member ) );\n"
    "    }\n"
    "  template <typename TTarget, typename TEntity>\n"
    "    inline PackedEntity<BI_Static<TTarget, TEntity>, DILE> PEInit( const TTarget &target, TEntity &ent ) {\n"
    "      return PackedEntity< BI_Static<TTarget, TEntity>, DILE >( BI_Static<TTarget, TEntity>( const_cast<TTarget &>( target ), ent ) );\n"
    "    }\n";

#if CLANG_VERSION_NUMBER < VERSION_NUMBER_3_9_1
    // Clang < 3.9.1 does not support the __float128 type and we defined the __float128 type by ourself inside
    //  namespace AC during parsing to avoid parse errors (see ClangIntroducer::tunit_start(...)). The woven code
    //  may reference our imitated __float128 type with ::AC::__float128. Later, when compiling the woven
    //  code (e.g. by using g++) __float128 is natively supported and we just forward ::AC::__float128 to the
    //  global/built-in __float128:
    // TODO: Add this typedef only if the source already contains __float128 (and if we use __float128 inside
    //  woven code)!
    // acdef += "  typedef __float128 __float128;\n";
#endif

#endif // FRONTEND_CLANG
  
  acdef +=
    "}\n"
    "inline void * operator new (" + config.size_type() + ", AC::AnyResultBuffer *p) { return p; }\n"
    "inline void operator delete "
    "(void *, AC::AnyResultBuffer *) { } // for VC++\n"
    "#endif // __cplusplus\n"
    "#endif // __ac_h_\n";

  return acdef;
}
