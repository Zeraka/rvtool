// This test shows whether error messages by the internal parser refer
// to the correct line

int main() {
  bla; // this is line 5 of main.cc!
  return 0;
}
