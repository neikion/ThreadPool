#pragma once
#ifdef ThreaPoolLibrary
#define ThreadPoolAPI __declspec(dllexport)
#else
#define ThreadPoolAPI __declspec(dllimport)
#endif // !1

