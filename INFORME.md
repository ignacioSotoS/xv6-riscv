# Ignacio Soto y Benjamín Urrutia - Guía de pasos realizar la Tarea 1.

En este markdown puede encontrar todos los pasos necesarios para realizar la Tarea 1.

## Instrucciones seguidas para realizar ambas partes de la tarea.

### 1) Para completar la parte I, se tiene que ir a la carpeta del kernel y editar el archivo de sysproc.c, agregando estas líneas:

```c
uint64
sys_getppid(void)
//Se crea un nuevo syscall para obtener el ID del proceso padre
{
  struct proc *curr_proc = myproc(); //Puntero al proceso actual
  if(curr_proc->parent == (void*)0){ //Este void*0 es NULL
    //Si el proceso actual no tiene padre, se devuelve -1
    return -1;
  }
  return curr_proc->parent->pid; //Retorna el ID del proceso padre
}
```

### 2) Para completar la parte II, se tiene que ir a la carpeta del kernel y editar el archivo de sysproc.c, agregando estas líneas:

```c
uint64
sys_getancestror(void)
/*
Se crea un nuevo syscall para obtener el ID de un ancestro en una generación determinada. Si la generación no existe, devuelve -1
Notar que Generación 0 es el proceso actual, generación 1 es el proceso padre, generación 2 es el proceso abuelo y así sucesivamente.
*/
{
  int num_gen; //Variable para obtener el número de generaciones desde el argumento de la función
  argint(0,&num_gen); //Obtiene el número de generaciones desde el argumento de la función
  int cont_gen = 0;
  struct proc *curr_proc = myproc(); //Puntero que recorre la lista de procesos ancestros
  while(curr_proc->parent != (void*)0 && cont_gen < num_gen){ //Lo recorre hasta llegar a la generación deseada o al proceso raíz
    cont_gen++;
    curr_proc = curr_proc->parent; //Avanzar al proceso padre de cada proceso
  }
  if(cont_gen == num_gen){ //Si se llega a la generación deseada, devuelve el ID de ese proceso ancestro
    return curr_proc->pid;
  }
  return -1; // Si no, devuelve -1 (se llegó al proceso raíz)
}
```

### 3) Para que ambas funciones logren realizar su finalidad, se agrega se debe agregar su nro de identificación de systemcall para que el procesador las pueda identificar. Para esto hay que dirigirse a la carpeta de kernel y luego a syscall.h agregando las siguiente líneas al final del archivo:

```h
#define SYS_getppid 22        //Número de syscall para getppid
#define SYS_getancestror 23   //Número de syscall para getancestror
```

### 4) Para que pueda llamar ambas funciones que se definieron en sysproc.c, primero se debe editar el archivo syscall.c de la carpeta kernel (luego de la línea 103):

```c
extern uint64 sys_getppid(void);         //Línea agregada para getppid
extern uint64 sys_getancestror(void);    //Línea agregada para getancestror
```

### 5) Lo 2do que se requiere para que pueda llamar ambas funciones que se definieron en sysproc.c, se debe editar el archivo syscall.c de la carpeta kernel de tal manera de que las funciones se agreguen al array de punteros de funciones (donde el índice es el identificador que se puso en syscall.h)

```c
[SYS_getppid] sys_getppid,         //Línea agregada para getppid
[SYS_getancestror] sys_getancestror, //Línea agregada para getancestror
```

### 6) Luego, en la carpeta de user, se debe editar el archivo user.h para "definir" la llamada a la función del kernel. Para esto se deben agregar las siguientes líneas:

```c
int getppid(void);
int getancestror(int);
```

### 7) Después, en la carpeta de user, se debe editar el archivo de usys.pl para que quitarle el prefijo "sys" a la función y que se llame como en el archivo user.h (ambas funciones definidas en el paso anterior)

```pl
entry("getppid");         #Línea agregada para getppid
entry("getancestror");    #Línea agregada para getancestror
```

## Instrucciones seguidas para realizar pruebas de la tarea.

### 1) Primero, para probar la función de getppid se creó el archivo "yosoytupadre.c" en la carpeta user:

```c
#include "kernel/types.h"
#include "user.h"

int main() {
  int pid = fork();
  if (pid < 0) {
    printf("Fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Proceso hijo
    printf("Hola, soy el proceso: PID = %d, mi padre tiene el: ID = %d\n", getpid(), getppid());
    exit(0);
  } else {
    // Proceso padre
    wait(0); // Esperar a que el hijo termine
    printf("Hola, soy el proceso: PID = %d, mi hijo tiene el: ID = %d\n", getpid(), pid);
  }
  exit(0);
}
```

### 2) Luego, para hacer la prueba sobre getancestror se creó el archivo "test_ancestror.c" en la carpeta user:

```c
#include "kernel/types.h"
#include "user.h"

int main() {
  printf("Hola, soy el proceso : %d\n", getancestror(0));
  printf("Mi padre es : %d\n", getancestror(1));
  printf("Mi abuelo es : %d\n", getancestror(2));
  printf("Mi bisabuelo es : %d\n", getancestror(3)); // Esto debería devolver -1 ya que el bisabuelo no existe, el kernel tiene 2 procesos iniciales.
  exit(0);
}
```

### 3) Luego para compilar ambas pruebas se deben agregar en el Makefile en la parte de UPROGS:

```Makefile
	$U/_yosoytupadre\
	$U/_test_ancestror\
```

### 4) Como se puede observar en la siguiente imagen, las pruebas funcionan correctamente:

![Prueba exitosa](image.png)

## Dificultades encontradas:

La principal dificultad al implementar las syscall fue comprender la arquitectura de xv6. Esto requirió tiempo de lectura y análisis del código del kernel para entender el rol de cada archivo. En particular, fue necesario identificar dónde se almacenaba el PCB de cada proceso, lo cual se encontró en el archivo proc.h, definido en la estructura proc. Una vez comprendido esto, resultó más sencillo entender que las syscall solicitadas consistían básicamente en recorrer una lista enlazada simple, ya que cada proceso posee un puntero hacia otro struct proc que representa a su proceso padre.

La implementación de las syscall en sí no presentó mayor complejidad, dado que en el archivo syscall.h se observa claramente que estas se identifican con un número entero, el cual se almacena en un registro de RISC-V. Luego, al invocar la interrupción de syscall, se accede al arreglo de punteros a funciones para ejecutar la llamada correspondiente, al igual que ocurre en sistemas operativos modernos como Linux.

Una dificultad más relevante surgió al intentar llamar a las syscall desde la carpeta user. No bastaba con definirlas en user.h; también era necesario escribir la función sin el prefijo sys dentro de usys.pl. Este archivo se encarga de mapear las funciones del kernel (con prefijo sys) y exponerlas al usuario sin él, lo que inicialmente resultó confuso.

Finalmente, una dificultad menor apareció al notar que, al no contar con librerías estándar de C, no estaban disponibles macros habituales como NULL. Esto generó problemas al recorrer la lista de procesos. Fue necesario investigar su definición y se descubrió que NULL corresponde a (void*)0.

## Funcionamiento del Syscall:

En xv6, las syscall funcionan a través de una tabla (un arreglo de punteros a funciones) que asocia cada llamada con un número identificador. Cuando un proceso en user mode necesita invocar una syscall, carga en un registro de RISC-V el número correspondiente. Posteriormente, se realiza la transición a kernel mode, donde el sistema lee dicho registro, localiza en la tabla la dirección de memoria de la función asociada, la ejecuta y, finalmente, retorna el control al user mode. Este mecanismo es similar al utilizado en los sistemas Unix, en los que xv6 está inspirado.