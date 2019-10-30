class Key {
 public:
  unsigned char scan;

  struct scan {
    enum{del=0x53};
  };

  int f () {
    return Key::scan::del;
  }
};
