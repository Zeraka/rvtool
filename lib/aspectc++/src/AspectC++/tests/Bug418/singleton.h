#ifndef __singleton_h__
#define __singleton_h__

// Example implementation of the singleton pattern:
// Constructor and copy constructor are private to prevent
// the uncontrolled creation of Singleton instances.
// The creation of the only allowed instance is accomplished
// using the public getInstance() method, which always returns
// the same object.

// While the implementation looks straightforward, the
// declaration of a friend class is a security hole.
// Thus the SingletonMonitor aspect can be used to look
// for violations of the singleton pattern.

class FalseFriend;

class Singleton 
{ 
 private: 
  Singleton() {}
  Singleton(const Singleton&) {}
  ~Singleton() {}
 public: 
  static Singleton& getInstance() { 
    static Singleton instance; 
    return instance; 
  }

  friend class FalseFriend;
}; 


#endif
