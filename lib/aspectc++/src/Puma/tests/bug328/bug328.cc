template<template <bool> class _PoolTp, bool _Thread>
struct S { int I_am_the_primary_template_code; };

template<template <bool> class _PoolTp>
struct S<_PoolTp, true> { typedef bool T; };

template <bool> class H {};

S<H,true>::T x;  // can not be resolved
