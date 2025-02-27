#include "int/impl/int_list.h"
#include "int/int.h"

_int_CallbackListEntry _int_callbackList[INT_EVENT_TYPE_COUNT][_INT_MAX_SUBSCR_COUNT + 1] = { 0 };

uint8_t int_SubscribeToInt(int_IntEventType eventType, int_CallbackFn cbFnHandle, void* context, void* halHandle) {
    if (eventType >= INT_EVENT_TYPE_COUNT)
        return 0;

    for (uint32_t p = 0; p < _INT_MAX_SUBSCR_COUNT; p++) {
        if (_int_callbackList[eventType][p].cbFn == 0) {
            _int_callbackList[eventType][p].cbFn = cbFnHandle;
            _int_callbackList[eventType][p].context = context;
            _int_callbackList[eventType][p].halHandle = halHandle;
            return 1;
        }
    }
    return 0;
}
