#ifndef INT_IMPL_INT_LIST_H_
#define INT_IMPL_INT_LIST_H_

#include "int/int.h"

#define _INT_MAX_SUBSCR_COUNT 10

typedef struct _int_CallbackListEntry {
    int_CallbackFn cbFn;

    // driver instance
    void* context;

    // interrupt source handle (HAL)
    void* halHandle;
} _int_CallbackListEntry;

void _int_triggerCbs(int_IntEventType eventType, void* handle);

#endif /* INT_IMPL_INT_LIST_H_ */
