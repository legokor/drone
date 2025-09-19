#include "err/err.h"
#include "irq/impl/irq_list.h"
#include "irq/irq.h"

#include "stm32f4xx_hal.h"

static _irq_CallbackListEntry _irq_callbackList[_irq_EVENT_TYPE_COUNT][_irq_MAX_SUBSCR_COUNT + 1] = { 0 };

void irq_subscribeToIrq(irq_IntEventType eventType, irq_CallbackFn cbFnHandle, void* context, void* halHandle) {
    err_assert(eventType < _irq_EVENT_TYPE_COUNT);

    for (int p = 0; p < _irq_MAX_SUBSCR_COUNT; p++) {
        if (_irq_callbackList[eventType][p].cbFn == NULL) {
            __disable_irq();

            _irq_callbackList[eventType][p] = (_irq_CallbackListEntry) {
                .cbFn = cbFnHandle,    //
                .context = context,    //
                .halHandle = halHandle //
            };

            __enable_irq();
            return;
        }
    }

    err_fatal("Failed to register interrupt");
}

void _irq_triggerCbs(irq_IntEventType eventType, void* handle) {
    _irq_CallbackListEntry* eventCbList = _irq_callbackList[eventType];

    for (uint32_t i = 0; eventCbList[i].cbFn != NULL; i++) {
        if (eventCbList[i].halHandle == handle)
            eventCbList[i].cbFn(eventCbList[i].context);
    }
}
