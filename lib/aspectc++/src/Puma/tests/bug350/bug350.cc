void print(char*,...) {
}

#define MSG msg
#define MSG1 msg1

#define MACRO1(x, arg...) \
  print(x ## arg)

#define MACRO2(x, arg...) \
  print(x, ## arg)

int main() {
  char *msg, *msg1;

  MACRO1(MSG);       // => print(msg)
  MACRO1(MSG,);      // => print(msg)
  MACRO1(,MSG);      // => print(msg)
  MACRO1(MSG,1);     // => print(msg1)

  MACRO2(MSG);       // => print(msg)
  MACRO2(MSG,);      // => print(msg,)
  MACRO2(MSG,1);     // => print(msg,1)
  MACRO2(MSG,1,2);   // => print(msg,1,2)
  
  return 0;
}
