#ifndef INT_IMPL_INT_LIST_H_
#define INT_IMPL_INT_LIST_H_

#include "irq/irq.h"

#define _irq_MAX_SUBSCR_COUNT 10

typedef struct [[nodiscard]] _irq_CallbackListEntry {
    irq_CallbackFn cbFn;

    // driver instance
    void* context;

    // interrupt source handle (HAL)
    void* halHandle;
} _irq_CallbackListEntry;

void _irq_triggerCbs(irq_IntEventType eventType, void* handle);

#endif /* INT_IMPL_INT_LIST_H_ */
