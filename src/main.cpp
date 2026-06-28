#include "xtdraw_app.h"
#include <signal.h>

xtdraw::App app;

void OnTerminationSignal() {
    app.OnTerminationSignal();
}
void OnTerminationSignal(int) {
    app.OnTerminationSignal();
}
void OnResizeSignal(int) {
    app.OnResizeSignal();
}

int main() {
    xtdraw::App app;
    app.Init();
    atexit(OnTerminationSignal);
    signal(SIGINT, OnTerminationSignal);
    signal(SIGTERM, OnTerminationSignal);
    signal(SIGWINCH, OnResizeSignal);

    return app.Run() ? 0 : 1;
}
