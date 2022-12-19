#ifndef CHECK_HPP
#define CHECK_HPP 1

#include <cstdio>
#include <cerrno>
#include <cstdlib>

inline void error(const char* file, int line)
{
	auto tmp = errno;
	fprintf(stderr, "%s (line %d) :", file, line);
	errno = tmp;
	perror(nullptr);
	exit(EXIT_FAILURE);
}

inline int xcheck(int p, const char* file, int line)
{
	if (p < 0)
        error(file, line);
	return p;
}

template<typename T>
inline T* xcheck(T* p, const char* file, int line)
{
	if (p == nullptr)
        error(file, line);
	return p;
}


#define check(x) xcheck(x, __FILE__, __LINE__ )
#endif