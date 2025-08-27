#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "node.hpp"

template<class T>
class LinkedList {
private:
	std::unique_ptr<Node<T>> head;
	Node<T>* tail;
public:

	LinkedList() : head(nullptr), tail(nullptr) {}


	~LinkedList() { clear(); }


	bool isEmpty() { return head == nullptr; }


	void clear() {
		while (!isEmpty()) {
			removeHead();
		}
	}


	void removeHead() {
		if (!isEmpty()) {
			head = std::move(head->getNextPtr());
		}
		if (!head) {
			tail = nullptr;
		}
	}


	void removeTail() {
		if (isEmpty()) {
			return;
		}
		else {
			if (head.get() == tail) {
				head.reset();
				tail = nullptr;
				return;
			}
			Node<T>* previous = head.get();
			while (previous->getNext() != tail) {
				previous = previous->getNext();
			}
			previous->getNextPtr().reset();
			tail = previous;
		}
	}


	void prepend(T value) {
		std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(value);
		newNode->getNextPtr() = std::move(head);
		head = std::move(newNode);
		if (!tail) {
			tail = head.get();
		}
	}


	void append(T value) {
		std::unique_ptr<Node<T>> newNode = std::make_unique<Node<T>>(value);
		if (isEmpty()) {
			head = std::move(newNode);
			tail = head.get();
		}
		else {
			tail->getNextPtr() = std::move(newNode);
			tail = tail->getNext();
		}
	}


	void print() {
		std::ostringstream output;
		if (!isEmpty()) {
			Node<T>* current = head.get();
			while (current) {
				output << current->getData() << ", ";
				current = current->getNext();
			}
		}
		std::cout << output.str() << std::endl;
	}


	T getHead() { return isEmpty() ? T{} : head->getData();	}


	T getTail() { return isEmpty() ? T{} : tail->getData(); }
};