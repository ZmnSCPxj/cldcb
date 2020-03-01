#include<ev.h>
#include"Ev/now.hpp"

namespace Ev {

double now() {
	return ev_now(EV_DEFAULT);
}

}
