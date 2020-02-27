#ifndef QEMU_SYSEMU_RESET_H
#define QEMU_SYSEMU_RESET_H

/*
 * The device reset handlers are executed in descending order of their priority
 * values (i.e. the reset handler with the greatest numerical priority value
 * will be executed first).
 */
#define QEMU_RESET_PRIORITY_DEFAULT     (0x8000)

/*
 * The priority level can range from -8 to +7, where the reset handler with the
 * highest priority level is executed first.
 */
#define QEMU_RESET_PRIORITY_LEVEL(l) \
    (QEMU_RESET_PRIORITY_DEFAULT + (l * 0x1000))

/*
 * Each priority level may be further divided into a maximum of 4096 sub-levels
 * (0 to 4095).
 */
#define QEMU_RESET_PRIORITY_SUBLEVEL(l, s) \
    (QEMU_RESET_PRIORITY_DEFAULT + (l * 0x1000) + s)

typedef void QEMUResetHandler(void *opaque);

void qemu_register_reset_with_priority(uint16_t priority,
    QEMUResetHandler *func, void *opaque);

void qemu_register_reset(QEMUResetHandler *func, void *opaque);
void qemu_unregister_reset(QEMUResetHandler *func, void *opaque);
void qemu_devices_reset(void);

#endif
