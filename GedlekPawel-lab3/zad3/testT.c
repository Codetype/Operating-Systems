#include <stdlib.h>
long int fib(long n){
  if(n == 0 || n == 1) return 1;
  else return fib(n-1) + fib(n-2);
}
int main(int argc, char* argv[]){
  long int n = strtol(argv[1],'\0',10);
  fib(n);
  return 0;
}
