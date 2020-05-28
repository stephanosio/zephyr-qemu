#ifndef __ARC_TIMER_H__
#define __ARC_TIMER_H__

void arc_initializeTIMER (ARCCPU *);
void arc_resetTIMER (ARCCPU *);

void aux_timer_set (struct arc_aux_reg_detail *, uint32_t, void *);
uint32_t aux_timer_get (struct arc_aux_reg_detail *, void *);

#endif
