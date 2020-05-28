/*
 *  QEMU ARC CPU
 * Copyright (C) 2019 Free Software Foundation, Inc.

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GAS or GDB; see the file COPYING3.  If not, write to
 * the Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "translate.h"
#include "qemu/qemu-print.h"


TCGv    cpu_gp;        /*  Global Pointer                      */
TCGv    cpu_fp;        /*  Frame Pointer                       */
TCGv    cpu_sp;        /*  Stack Pointer                       */
TCGv    cpu_ilink1;    /*  Level 1 interrupt link register     */
TCGv    cpu_ilink2;    /*  Level 2 interrupt link register     */
TCGv    cpu_blink;     /*  Branch link register                */
TCGv    cpu_acclo;     /*  64-bit accumulator register: low    */
TCGv    cpu_acchi;     /*  64-bit accumulator register: high   */
TCGv    cpu_limm;      /*  Long immediate data indicator       */
TCGv    cpu_pcl;       /*  Program Counter [31:2], read-only,
                           32-bit aligned address.             */

TCGv    cpu_S1f;
TCGv    cpu_S2f;
TCGv    cpu_CSf;

TCGv    cpu_Lf;
TCGv    cpu_Zf;
TCGv    cpu_Nf;
TCGv    cpu_Cf;
TCGv    cpu_Vf;
TCGv    cpu_Uf;

TCGv    cpu_DEf;
TCGv    cpu_ESf;
TCGv    cpu_AEf;
TCGv    cpu_Hf;
TCGv    cpu_IEf;
TCGv    cpu_Ef;

TCGv    cpu_is_delay_slot_instruction;

TCGv    cpu_l1_Lf;
TCGv    cpu_l1_Zf;
TCGv    cpu_l1_Nf;
TCGv    cpu_l1_Cf;
TCGv    cpu_l1_Vf;
TCGv    cpu_l1_Uf;

TCGv    cpu_l1_DEf;
TCGv    cpu_l1_AEf;
TCGv    cpu_l1_Hf;

TCGv    cpu_l2_Lf;
TCGv    cpu_l2_Zf;
TCGv    cpu_l2_Nf;
TCGv    cpu_l2_Cf;
TCGv    cpu_l2_Vf;
TCGv    cpu_l2_Uf;

TCGv    cpu_l2_DEf;
TCGv    cpu_l2_AEf;
TCGv    cpu_l2_Hf;

TCGv    cpu_er_Lf;
TCGv    cpu_er_Zf;
TCGv    cpu_er_Nf;
TCGv    cpu_er_Cf;
TCGv    cpu_er_Vf;
TCGv    cpu_er_Uf;

TCGv    cpu_er_DEf;
TCGv    cpu_er_AEf;
TCGv    cpu_er_Hf;

TCGv    cpu_eret;
TCGv    cpu_erbta;
TCGv    cpu_ecr;
TCGv    cpu_efa;

TCGv    cpu_bta;
TCGv    cpu_bta_l1;
TCGv    cpu_bta_l2;

TCGv    cpu_pc;
TCGv    cpu_lpc;
/* replaced by AUX_REG array */
TCGv    cpu_lps;
TCGv    cpu_lpe;

TCGv    cpu_r[64];

TCGv    cpu_intvec;

TCGv    cpu_debug_LD;
TCGv    cpu_debug_SH;
TCGv    cpu_debug_BH;
TCGv    cpu_debug_UB;
TCGv    cpu_debug_ZZ;
TCGv    cpu_debug_RA;
TCGv    cpu_debug_IS;
TCGv    cpu_debug_FH;
TCGv    cpu_debug_SS;

TCGv    cpu_lock_lf_var;

/* NOTE: Pseudo register required for comparison with lp_end */
TCGv    cpu_npc;

#include "exec/gen-icount.h"
#define REG(x)  (cpu_r[x])

static inline bool use_goto_tb(DisasContext *dc, target_ulong dest)
{
    if (unlikely(dc->base.singlestep_enabled)) {
        return false;
    }
#ifndef CONFIG_USER_ONLY
    return (dc->base.tb->pc & TARGET_PAGE_MASK) == (dest & TARGET_PAGE_MASK);
#else
    return true;
#endif
}

void gen_goto_tb(DisasContext *ctx, int n, TCGv dest)
{
    tcg_gen_mov_tl(cpu_pc, dest);
    tcg_gen_andi_tl(cpu_pcl, dest, 0xfffffffc);
    if (ctx->base.singlestep_enabled) {
        gen_helper_debug(cpu_env);
    }
    tcg_gen_exit_tb(NULL, 0);
}

