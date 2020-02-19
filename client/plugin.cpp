#include<iostream>
#include"Plugin/Main.hpp"

int main(int argc, char **argv) {
	return Plugin::Main(std::cin, std::cout, argc, argv).run();
}
