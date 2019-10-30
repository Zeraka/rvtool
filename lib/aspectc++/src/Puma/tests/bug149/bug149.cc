template <class T>
class sc_signal {
    virtual void register_port(const char* );
    virtual void write( const T& );
};
template <class T>
inline void sc_signal<T>::register_port(const char* if_typename_ ) {}
template <class T>
inline void sc_signal<T>::write( const T& value_ ) {}

template <>
class sc_signal<bool> {
    virtual void register_port(const char* );
    virtual void write( const bool& );
};
inline void sc_signal<bool>::register_port(const char* if_typename_ ) {}
inline void sc_signal<bool>::write( const bool& value_ ) {}

template <>
class sc_signal<int> {
    virtual void register_port(const char* );
    virtual void write( const int& );
};
inline void sc_signal<int>::register_port(const char* if_typename_ ) {}
inline void sc_signal<int>::write( const int& value_ ) {}
