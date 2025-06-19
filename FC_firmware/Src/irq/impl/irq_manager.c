#include "err/err.h"
#include "irq/impl/irq_list.h"
#include "irq/irq.h"

#include "stm32f4xx_hal.h"

static _int_CallbackListEntry _int_callbackList[_int_EVENT_TYPE_COUNT][_int_MAX_SUBSCR_COUNT + 1] = { 0 };

void int_subscribeToInt(int_IntEventType eventType, int_CallbackFn cbFnHandle, void* context, void* halHandle) {
    err_assert(eventType >= _int_EVENT_TYPE_COUNT);

    for (int p = 0; p < _int_MAX_SUBSCR_COUNT; p++) {
        if (_int_callbackList[eventType][p].cbFn == NULL) {
            __disable_irq();

            _int_callbackList[eventType][p] = (_int_CallbackListEntry) {
                .cbFn = cbFnHandle,    //
                .context = context,    //
                .halHandle = halHandle //
            };

            __enable_irq();
            return;
        }
    }

    err_fatal("Event pool is full");
}

void _int_triggerCbs(int_IntEventType eventType, void* handle) {
    _int_CallbackListEntry* eventCbList = _int_callbackList[eventType];

    for (uint32_t i = 0; eventCbList[i].cbFn != NULL; i++) {
        if (eventCbList[i].halHandle == handle)
            eventCbList[i].cbFn(eventCbList[i].context);
    }
}
