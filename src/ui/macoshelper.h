#pragma once

#if defined(__APPLE__) || defined(Q_OS_MACOS)

namespace MacOSHelper {
    void setWindowAppearance(void* windowHandle, bool dark);
}

#endif
