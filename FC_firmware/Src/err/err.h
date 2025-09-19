#ifndef ERR_H
#define ERR_H

#include <stdbool.h>
#include "utils/utils.h"

#include "stm32f4xx_hal.h"

#define err_try(expr)                 \
    do {                              \
        bool __err_status__ = (expr); \
        if (!__err_status__) {        \
            return false;             \
        }                             \
    } while (0)

/// FATAL ERRORS

/**
 * Triggers a fatal error.
 *
 * @param descr The errors description
 */
void err_handle_fatal(const char* descr);

/**
 * Shorthand for triggering a fatal error.
 * This macro includes the call location in the message.
 *
 * @param expr  The expression
 * @param descr The description of the error
 */
#define err_fatal(descr) _err_fatal(descr, __FILE__, __LINE__)
#define _err_fatal(descr, file, line) err_handle_fatal(descr " (" file ":" utils_EXPAND_AND_QUOTE(line) ")")

/**
 * Shorthand for trying to run a boolean expression and triggering a faltal error when it fails.
 * This macro includes the call location in the message.
 *
 * @param expr  The expression
 * @param descr The description of the error
 */
#define err_tryFatal(expr, descr) _err_tryFatal(expr, descr, __FILE__, __LINE__)
#define _err_tryFatal(expr, descr, file, line) \
    do {                                       \
        bool __err_status__ = (expr);          \
        if (!__err_status__) {                 \
            err_fatal(descr);                  \
        }                                      \
    } while (0)

/// IGNORABLE ERRORS

/**
 * Triggers an ignorable error.
 *
 * @param descr The errors description
 */
void err_handle_ignorable(const char* descr);

/**
 * Shorthand for triggering an ignorable error.
 * This macro includes the call location in the message.
 *
 * @param expr  The expression
 * @param descr The description of the error
 */
#define err_ignorable(descr) _err_ignorable(descr, __FILE__, __LINE__)
#define _err_ignorable(descr, file, line) err_handle_ignorable(descr " (" file ":" utils_EXPAND_AND_QUOTE(line) ")")

/**
 * Shorthand for trying to run a boolean expression and triggering an ignorable error when it fails.
 * This macro includes the call location in the message.
 *
 * @param expr  The expression
 * @param descr The description of the error
 */
#define err_tryIgnorable(expr, descr) _err_tryIgnorable(expr, descr, __FILE__, __LINE__)
#define _err_tryIgnorable(expr, descr, file, line) \
    do {                                           \
        bool __err_status__ = (expr);              \
        if (!__err_status__) {                     \
            err_ignorable(descr);                  \
        }                                          \
    } while (0)

// ASSERTS

/**
 * Evaluates an expression, and if it is false, then it triggers a fatal error.
 * If the DEBUG flag is defined.
 *
 * @param expr The expression to be asserted
 */
#define err_assert(expr) _err_assert(expr, __FILE__, __LINE__)

#ifdef DEBUG

#define _err_assert(expr, file, line)                       \
    do {                                                    \
        bool __err_assert_status__ = (expr);                \
        if (!__err_assert_status__) {                       \
            err_fatal("Assert failed: " utils_QUOTE(expr)); \
        }                                                   \
    } while (0)

#else

#define _err_assert(expr, file, line) \
    do {                              \
    } while (0)

#endif

#define err_todo(...) err_fatal("TODO" __VA_OPT__(": " reason))

#endif // ERR_H
