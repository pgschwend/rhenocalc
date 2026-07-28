#include "macoshelper.h"

#if defined(__APPLE__) || defined(Q_OS_MACOS)

#import <AppKit/AppKit.h>
#include <QWindow>

namespace MacOSHelper {
    void setWindowAppearance(void* windowHandle, bool dark) {
        if (!windowHandle) return;
        
        QWindow* qwindow = static_cast<QWindow*>(windowHandle);
        NSView* nsview = reinterpret_cast<NSView*>(qwindow->winId());
        NSWindow* nswindow = [nsview window];
        
        if (nswindow) {
            if (dark) {
                nswindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
            } else {
                nswindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
            }
        }
    }
}

#endif
