#ifndef CLDCB_COMMON_MAIN_HPP
#define CLDCB_COMMON_MAIN_HPP

#include<functional>
#include<iostream>
#include<memory>
#include<string>
#include<utility>
#include<vector>

/*
Usage:

return Main("my-program")
	.add_method("whatever", handler_func)
	.main(argc, argv)
	;
*/
class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;
	explicit Main(std::string const& program_name);
	Main(Main&& o);
	Main& operator=(Main&& o) {
		auto tmp = std::move(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}
	~Main();

	Main& add_method( std::string const& method_name
			, std::function<int (std::vector<std::string>)> method
			);

	int main( int argc, char** argv
		, std::ostream& cout = std::cout
		, std::ostream& cerr = std::cerr
		);
};

#endif /* CLDCB_COMMON_MAIN_HPP */
