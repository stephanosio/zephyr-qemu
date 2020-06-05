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
#include "cpu.h"
#include "hw/hw.h"
#include "hw/boards.h"
#include "elf.h"
#include "hw/char/serial.h"
#include "net/net.h"
#include "hw/loader.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"
#include "sysemu/reset.h"
#include "sysemu/runstate.h"
#include "sysemu/sysemu.h"
#include "hw/sysbus.h"
#include "hw/arc/cpudevs.h"
#include "boot.h"

static void arc_sim_net_init(MemoryRegion *address_space,
                             hwaddr base,
                             hwaddr descriptors,
                             qemu_irq irq, NICInfo *nd)
{
    DeviceState *dev;
    SysBusDevice *s;

    dev = qdev_create(NULL, "open_eth");
    qdev_set_nic_properties(dev, nd);
    qdev_init_nofail(dev);

    s = SYS_BUS_DEVICE(dev);
    sysbus_connect_irq(s, 0, irq);
    memory_region_add_subregion(address_space, base,
                                sysbus_mmio_get_region(s, 0));
    memory_region_add_subregion(address_space, descriptors,
                                sysbus_mmio_get_region(s, 1));
}

static uint64_t arc_io_read(void *opaque, hwaddr addr,
                            unsigned size)
{
    return 0;
}

static void arc_io_write(void *opaque, hwaddr addr,
                         uint64_t val, unsigned size)
{
    switch (addr) {
    case 0x08: /* board reset.  */
        qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
        break;
    default:
        break;
    }
}

static const MemoryRegionOps arc_io_ops = {
    .read = arc_io_read,
    .write = arc_io_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void arc_sim_init(MachineState *machine)
{
    static struct arc_boot_info boot_info;
    unsigned int smp_cpus = machine->smp.cpus;
    ram_addr_t ram_base = 0;
    ram_addr_t ram_size = machine->ram_size;
    ARCCPU *cpu = NULL;
    MemoryRegion *ram, *system_io;
    int n;

    boot_info.ram_start = ram_base;
    boot_info.ram_size = ram_size;
    boot_info.kernel_filename = machine->kernel_filename;

    for (n = 0; n < smp_cpus; n++) {
        cpu = ARC_CPU(object_new(machine->cpu_type));
        if (cpu == NULL) {
            fprintf(stderr, "Unable to find CPU definition!\n");
            exit(1);
        }

        /* Set the initial CPU properties.  */
        object_property_set_uint (OBJECT (cpu), 1000000, "freq_hz",
                                  &error_fatal);
        object_property_set_bool (OBJECT (cpu), true, "rtc-opt",
                                  &error_fatal);
        object_property_set_bool(OBJECT (cpu), true, "realized", &error_fatal);

        /* Initialize internal devices.  */
        cpu_arc_pic_init(cpu);
        cpu_arc_clock_init(cpu);

        qemu_register_reset(arc_cpu_reset, cpu);
    }

    ram = g_new(MemoryRegion, 1);
    memory_region_init_ram(ram, NULL, "arc.ram", ram_size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), ram_base, ram);

    system_io = g_new(MemoryRegion, 1);
    memory_region_init_io(system_io, NULL, &arc_io_ops, NULL, "arc.io",
                           1024);
    memory_region_add_subregion(get_system_memory(), 0xf0000000, system_io);

    serial_mm_init(get_system_memory(), 0x90000000, 2, cpu->env.irq[20],
                   115200, serial_hd(0), DEVICE_NATIVE_ENDIAN);

    if (nd_table[0].used) {
        arc_sim_net_init(get_system_memory(), 0x92000000,
                              0x92000400, cpu->env.irq[4], nd_table);
    }

    arc_load_kernel(cpu, &boot_info);
}

static void arc_sim_machine_init(MachineClass *mc)
{
    mc->desc = "ARCxx simulation";
    mc->init = arc_sim_init;
    mc->max_cpus = 1;
    mc->is_default = false;
    mc->default_cpu_type = ARC_CPU_TYPE_NAME("archs");
}

DEFINE_MACHINE("arc-sim", arc_sim_machine_init)


/*-*-indent-tabs-mode:nil;tab-width:4;indent-line-function:'insert-tab'-*-*/
/* vim: set ts=4 sw=4 et: */
