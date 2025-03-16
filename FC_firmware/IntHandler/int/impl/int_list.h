#ifndef INT_IMPL_INT_LIST_H_
#define INT_IMPL_INT_LIST_H_

#include "int/int.h"

#define _INT_MAX_SUBSCR_COUNT 10

typedef struct {
    int_CallbackFn cbFn;
    void* context;
    void* halHandle;
} _int_CallbackListEntry;

extern _int_CallbackListEntry _int_callbackList[INT_EVENT_TYPE_COUNT][_INT_MAX_SUBSCR_COUNT + 1];

void _int_triggerCbs(int_IntEventType eventType, void* handle);

#endif /* INT_IMPL_INT_LIST_H_ */
