#include <iostream>
#include "linkedList.hpp"

void run() {
	LinkedList<std::string> ll;
	ll.prepend("zero");
	ll.prepend("one");
	ll.prepend("two");
	ll.removeHead();
	ll.append("four");
	ll.append("five");
	ll.append("seven");
	ll.removeTail();
	ll.print();
	if (!ll.isEmpty()) {
		std::cout << "Head: " << ll.getHead() << std::endl;
		std::cout << "Tail: " << ll.getTail() << std::endl;
	}
}


int main() {
	try {
		run();
	}
	catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
	return EXIT_SUCCESS;
}