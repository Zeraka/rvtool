#include <iostream>

int prod(int m, int n) {
  if (m > n)
    return -1;
  int prod = m;
  do {
    m++;
    prod *= m;
  } while (m < n);
  return prod;
}
       
int prod_r(int m, int n) {
  if (m > n)
    return -1;
  if (m == n-1)
    return m * n;
  else
    return m * prod_r(++m, n);
}
       
int fac(int n) {
  int fac = n;
  while (n > 1) {
    n--;
    fac *= n;
  }
  return fac;
}
       
int fac_r(int n) {
  if (n == 0)
    return 1;
  return n * fac_r(--n);
}
       
int binom1(int m, int n) {
  if (m < n)
    return 0;
  return fac(m) / (fac(n) * fac(m-n));
}
       
int binom2(int m, int n) {
  return prod(n, m) * (m-n + 1) / fac(n);
}


int main() {
  int x = 2, y = 4;
  std::cout << "Product:    " << prod(x,y) << std::endl;
  std::cout << "Product:    " << prod_r(x,y) << std::endl;
  std::cout << "Faculty:    " << fac(x) << std::endl;
  std::cout << "Faculty:    " << fac_r(x) << std::endl;  
  std::cout << "Binom 1:    " << binom1(y,x) << std::endl;
  std::cout << "Binom 2:    " << binom2(y,x) << std::endl;
  return 0;
}
