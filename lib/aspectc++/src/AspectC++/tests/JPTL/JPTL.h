// This file is part of the JoinPoint Template Library (JPTL)
//
// JPTL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// JPTL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with JPTL.  If not, see <http://www.gnu.org/licenses/>.
// 
// Copyright (C) 2012-2014 Christoph Borchert

#ifndef __JPTL_H__
#define __JPTL_H__

namespace JPTL {


namespace SFINAE_CHECK {

namespace EXEC {
// static check, whether <T> has an attribute 'EXEC'
template<typename T> long ttest(...); // overload resolution matches always
template<typename T> char ttest(typename T::EXEC const volatile *); // preferred by overload resolution

template<typename T, unsigned hasEXEC = ((sizeof(ttest<T>(0))==sizeof(char))?1:0) >
struct GET {
  typedef typename T::EXEC Type;
};
template<typename T>
struct GET<T, 0> {
  typedef void Type;
};
} // namespace EXEC

// For looking up EXEC, use this dummy ones if the real one has no members/functions
// FIXME: this needs maintenance if future ac++ versions change this structures
template <int __D=0> struct DummyMember { typedef char Type; typedef AC::Referred<Type>::type ReferredType; static const AC::Protection prot = AC::PROT_PRIVATE; static const AC::Specifiers spec = AC::SPEC_NONE; static ReferredType *pointer (const void* const obj = 0) { return 0; } static const char *name () { return "null"; } };
template <int __D=0> struct DummyFunction { static const AC::Protection prot = AC::PROT_PRIVATE; static const AC::Specifiers spec = AC::SPEC_NONE; };
template <int __D=0> struct DummyConstructor { static const AC::Protection prot = AC::PROT_PRIVATE; static const AC::Specifiers spec = AC::SPEC_NONE; };
template <int __D=0> struct DummyDestructor { static const AC::Protection prot = AC::PROT_PRIVATE; static const AC::Specifiers spec = AC::SPEC_NONE; };

template<typename TypeInfo, unsigned MEMBERS=TypeInfo::MEMBERS>
struct GetFirstMember {
  typedef typename TypeInfo::template Member<0> MemberInfo;
};
template<typename TypeInfo>
struct GetFirstMember<TypeInfo, 0> {
  typedef DummyMember<> MemberInfo;
};

template<typename TypeInfo, unsigned FUNCTIONS=TypeInfo::FUNCTIONS>
struct GetFirstFunction {
  typedef typename TypeInfo::template Function<0> FunctionInfo;
};
template<typename TypeInfo>
struct GetFirstFunction<TypeInfo, 0> {
  typedef DummyFunction<> FunctionInfo;
};

template<typename TypeInfo, unsigned CONSTRUCTORS=TypeInfo::CONSTRUCTORS>
struct GetFirstConstructor {
  typedef typename TypeInfo::template Constructor<0> ConstructorInfo;
};
template<typename TypeInfo>
struct GetFirstConstructor<TypeInfo, 0> {
  typedef DummyConstructor<> ConstructorInfo;
};

template<typename TypeInfo, unsigned DESTRUCTORS=TypeInfo::DESTRUCTORS>
struct GetFirstDestructor {
  typedef typename TypeInfo::template Destructor<0> DestructorInfo;
};
template<typename TypeInfo>
struct GetFirstDestructor<TypeInfo, 0> {
  typedef DummyDestructor<> DestructorInfo;
};

} // namespace SFINAE_CHECK

//-------------------------------------------------------------------------------------------------------------------------------

#ifndef __acweaving

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<TypeInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::BASECLASSES>
struct BaseIterator {

  //base type information
  typedef typename TypeInfo::template BaseClass<I-1>::Type BASE_TYPE;
  typedef AC::TypeInfo<BASE_TYPE> BASE_TYPE_INFO;

