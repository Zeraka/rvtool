#ifndef __classes_h__
#define __classes_h__

class A {
public:
  friend void declared_friend_1(const A&);
  friend void declared_friend_2(const A&);
  friend inline void defined_friend_1(const A&) {}
  friend void defined_friend_2(const A&);
};

void declared_friend_1(const A&);
inline void defined_friend_2(const A&) {}

namespace N {
  class B {
  public:
    friend void declared_friend(const B&);
    friend inline void defined_friend_1(const B&) {}
    friend void defined_friend_2(const B&);
  };

  inline void defined_friend_2(const B&) {}
}

namespace QT {
  class QString {
    friend bool operator==(const QString&, const QString&);
  };
}

namespace Peer {
  class TorrentPeer {
  public:
    QT::QString id;

    inline bool operator==(const TorrentPeer &other) {
      return id == other.id;
    }
    inline bool foo();
  };


  bool TorrentPeer::foo () {
    return id == id;
  }
}

#endif // __classes_h__
