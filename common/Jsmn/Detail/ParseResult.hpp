#ifndef CLDCB_COMMON_JSMN_DETAIL_PARSERESULT_HPP
#define CLDCB_COMMON_JSMN_DETAIL_PARSERESULT_HPP

#include<string>
#include<vector>
#include"Jsmn/Detail/Token.hpp"

namespace Jsmn { namespace Detail {

struct ParseResult {
	std::string orig_string;
	std::vector<Token> tokens;
};

}}

#endif /* CLDCB_COMMON_JSMN_DETAIL_PARSERESULT_HPP */