  // try to cast (a pointer) from TypeInfo::That* to BASE_TYPE*
  // if that fails, return the original value
  template<typename T>
  __attribute__((always_inline)) inline static T base_cast(T arg) { return arg; }
  __attribute__((always_inline)) inline static BASE_TYPE* base_cast(typename TypeInfo::That* arg) {
    return (BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const BASE_TYPE* base_cast(const typename TypeInfo::That* arg) {
    return (const BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static volatile BASE_TYPE* base_cast(volatile typename TypeInfo::That* arg) {
    return (volatile BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const volatile BASE_TYPE* base_cast(const volatile typename TypeInfo::That* arg) {
    return (const volatile BASE_TYPE*)arg;
  }

  // calculation of the context type
  typedef typename BaseIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::EXEC PREV_EXEC; // recursion to the current base class
  typedef typename BaseIterator<TypeInfo, Action, PREV_EXEC, I-1>::EXEC EXEC; // process next base class

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() { // rule: I > 0
    BaseIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec();  // recursion to the current base class
    BaseIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(); // process next base class
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) { // rule: I > 0
    BaseIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) );  // recursion to the current base class
    BaseIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0); // process next base class
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) { // rule: I > 0
    BaseIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) );  // recursion to the current base class
    BaseIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1); // process next base class
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) { // rule: I > 0
    BaseIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) , base_cast(arg2) );  // recursion to the current base class
    BaseIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1, arg2); // process next base class
  }
};

// Specialization for I=0
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct BaseIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // calculation of the context type
  typedef typename SFINAE_CHECK::EXEC::GET< Action<TypeInfo, LAST_EXEC> >::Type EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    Action<TypeInfo, LAST_EXEC>::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    Action<TypeInfo, LAST_EXEC>::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    Action<TypeInfo, LAST_EXEC>::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    Action<TypeInfo, LAST_EXEC>::exec(arg0, arg1, arg2);
  }
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstMember<TypeInfo>::MemberInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::MEMBERS>
struct MemberIterator {

  //member type info
  typedef typename TypeInfo::template Member<I-1> MEMBER_TYPE_INFO;

  // calculation of the context type
  typedef typename MemberIterator<TypeInfo, Action, LAST_EXEC, I-1>::EXEC PREV_EXEC;
  typedef typename SFINAE_CHECK::EXEC::GET< Action<MEMBER_TYPE_INFO, PREV_EXEC> >::Type EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    MemberIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec();
    Action<MEMBER_TYPE_INFO, PREV_EXEC>::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    MemberIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0);
    Action<MEMBER_TYPE_INFO, PREV_EXEC>::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    MemberIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1);
    Action<MEMBER_TYPE_INFO, PREV_EXEC>::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    MemberIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1, arg2);
    Action<MEMBER_TYPE_INFO, PREV_EXEC>::exec(arg0, arg1, arg2);
  }
};

// Specialization for I=0 (end of recursion)
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct MemberIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // calculation of the context type
  typedef LAST_EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstFunction<TypeInfo>::FunctionInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::FUNCTIONS>
struct FunctionIterator {

  //member type info
  typedef typename TypeInfo::template Function<I-1> FUNCTION_INFO;

  // calculation of the context type
  typedef typename FunctionIterator<TypeInfo, Action, LAST_EXEC, I-1>::EXEC PREV_EXEC;
  typedef typename SFINAE_CHECK::EXEC::GET< Action<FUNCTION_INFO, PREV_EXEC> >::Type EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    FunctionIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec();
    Action<FUNCTION_INFO, PREV_EXEC>::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    FunctionIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0);
    Action<FUNCTION_INFO, PREV_EXEC>::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    FunctionIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1);
    Action<FUNCTION_INFO, PREV_EXEC>::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    FunctionIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1, arg2);
    Action<FUNCTION_INFO, PREV_EXEC>::exec(arg0, arg1, arg2);
  }
};

// Specialization for I=0 (end of recursion)
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct FunctionIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // calculation of the context type
  typedef LAST_EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

//-------------------------------------------------------------------------------------------------------------------------------

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstConstructor<TypeInfo>::ConstructorInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::CONSTRUCTORS>
struct ConstructorIterator {

  //member type info
  typedef typename TypeInfo::template Constructor<I-1> CONSTRUCTOR_INFO;

  // calculation of the context type
  typedef typename ConstructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::EXEC PREV_EXEC;
  typedef typename SFINAE_CHECK::EXEC::GET< Action<CONSTRUCTOR_INFO, PREV_EXEC> >::Type EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    ConstructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec();
    Action<CONSTRUCTOR_INFO, PREV_EXEC>::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    ConstructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0);
    Action<CONSTRUCTOR_INFO, PREV_EXEC>::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    ConstructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1);
    Action<CONSTRUCTOR_INFO, PREV_EXEC>::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    ConstructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1, arg2);
    Action<CONSTRUCTOR_INFO, PREV_EXEC>::exec(arg0, arg1, arg2);
  }
};

