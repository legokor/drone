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

#define _INT_TRIGGER_CBS(EVENT_TYPE, ARG)                                    \
    do {                                                                     \
        _int_CallbackListEntry* eventCbList = _int_callbackList[EVENT_TYPE]; \
        for (size_t i = 0; eventCbList[i].cbFn != 0; i++) {                  \
            if (eventCbList[i].halHandle == ARG)                             \
                eventCbList[i].cbFn(eventCbList[i].context);                 \
        }                                                                    \
    } while (0)

#endif /* INT_IMPL_INT_LIST_H_ */
