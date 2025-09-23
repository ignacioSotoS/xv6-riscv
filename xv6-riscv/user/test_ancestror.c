#include "kernel/types.h"
#include "user.h"

int main()
{
  printf("Hola, soy el proceso : %d\n", getancestror(0));
  printf("Mi padre es : %d\n", getancestror(1));
  printf("Mi abuelo es : %d\n", getancestror(2));
  printf("Mi bisabuelo es : %d\n", getancestror(3)); // Esto debería devolver -1 ya que el bisabuelo no existe, el kernel tiene 2 procesos iniciales.
  exit(0);
}
