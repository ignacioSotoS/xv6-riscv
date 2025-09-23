#include "kernel/types.h"
#include "user.h"

int main()
{
  int pid = fork();
  if (pid < 0)
  {
    printf("Fork failed\n");
    exit(1);
  }
  else if (pid == 0)
  {
    // Proceso hijo
    printf("Hola, soy el proceso: PID = %d, mi padre tiene el: ID = %d\n", getpid(), getppid());
    exit(0);
  }
  else
  {
    // Proceso padre
    wait(0); //Espera a que el hijo termine
    printf("Hola, soy el proceso: PID = %d, mi hijo tiene el: ID = %d\n", getpid(), pid);
  }
  exit(0);
}
