#ifndef SLIME_DELAY_H
#define SLIME_DELAY_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief				Delay given amount of time in milliseconds.
 * @param milliseconds	The count of time to delay in milliseconds.
 */
void slime_delay_milliseconds(uint32_t milliseconds);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //SLIME_DELAY_H