static void gen_gotoi_tb(DisasContext *ctx, int n, target_ulong dest)
{
    if (use_goto_tb(ctx, dest)) {
        tcg_gen_goto_tb(n);
        tcg_gen_movi_tl(cpu_pc, dest);
        tcg_gen_movi_tl(cpu_pcl, dest & 0xfffffffc);
        tcg_gen_exit_tb(ctx->base.tb, n);
    } else {
        tcg_gen_movi_tl(cpu_pc, dest);
        tcg_gen_movi_tl(cpu_pcl, dest & 0xfffffffc);
        if (ctx->base.singlestep_enabled) {
            gen_helper_debug(cpu_env);
        }
        tcg_gen_exit_tb(NULL, 0);
    }
}

void arc_translate_init(void)
{
    int i;
    static int init_not_done = 1;

    if (init_not_done == 0) {
        return;
    }
#define ARC_REG_OFFS(x) offsetof(CPUARCState, x)

#define NEW_ARC_REG(x) \
        tcg_global_mem_new_i32(cpu_env, offsetof(CPUARCState, x), #x)

    // TODO: Check
    //cpu_env = tcg_global_reg_new_ptr(TCG_AREG0, "env");

    cpu_S1f = NEW_ARC_REG(macmod.S1);
    cpu_S2f = NEW_ARC_REG(macmod.S2);
    cpu_CSf = NEW_ARC_REG(macmod.CS);

    cpu_Zf  = NEW_ARC_REG(stat.Zf);
    cpu_Lf  = NEW_ARC_REG(stat.Lf);
    cpu_Nf  = NEW_ARC_REG(stat.Nf);
    cpu_Cf  = NEW_ARC_REG(stat.Cf);
    cpu_Vf  = NEW_ARC_REG(stat.Vf);
    cpu_Uf  = NEW_ARC_REG(stat.Uf);
    cpu_DEf = NEW_ARC_REG(stat.DEf);
    cpu_ESf = NEW_ARC_REG(stat.ESf);
    cpu_AEf = NEW_ARC_REG(stat.AEf);
    cpu_Hf  = NEW_ARC_REG(stat.Hf);
    cpu_IEf = NEW_ARC_REG(stat.IEf);
    cpu_Ef  = NEW_ARC_REG(stat.Ef);

    cpu_is_delay_slot_instruction = NEW_ARC_REG(stat.is_delay_slot_instruction);

    cpu_l1_Zf = NEW_ARC_REG(stat_l1.Zf);
    cpu_l1_Lf = NEW_ARC_REG(stat_l1.Lf);
    cpu_l1_Nf = NEW_ARC_REG(stat_l1.Nf);
    cpu_l1_Cf = NEW_ARC_REG(stat_l1.Cf);
    cpu_l1_Vf = NEW_ARC_REG(stat_l1.Vf);
    cpu_l1_Uf = NEW_ARC_REG(stat_l1.Uf);
    cpu_l1_DEf = NEW_ARC_REG(stat_l1.DEf);
    cpu_l1_AEf = NEW_ARC_REG(stat_l1.AEf);
    cpu_l1_Hf = NEW_ARC_REG(stat_l1.Hf);

    cpu_er_Zf = NEW_ARC_REG(stat_er.Zf);
    cpu_er_Lf = NEW_ARC_REG(stat_er.Lf);
    cpu_er_Nf = NEW_ARC_REG(stat_er.Nf);
    cpu_er_Cf = NEW_ARC_REG(stat_er.Cf);
    cpu_er_Vf = NEW_ARC_REG(stat_er.Vf);
    cpu_er_Uf = NEW_ARC_REG(stat_er.Uf);
    cpu_er_DEf = NEW_ARC_REG(stat_er.DEf);
    cpu_er_AEf = NEW_ARC_REG(stat_er.AEf);
    cpu_er_Hf = NEW_ARC_REG(stat_er.Hf);

    cpu_eret = NEW_ARC_REG(eret);
    cpu_erbta = NEW_ARC_REG(erbta);
    cpu_ecr = NEW_ARC_REG(ecr);
    cpu_efa = NEW_ARC_REG(efa);
    cpu_bta = NEW_ARC_REG(bta);
    cpu_lps = NEW_ARC_REG(lps);
    cpu_lpe = NEW_ARC_REG(lpe);
    cpu_pc = NEW_ARC_REG(pc);
    cpu_npc = NEW_ARC_REG(npc);

    cpu_bta_l1 = NEW_ARC_REG(bta_l1);
    cpu_bta_l2 = NEW_ARC_REG(bta_l2);

    cpu_intvec = NEW_ARC_REG(intvec);

    for (i = 0; i < 64; i++) {
        char name[16];

        sprintf(name, "r[%d]", i);

        cpu_r[i] = tcg_global_mem_new_i32(cpu_env,
                                          ARC_REG_OFFS(r[i]),
                                          strdup(name));
    }

    cpu_gp     = cpu_r[26];
    cpu_fp     = cpu_r[27];
    cpu_sp     = cpu_r[28];
    cpu_ilink1 = cpu_r[29];
    cpu_ilink2 = cpu_r[30];
    cpu_blink  = cpu_r[31];
    cpu_acclo  = cpu_r[58];
    cpu_acchi  = cpu_r[59];
    cpu_lpc    = cpu_r[60];
    cpu_limm   = cpu_r[62];
    cpu_pcl    = cpu_r[63];

    cpu_debug_LD = NEW_ARC_REG(debug.LD);
    cpu_debug_SH = NEW_ARC_REG(debug.SH);
    cpu_debug_BH = NEW_ARC_REG(debug.BH);
    cpu_debug_UB = NEW_ARC_REG(debug.UB);
    cpu_debug_ZZ = NEW_ARC_REG(debug.ZZ);
    cpu_debug_RA = NEW_ARC_REG(debug.RA);
    cpu_debug_IS = NEW_ARC_REG(debug.IS);
    cpu_debug_FH = NEW_ARC_REG(debug.FH);
    cpu_debug_SS = NEW_ARC_REG(debug.SS);

    cpu_lock_lf_var = NEW_ARC_REG(lock_lf_var);

    init_not_done = 0;
}

static void arc_tr_init_disas_context(DisasContextBase *dcbase,
                                      CPUState *cs)
{
    DisasContext *dc = container_of(dcbase, DisasContext, base);

    dc->base.is_jmp = DISAS_NEXT;
    dc->mem_idx = dc->base.tb->flags & 1;
}
static void arc_tr_tb_start(DisasContextBase *dcbase, CPUState *cpu)
{
    /* place holder for now */
}

static void arc_tr_insn_start(DisasContextBase *dcbase, CPUState *cpu)
{
    DisasContext *dc = container_of(dcbase, DisasContext, base);


    tcg_gen_insn_start(dc->base.pc_next);
    dc->cpc = dc->base.pc_next;

    if (dc->base.num_insns == dc->base.max_insns &&
        (dc->base.tb->cflags & CF_LAST_IO)) {
        gen_io_start();
    }
}

static bool arc_tr_breakpoint_check(DisasContextBase *dcbase, CPUState *cpu,
                                    const CPUBreakpoint *bp)
{
    DisasContext *dc = container_of(dcbase, DisasContext, base);

    tcg_gen_movi_tl(cpu_pc, dc->cpc);
    dc->base.is_jmp = DISAS_NORETURN;
    gen_helper_debug(cpu_env);
    dc->base.pc_next += 2;
    return true;
}

extern bool enabled_interrupts;

void decode_opc(CPUARCState *env, DisasContext *ctx)
{
    ctx->env = env;

    enabled_interrupts = false;

    const struct arc_opcode *opcode = NULL;
    if (!read_and_decode_context(ctx, &opcode)) {
        ctx->base.is_jmp = arc_gen_INVALID(ctx);
        return;
    }

    ctx->base.is_jmp = arc_decode(ctx, opcode);

    TCGv npc = tcg_const_local_i32(ctx->npc);
    gen_helper_zol_verify(cpu_env, npc);
    tcg_temp_free(npc);

    enabled_interrupts = true;
}

static void arc_tr_translate_insn(DisasContextBase *dcbase, CPUState *cpu)
{
    bool in_a_delayslot_instruction = false;
    DisasContext *dc = container_of(dcbase, DisasContext, base);
    CPUARCState *env = cpu->env_ptr;

    /* TODO (issue #62): these must be removed */
    dc->zero = tcg_const_local_i32(0);
    dc->one  = tcg_const_local_i32(1);

    if(env->stat.is_delay_slot_instruction == 1) {
        in_a_delayslot_instruction = true;
    }

    dc->cpc = dc->base.pc_next;
    decode_opc(env, dc);

    dc->base.pc_next = dc->npc;
    tcg_gen_movi_tl(cpu_npc, dc->npc);

    if(in_a_delayslot_instruction == true) {
      dc->base.is_jmp = DISAS_NORETURN;

      /* Post execution delayslot logic. */
      TCGLabel *DEf_not_set_label1 = gen_new_label();
      tcg_gen_brcondi_i32(TCG_COND_NE, cpu_DEf, 1, DEf_not_set_label1);
      tcg_gen_movi_tl(cpu_DEf, 0);
      gen_goto_tb(dc, 1, cpu_bta);
      gen_set_label(DEf_not_set_label1);
      env->stat.is_delay_slot_instruction = 0;
    }

    if(dc->base.is_jmp == DISAS_NORETURN) {
        gen_gotoi_tb(dc, 0, dc->npc);
    }
    else if(dc->base.is_jmp == DISAS_NEXT) {
        target_ulong page_start;

        page_start = dc->base.pc_first & TARGET_PAGE_MASK;
        if (dc->base.pc_next - page_start >= TARGET_PAGE_SIZE) {
            dc->base.is_jmp = DISAS_TOO_MANY;
        }
    }

    /* TODO (issue #62): these must be removed */
    tcg_temp_free_i32(dc->zero);
    tcg_temp_free_i32(dc->one );

    /* verify if there is any TCG temporaries leakge */
    translator_loop_temp_check(dcbase);
}

static void arc_tr_tb_stop(DisasContextBase *dcbase, CPUState *cpu)
{
    DisasContext *dc = container_of(dcbase, DisasContext, base);

    switch (dc->base.is_jmp) {
    case DISAS_TOO_MANY:
    case DISAS_UPDATE:
        gen_gotoi_tb(dc, 0, dc->base.pc_next);
        break;
    case DISAS_BRANCH_IN_DELAYSLOT:
    case DISAS_NORETURN:
        break;
    default:
         g_assert_not_reached();
    }

    if (dc->base.num_insns == dc->base.max_insns &&
        (dc->base.tb->cflags & CF_LAST_IO)) {
        gen_io_end();
    }
}

static void arc_tr_disas_log(const DisasContextBase *dcbase, CPUState *cpu)
{
    DisasContext *dc = container_of(dcbase, DisasContext, base);

    qemu_log("IN: %s\n", lookup_symbol(dc->base.pc_first));
    log_target_disas(cpu, dc->base.pc_first, dc->base.tb->size);
}


static const TranslatorOps arc_translator_ops = {
    .init_disas_context = arc_tr_init_disas_context,
    .tb_start           = arc_tr_tb_start,
    .insn_start         = arc_tr_insn_start,
    .breakpoint_check   = arc_tr_breakpoint_check,
    .translate_insn     = arc_tr_translate_insn,
    .tb_stop            = arc_tr_tb_stop,
    .disas_log          = arc_tr_disas_log,
};

/* generate intermediate code for basic block 'tb'. */
void gen_intermediate_code(CPUState *cpu,
                           TranslationBlock *tb,
                           int max_insns)
{
    DisasContext dc;
    const TranslatorOps *ops = &arc_translator_ops;
    translator_loop(ops, &dc.base, cpu, tb, max_insns);
}

void restore_state_to_opc(CPUARCState *env,
                          TranslationBlock *tb,
                          target_ulong *data)
{
    env->pc = data[0];
}

void arc_cpu_dump_state(CPUState *cs, FILE *f, int flags)
{
    ARCCPU *cpu = ARC_CPU(cs);
    CPUARCState *env = &cpu->env;
    int i;

    qemu_fprintf(f, "STATUS:  [ %c %c %c %c %c %c %s %s %s %s %s %s %c]\n",
                        env->stat.Lf ? 'L' : '-',
                        env->stat.Zf ? 'Z' : '-',
                        env->stat.Nf ? 'N' : '-',
                        env->stat.Cf ? 'C' : '-',
                        env->stat.Vf ? 'V' : '-',
                        env->stat.Uf ? 'U' : '-',
                        env->stat.DEf ? "DE" : "--",
                        env->stat.AEf ? "AE" : "--",
                        env->stat.Ef ? "E" : "--",
                        env->stat.DZf ? "DZ" : "--",
                        env->stat.SCf ? "SC" : "--",
                        env->stat.IEf ? "IE" : "--",
                        env->stat.Hf ? 'H' : '-'
                        );

    qemu_fprintf(f, "\n");
    for (i = 0; i < ARRAY_SIZE(env->r); i++) {
        qemu_fprintf(f, "R[%02d]:  %02x   ", i, env->r[i]);

        if ((i % 8) == 7) {
            qemu_fprintf(f, "\n");
        }
    }
}


/*-*-indent-tabs-mode:nil;tab-width:4;indent-line-function:'insert-tab'-*-*/
/* vim: set ts=4 sw=4 et: */