// Specialization for I=0 (end of recursion)
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct ConstructorIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // calculation of the context type
  typedef LAST_EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

//-------------------------------------------------------------------------------------------------------------------------------

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstDestructor<TypeInfo>::DestructorInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::DESTRUCTORS>
struct DestructorIterator {

  //member type info
  typedef typename TypeInfo::template Destructor<I-1> DESTRUCTOR_INFO;

  // calculation of the context type
  typedef typename DestructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::EXEC PREV_EXEC;
  typedef typename SFINAE_CHECK::EXEC::GET< Action<DESTRUCTOR_INFO, PREV_EXEC> >::Type EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    DestructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec();
    Action<DESTRUCTOR_INFO, PREV_EXEC>::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    DestructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0);
    Action<DESTRUCTOR_INFO, PREV_EXEC>::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    DestructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1);
    Action<DESTRUCTOR_INFO, PREV_EXEC>::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    DestructorIterator<TypeInfo, Action, LAST_EXEC, I-1>::exec(arg0, arg1, arg2);
    Action<DESTRUCTOR_INFO, PREV_EXEC>::exec(arg0, arg1, arg2);
  }
};

// Specialization for I=0 (end of recursion)
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct DestructorIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // calculation of the context type
  typedef LAST_EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstMember<TypeInfo>::MemberInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::BASECLASSES>
struct BaseMemberIterator {

  //base type information
  typedef typename TypeInfo::template BaseClass<I-1>::Type BASE_TYPE;
  typedef AC::TypeInfo<BASE_TYPE> BASE_TYPE_INFO;

  // try to cast (a pointer) from TypeInfo::That* to BASE_TYPE*
  // if that fails, return the original value
  template<typename T>
  __attribute__((always_inline)) inline static T base_cast(T arg) { return arg; }
  __attribute__((always_inline)) inline static BASE_TYPE* base_cast(typename TypeInfo::That* arg) {
    return (BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const BASE_TYPE* base_cast(const typename TypeInfo::That* arg) {
    return (const BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static volatile BASE_TYPE* base_cast(volatile typename TypeInfo::That* arg) {
    return (volatile BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const volatile BASE_TYPE* base_cast(const volatile typename TypeInfo::That* arg) {
    return (const volatile BASE_TYPE*)arg;
  }

  // calculation of the context type
  typedef typename BaseMemberIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::EXEC PREV_EXEC; // recursion to the current base class
  typedef typename BaseMemberIterator<TypeInfo, Action, PREV_EXEC, I-1>::EXEC EXEC; // process next base class

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() { // rule: I > 0
    BaseMemberIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec();  // recursion to the current base class
    BaseMemberIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(); // process next base class
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) { // rule: I > 0
    BaseMemberIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) );  // recursion to the current base class
    BaseMemberIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0); // process next base class
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) { // rule: I > 0
    BaseMemberIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) );  // recursion to the current base class
    BaseMemberIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1); // process next base class
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) { // rule: I > 0
    BaseMemberIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) , base_cast(arg2) );  // recursion to the current base class
    BaseMemberIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1, arg2); // process next base class
  }
};

// Specialization for I=0
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct BaseMemberIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // use the MemberIterator for now, as we reached a child node
  typedef MemberIterator<TypeInfo, Action, LAST_EXEC> MIT;

  // calculation of the context type
  typedef typename MIT::EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    MIT::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    MIT::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    MIT::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    MIT::exec(arg0, arg1, arg2);
  }
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstFunction<TypeInfo>::FunctionInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::BASECLASSES>
struct BaseFunctionIterator {

  //base type information
  typedef typename TypeInfo::template BaseClass<I-1>::Type BASE_TYPE;
  typedef AC::TypeInfo<BASE_TYPE> BASE_TYPE_INFO;

