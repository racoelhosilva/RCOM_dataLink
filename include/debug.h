#ifndef _DEBUG_H_
#define _DEBUG_H_

// Logs debug information to the console.
// This function will only output to the console if the program is compiled with the flag -DDEBUG
void debugLog(const char *format, ...);

// Logs errors to the console, using the function name and custom messages.
void errorLog(const char *funcName, const char *messageFormat, ...);

#endif // _DEBUG_H_