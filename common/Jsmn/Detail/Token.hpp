#ifndef CLDCB_COMMON_JSMN_DETAIL_TOKEN_HPP
#define CLDCB_COMMON_JSMN_DETAIL_TOKEN_HPP

#include"Jsmn/Detail/Type.hpp"

namespace Jsmn { namespace Detail {

struct Token {
	Type type;
	int start;
	int end;
	int size;
};

}}

#endif /* CLDCB_COMMON_JSMN_DETAIL_TOKEN_HPP */
