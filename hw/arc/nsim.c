/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */


#include "qemu/osdep.h"
#include "qapi/error.h"
#include "boot.h"
#include "sysemu/reset.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/char/serial.h"
#include "exec/address-spaces.h"
#include "hw/arc/arc_uart.h"
#include "hw/arc/cpudevs.h"
#include "hw/sysbus.h"

#define NSIM_RAM_BASE		0x80000000
#define NSIM_RAM_SIZE		0x40000000
#define NSIM_ARC_UART_OFFSET	0xc0fc1000

static void nsim_init(MachineState *machine)
{
    static struct arc_boot_info boot_info;
    unsigned int smp_cpus = machine->smp.cpus;
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *system_ram;
    ARCCPU *cpu = NULL;
    int n;

    boot_info.ram_start = NSIM_RAM_BASE;
    boot_info.ram_size = NSIM_RAM_SIZE;
    boot_info.kernel_filename = machine->kernel_filename;
    boot_info.kernel_cmdline = machine->kernel_cmdline;

    for (n = 0; n < smp_cpus; n++) {
        cpu = ARC_CPU (cpu_create ("archs-" TYPE_ARC_CPU));
        if (cpu == NULL) {
            fprintf(stderr, "Unable to find CPU definition!\n");
            exit(1);
        }

       /* Initialize internal devices.  */
        cpu_arc_pic_init (cpu);
        cpu_arc_clock_init (cpu);

        qemu_register_reset(arc_cpu_reset, cpu);
    }

    /* Init system DDR */
    system_ram = g_new(MemoryRegion, 1);
    memory_region_init_ram(system_ram, NULL, "arc.ram", NSIM_RAM_SIZE,
                           &error_fatal);
    memory_region_add_subregion(system_memory, NSIM_RAM_BASE, system_ram);

    /* Init ARC UART */
    arc_uart_create(get_system_memory(), NSIM_ARC_UART_OFFSET, serial_hd(0), cpu->env.irq[24]);

    arc_load_kernel(cpu, &boot_info);
}

static void nsim_machine_init(MachineClass *mc)
{
    mc->desc = "ARC nSIM";
    mc->init = nsim_init;
    mc->max_cpus = 1;
    mc->is_default = false;
}

DEFINE_MACHINE("nsim", nsim_machine_init)
