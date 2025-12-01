# Ignacio Soto Saniter y Benjamín Urrutia - Guía de pasos para la Tarea 3

En este markdown puede encontrar todos los pasos necesarios para realizar la tarea 3. Notar que todos estos pasos se hicieron sobre un fork del repositorio oficial de XV6 - Riscv.

## Incorporación de funciones y modificación del Page Table

### 1) Primero fue necesario modificar el archivo `kernel/vm.c` para crear e incluir las funciones correspondientes para manejar el acceso de lectura:

```c

int
mrdprotect(void *addr, int len) //función que quita el permiso de lectura
{
  struct proc *p = myproc(); 
  uint64 va = (uint64)addr; //obtiene la dirección virtual conviertiendolo a un enterno sin signos

  if((va % PGSIZE != 0) || (len <= 0)) //revisa y valida que el tamaño de la dir. virtual sea coherente con el tamaño de página y que la cantidad de páginas sea mayor a 0 (para avanzar)
    return -1;

  for(int i = 0; i < len; i++){ //se recorre cada pagina del page table con ayuda de walk 
    pte_t *pte = walk(p->pagetable, va + i*PGSIZE, 0); //walk retorna el PTE del último nivel (nivel 0), que mapea la página física
    if( (pte == 0) || ((*pte & PTE_V) == 0) || ((*pte & PTE_U) == 0)) 
      return -1; //como lo pide la tarea, si no existe el PTE, no es válido/mapeable o no es de usuario (es de kernel), retorna -1

    //quita el permiso de lectura para la página
    *pte &= ~PTE_R;
  }

  //limpia el TLB
  sfence_vma();

  return 0;
}

int
munrdprotect(void *addr, int len) //función que otorga el permiso de lectura
{
  struct proc *p = myproc(); 
  uint64 va = (uint64)addr;

  if((va % PGSIZE != 0) || (len <= 0)) //realiza las mismas validaciones que la función anterior
    return -1;

  for(int i = 0; i < len; i++){ //se mantiene la estructura del ciclo for, excepto su final
    pte_t *pte = walk(p->pagetable, va + i*PGSIZE, 0);  
    if( (pte == 0) || ((*pte & PTE_V) == 0) || ((*pte & PTE_U) == 0))
      return -1; 

    //restaura el permiso de lectura para cada página
    *pte |= PTE_R; 
    
  }
  sfence_vma(); //al igual que antes, limpia el TLB
  return 0;
}
```

> Observación 1: Notar que en xv6-riscv existen varios niveles dentro del Page Table. Por lo tanto walk() es muy útil para encontrar la dirección física de una dirección virtual.

> Observación 2: `*pte &= ~PTE_R` Funciona porque `~PTE_R` es la máscara con el bit de lectura en 0 y los demás bits en 1; por lo tanto `*pte &= ~PTE_R` borra (limpia) el bit de lectura (PTE_R) en el PTE sin intervenir los otros bits (W, U, V, etc.)

> Observación 3: `*pte |= PTE_R` Funciona porque `PTE_R` representa el bit de lectura; por lo tanto `*pte |= PTE_R;` activa el bit de lectura en el PTE sin modificar los demás bits.

### 2) Luego fue necesario agregar las system calls para cada función declarando y enumerando cada una en `kernel/syscall.h` y en `kernel/syscall.c` respectivamente:

Archivo syscall.h:
```h
#define SYS_mrdprotect  22
#define SYS_munrdprotect 23
```

Archivo syscall.c
```c
//líneas que se agregan
extern uint64 sys_mrdprotect(void);
extern uint64 sys_munrdprotect(void);


static uint64 (*syscalls[])(void) = {
  ...,
  [SYS_mrdprotect]  sys_mrdprotect, //línea que se agrega
  [SYS_munrdprotect] sys_munrdprotect, //línea que se agrega
}
```
### 3) En tercer lugar, para que todo lo anterior funcione correctamente, es necesario implementar las syscall en `kernel/sysproc.c`, agregando las siguientes syscalls:

```c
uint64
sys_mrdprotect(void)
{
  uint64 addr; //dir. virtual
  int len; //cantidad de páginas

  argaddr(0, &addr); //obtiene el primer argumento (dir. virtual)
  argint(1, &len); //obtiene el segundo argumento (cantidad de páginas)

  return mrdprotect((void*)addr, len); //se opera la función, desde el syscall
}

uint64
sys_munrdprotect(void)
{
  uint64 addr; //lo mismo que antes
  int len;

  argaddr(0, &addr); //lo mismo
  argint(1, &len);

  return munrdprotect((void*)addr, len); //lo mismo que antes
}
```
### 4) En cuarto lugar, se edita y agregan las siguiente líneas en `user/user.h` para que el usuario pueda llamar a esas nuevas syscalls como funciones comunes:

```h
int mrdprotect(void *addr, int len);
int munrdprotect(void *addr, int len);
```

### 5) Luego, para que el usuario pueda usar y llamar a esas nuevas syscalls sin el prefijo "sys" se edito y agregaron las siguientes líneas en `user/usys.pl`:

```pl
entry("mrdprotect");
entry("munrdprotect");
```

### 6) Por último,dado un error que no nos habíamos percatado, fue necesario agregar las siguientes líneas en `kernel/defs.h` para declarar estas funciones como parte del kernel:
```h
//para las nuevas syscalls
int             mrdprotect(void *addr, int len);
int             munrdprotect(void *addr, int len);
```

## Implementación de programa de prueba

### A) Primero se creó el `user/rdprotect_test.c` que contiene el siguiente contenido:

```c
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
```

### B) Luego se debe agregar el `rdprotect_test` a los programas del usuario para que así sea ejecutable. Para ello se debe modificar el `Makefile`, específicamente en UPROGS:

```Makefile
$U/_rdprotect_test\
```

## Captura de pantalla que demuestra la correcta implementación de las funciones con `rdprotect_test`:

![Imagen Demostrativa](image.png)

## Observaciones:

Lo que se ve en la captura anterior es esperable, ya que el error de:

```
usertrap(): unexpected scause 0xf pid=4
            sepc=0x30 stval=0x4000
```

significa que la CPU generó una excepción de página (load access fault) cuando se intentó leer la página que tiene el bit de lectura eliminado. Es decir, `mrdprotect` está funcionando, pues se bloquea la lectura y cuando el proceso intenta `char c = addr[0];`, el kernel detecta un fallo de acceso. Se entiende que no es un bug del código, sino que es la señal de que la protección de lectura se aplicó correctamente. 



