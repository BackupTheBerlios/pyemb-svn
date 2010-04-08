#ifndef CDEBUG_H
#define CDEBUG_H

#include <iostream>
struct nullstream:
std::ostream {
	nullstream(): std::ios(0), std::ostream(0) {}
};

#ifdef _DEBUG
#define CDEBUG std::cout
#else
#define CDEBUG nullstream()
#endif

#endif
