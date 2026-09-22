#include <signal.h>

#include "xtdraw_app.h"

xtdraw::App app;

void OnTerminationSignal() { app.OnTerminationSignal(); }
void OnTerminationSignal(int) { app.OnTerminationSignal(); }
void OnResizeSignal(int) { app.OnResizeSignal(); }

int main() {
    signal(SIGINT, OnTerminationSignal);
    signal(SIGTERM, OnTerminationSignal);
    signal(SIGWINCH, OnResizeSignal);
    atexit(OnTerminationSignal);

    app.Init();

    return app.Run() ? 0 : 1;
}
