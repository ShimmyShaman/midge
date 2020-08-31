/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

extern "C" {
void midge_initialize_app(struct timespec *app_begin_time);
void midge_run_app();
void midge_cleanup_app();
}

#endif // MIDGE_APP_H