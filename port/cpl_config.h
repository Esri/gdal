#if defined(RTC_COCOA_FAMILY)
#include "cpl_config_macos.h"
#elif defined(RTC_LINUX_DESKTOP)
#include "cpl_config_linux.h"
#elif defined(RTC_WINDOWS_DESKTOP)
#include "cpl_config_windows.h"
#endif
