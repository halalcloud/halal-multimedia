#include "bbits.h"
uint32_t bbits_get_version()
{
    return BBITS_VERSION;
}
int32_t bbits_is_compatiable()
{
    uint32_t major = bbits_get_version() >> 16;
    return major == BBITS_VERSION_MAJOR;
}

