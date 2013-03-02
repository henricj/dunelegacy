#ifndef FUNCTIONAL_INCLUDED
#define FUNCTIONAL_INCLUDED

#include <tr1/functional>

namespace std {
	using std::tr1::bind;
	using std::tr1::function;

	namespace placeholders {
		using namespace std::tr1::placeholders;
	}
}

#endif //FUNCTIONAL_INCLUDED
