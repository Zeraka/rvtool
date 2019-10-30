void foo () {
  static test = 0;  // implicit int
  if (!test) {
    test = 1;
  }
}
