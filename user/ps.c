#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// function to prinf the state as string and not number
char* state_to_string(int state) {
  switch (state) {
    case 0: return "UNUSED";
    case 1: return "USED";
    case 2: return "SLEEP";
    case 3: return "RUNNABLE";
    case 4: return "RUNNING";
    case 5: return "ZOMBIE";
    default: return "UNKNOWN";
  }
};

int main(void) {
  struct pstat st;

  if (getpinfo(&st) < 0) {
    printf("ps: getpinfo failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  for (int i = 0; i < st.num_processes; i++) {
    if (st.in_use[i]) {
      printf("%d\t%d\t%s\t%d\t%s\n",
             st.pid[i], st.ppid[i], state_to_string(st.state[i]), st.size[i], st.name[i]);
    }
  }
  exit(0);
}
