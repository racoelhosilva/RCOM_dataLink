#ifndef _DEBUG_H_
#define _DEBUG_H_

void debugLog(const char *format, ...);
void errorLog(const char *funcName, const char *messageFormat, ...);

#endif // _DEBUG_H_