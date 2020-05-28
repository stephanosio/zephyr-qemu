#include <stdio.h>

void main (void)
{
  printf ("Trap exception test\n");
  __builtin_arc_trap_s (0);
  printf ("First test OK.\n");
  __builtin_arc_trap_s (1);
  printf ("Second test OK.\n");
}

void __attribute__ ((interrupt("ilink")))
EV_Trap (void)
{
  printf ("IRQ Pass\n");
}
