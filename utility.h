#pragma once
#ifdef ThreaPoolLibrary
#define ThreadPoolexport __declspec(dllexport)
#else
#define ThreadPoolimport __declspec(dllimport)
#endif // !1

