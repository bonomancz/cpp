#pragma once
#include <string>
#include <memory>

template<class T>
class Node {
private:
	T data;
	std::unique_ptr<Node<T>> next;
public:
	Node(T value) : data(value), next(nullptr) {}

	T getData() const { return data; }

	Node* getNext() const { return next.get(); }

	std::unique_ptr<Node<T>>& getNextPtr() { return next; }
};