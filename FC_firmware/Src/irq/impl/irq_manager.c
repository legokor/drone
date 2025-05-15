#include "irq/impl/irq_list.h"
#include "irq/irq.h"

#include "stm32f4xx_hal.h"

static _int_CallbackListEntry _int_callbackList[INT_EVENT_TYPE_COUNT][_int_MAX_SUBSCR_COUNT + 1] = { 0 };

bool int_SubscribeToInt(int_IntEventType eventType, int_CallbackFn cbFnHandle, void* context, void* halHandle) {
    if (eventType >= INT_EVENT_TYPE_COUNT)
        return false;

    for (uint32_t p = 0; p < _int_MAX_SUBSCR_COUNT; p++) {
        if (_int_callbackList[eventType][p].cbFn == NULL) {
            __disable_irq();

            _int_callbackList[eventType][p].cbFn = cbFnHandle;
            _int_callbackList[eventType][p].context = context;
            _int_callbackList[eventType][p].halHandle = halHandle;

            __enable_irq();
            return true;
        }
    }

    return false;
}

static void _int_triggerCbs(int_IntEventType eventType, void* handle) {
    _int_CallbackListEntry* eventCbList = _int_callbackList[eventType];

    for (uint32_t i = 0; eventCbList[i].cbFn != NULL; i++) {
        if (eventCbList[i].halHandle == handle)
            eventCbList[i].cbFn(eventCbList[i].context);
    }
}