  // try to cast (a pointer) from TypeInfo::That* to BASE_TYPE*
  // if that fails, return the original value
  template<typename T>
  __attribute__((always_inline)) inline static T base_cast(T arg) { return arg; }
  __attribute__((always_inline)) inline static BASE_TYPE* base_cast(typename TypeInfo::That* arg) {
    return (BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const BASE_TYPE* base_cast(const typename TypeInfo::That* arg) {
    return (const BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static volatile BASE_TYPE* base_cast(volatile typename TypeInfo::That* arg) {
    return (volatile BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const volatile BASE_TYPE* base_cast(const volatile typename TypeInfo::That* arg) {
    return (const volatile BASE_TYPE*)arg;
  }

  // calculation of the context type
  typedef typename BaseFunctionIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::EXEC PREV_EXEC; // recursion to the current base class
  typedef typename BaseFunctionIterator<TypeInfo, Action, PREV_EXEC, I-1>::EXEC EXEC; // process next base class

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() { // rule: I > 0
    BaseFunctionIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec();  // recursion to the current base class
    BaseFunctionIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(); // process next base class
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) { // rule: I > 0
    BaseFunctionIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) );  // recursion to the current base class
    BaseFunctionIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0); // process next base class
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) { // rule: I > 0
    BaseFunctionIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) );  // recursion to the current base class
    BaseFunctionIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1); // process next base class
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) { // rule: I > 0
    BaseFunctionIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) , base_cast(arg2) );  // recursion to the current base class
    BaseFunctionIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1, arg2); // process next base class
  }
};

// Specialization for I=0
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct BaseFunctionIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // use the FunctionIterator for now, as we reached a child node
  typedef FunctionIterator<TypeInfo, Action, LAST_EXEC> FIT;

  // calculation of the context type
  typedef typename FIT::EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    FIT::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    FIT::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    FIT::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    FIT::exec(arg0, arg1, arg2);
  }
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstConstructor<TypeInfo>::ConstructorInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::BASECLASSES>
struct BaseConstructorIterator {

  //base type information
  typedef typename TypeInfo::template BaseClass<I-1>::Type BASE_TYPE;
  typedef AC::TypeInfo<BASE_TYPE> BASE_TYPE_INFO;

  // try to cast (a pointer) from TypeInfo::That* to BASE_TYPE*
  // if that fails, return the original value
  template<typename T>
  __attribute__((always_inline)) inline static T base_cast(T arg) { return arg; }
  __attribute__((always_inline)) inline static BASE_TYPE* base_cast(typename TypeInfo::That* arg) {
    return (BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const BASE_TYPE* base_cast(const typename TypeInfo::That* arg) {
    return (const BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static volatile BASE_TYPE* base_cast(volatile typename TypeInfo::That* arg) {
    return (volatile BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const volatile BASE_TYPE* base_cast(const volatile typename TypeInfo::That* arg) {
    return (const volatile BASE_TYPE*)arg;
  }

  // calculation of the context type
  typedef typename BaseConstructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::EXEC PREV_EXEC; // recursion to the current base class
  typedef typename BaseConstructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::EXEC EXEC; // process next base class

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() { // rule: I > 0
    BaseConstructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec();  // recursion to the current base class
    BaseConstructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(); // process next base class
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) { // rule: I > 0
    BaseConstructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) );  // recursion to the current base class
    BaseConstructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0); // process next base class
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) { // rule: I > 0
    BaseConstructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) );  // recursion to the current base class
    BaseConstructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1); // process next base class
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) { // rule: I > 0
    BaseConstructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) , base_cast(arg2) );  // recursion to the current base class
    BaseConstructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1, arg2); // process next base class
  }
};

// Specialization for I=0
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct BaseConstructorIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // use the FunctionIterator for now, as we reached a child node
  typedef ConstructorIterator<TypeInfo, Action, LAST_EXEC> CIT;

  // calculation of the context type
  typedef typename CIT::EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    CIT::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    CIT::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    CIT::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    CIT::exec(arg0, arg1, arg2);
  }
};

//-------------------------------------------------------------------------------------------------------------------------------


template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::GetFirstDestructor<TypeInfo>::DestructorInfo, void> >::Type, // look for EXEC in Action<...> //TODO: improve readability of this particular feature
         unsigned I=TypeInfo::BASECLASSES>
struct BaseDestructorIterator {

  //base type information
  typedef typename TypeInfo::template BaseClass<I-1>::Type BASE_TYPE;
  typedef AC::TypeInfo<BASE_TYPE> BASE_TYPE_INFO;

