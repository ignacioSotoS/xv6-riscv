#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  char *addr = sbrk(0); // Dirección actual del heap
  sbrk(4096); // Reservar una página
  
  addr[0] = 'Z'; // Escribir valor inicial

  // Proteger contra lectura
  if (mrdprotect(addr, 1) < 0) {
    printf("mrdprotect falló\n");
    exit(1);
  }

  addr[0] = 'A'; // Escritura aún permitida

  char c = addr[0]; // Intento de lectura debería provocar fallo
  printf("Valor leído: %c (esto NO debería imprimirse)\n", c);

  // Revertir protección
  if (munrdprotect(addr, 1) < 0) {
    printf("munrdprotect falló\n");
    exit(1);
  }

  printf("Protección revertida correctamente.\n");
  exit(0);
}