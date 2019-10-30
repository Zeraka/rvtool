struct Stream {
  template <typename T> Stream & operator << (T o) {
    return *this;
  }
};

int main () {
  Stream out;
  out << "Text";
}
