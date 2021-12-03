/*
 * oprintf.h
 */

#include "stdarg.h"

char *voprintf(char *fs, va_list arg_ptr);
char *oprintf(char *fs, ...);
