#include "kernel/types.h" //include the types first
#include "user.h"

int main() {
  for(int i = 0; i < 10; i++){
    int pid = fork();
    if (pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }else if (pid == 0) {
      settickets(50 * (i+1));
      int slices;
      int prev_slices = 0;
      while(1){
        slices = getslices();
        if(slices != prev_slices){
          printf("PID = %d, Slices = %d\n", getpid(), slices);
          prev_slices = slices;
        }
        if(slices >= 10){
          break;
        }
      }
      exit(0);
    }
  }
  for(int i = 0; i < 10; i++){
    wait(0); // Parent process waits for all children to finish
  }
  exit(0);
}