  // try to cast (a pointer) from TypeInfo::That* to BASE_TYPE*
  // if that fails, return the original value
  template<typename T>
  __attribute__((always_inline)) inline static T base_cast(T arg) { return arg; }
  __attribute__((always_inline)) inline static BASE_TYPE* base_cast(typename TypeInfo::That* arg) {
    return (BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const BASE_TYPE* base_cast(const typename TypeInfo::That* arg) {
    return (const BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static volatile BASE_TYPE* base_cast(volatile typename TypeInfo::That* arg) {
    return (volatile BASE_TYPE*)arg;
  }
  __attribute__((always_inline)) inline static const volatile BASE_TYPE* base_cast(const volatile typename TypeInfo::That* arg) {
    return (const volatile BASE_TYPE*)arg;
  }

  // calculation of the context type
  typedef typename BaseDestructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::EXEC PREV_EXEC; // recursion to the current base class
  typedef typename BaseDestructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::EXEC EXEC; // process next base class

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() { // rule: I > 0
    BaseDestructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec();  // recursion to the current base class
    BaseDestructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(); // process next base class
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) { // rule: I > 0
    BaseDestructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) );  // recursion to the current base class
    BaseDestructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0); // process next base class
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) { // rule: I > 0
    BaseDestructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) );  // recursion to the current base class
    BaseDestructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1); // process next base class
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) { // rule: I > 0
    BaseDestructorIterator<BASE_TYPE_INFO, Action, LAST_EXEC>::exec( base_cast(arg0) , base_cast(arg1) , base_cast(arg2) );  // recursion to the current base class
    BaseDestructorIterator<TypeInfo, Action, PREV_EXEC, I-1>::exec(arg0, arg1, arg2); // process next base class
  }
};

// Specialization for I=0
template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC>
struct BaseDestructorIterator<TypeInfo, Action, LAST_EXEC, 0> {

  // use the FunctionIterator for now, as we reached a child node
  typedef DestructorIterator<TypeInfo, Action, LAST_EXEC> DIT;

  // calculation of the context type
  typedef typename DIT::EXEC EXEC;

  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {
    DIT::exec();
  }
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {
    DIT::exec(arg0);
  }
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {
    DIT::exec(arg0, arg1);
  }
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {
    DIT::exec(arg0, arg1, arg2);
  }
};

#else // __acweaving

// These are dummy iterators to speed up the weaving phase (no recursion).
// Thus, the real iterators get only instantiated once during the final compilation.

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct BaseIterator {
  typedef typename SFINAE_CHECK::EXEC::GET< Action<TypeInfo, LAST_EXEC> >::Type EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct MemberIterator {
  typedef typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::DummyMember<>, LAST_EXEC> >::Type EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct FunctionIterator {
  typedef typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::DummyFunction<>, LAST_EXEC> >::Type EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct ConstructorIterator {
  typedef typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::DummyConstructor<>, LAST_EXEC> >::Type EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct DestructorIterator {
  typedef typename SFINAE_CHECK::EXEC::GET< Action<typename SFINAE_CHECK::DummyDestructor<>, LAST_EXEC> >::Type EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct BaseMemberIterator {
  typedef typename MemberIterator<TypeInfo, Action, LAST_EXEC>::EXEC EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct BaseFunctionIterator {
  typedef typename FunctionIterator<TypeInfo, Action, LAST_EXEC>::EXEC EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct BaseConstructorIterator {
  typedef typename ConstructorIterator<TypeInfo, Action, LAST_EXEC>::EXEC EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};

template<typename TypeInfo,
         template <typename, typename> class Action,
         typename LAST_EXEC=void,
         unsigned I=0>
struct BaseDestructorIterator {
  typedef typename DestructorIterator<TypeInfo, Action, LAST_EXEC>::EXEC EXEC;
  // the exec(...) function
  __attribute__((always_inline)) inline static void exec() {}
  template<typename ARG_0>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0) {}
  template<typename ARG_0, typename ARG_1>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1) {}
  template<typename ARG_0, typename ARG_1, typename ARG_2>
  __attribute__((always_inline)) inline static void exec(ARG_0 arg0, ARG_1 arg1, ARG_2 arg2) {}
};
#endif // __acweaving

} // namespace JPTL

#endif // __JPTL_H__

