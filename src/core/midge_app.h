/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

extern "C" {
int midge_initialize_app();
int midge_run_app();
int midge_cleanup_app();
}

#endif // MIDGE_APP_H