#pragma once
#include <limits>
#include <mutex>
#include <shared_mutex>

class Node {
	friend class RedBlackTree;

	std::shared_mutex rot_mtx;	//block all operations(search, insert, and rotate) when a rotation is in progress.
	std::mutex insert_mtx;//Unlike search locks, these locks protect the parent node rathen than the current node. 
	//They prevent other insertions on the parent node ,which also indirectly prevents rotations since rotations occur after insertions.

	Node* pParent;
	Node* pLeft;
	Node* pRight;
	int element;
	bool isRed;

public:
	Node(int val = std::numeric_limits<int>::min()) :pLeft{}, pRight{}, pParent{}, element{ val },isRed{ true }
	{}
	~Node() {
		delete pLeft;
		delete pRight;
		pParent = nullptr;
		pLeft = nullptr;
		pRight = nullptr;
	}

	int GetValue() const noexcept {
		return element;
	}

	Node* GetLeft() noexcept {
		return pLeft;
	}
	Node* GetRight() noexcept {
		return pRight;
	}
	Node* GetParent() noexcept {
		return pParent;
	}

	void SetLeft(Node* Left) noexcept {
		pLeft = Left;
	}
	void SetRight(Node* Right) noexcept {
		pRight = Right;
	}
	void SetParent(Node* Parent) noexcept {
		pParent = Parent;
	}
	void SetValue(int newValue) noexcept {
		element = newValue;
	}

	bool IsRed() const noexcept {
		return isRed;
	}

	void PaintRed() noexcept {
		isRed = true;
	}
	void PaintBlack() noexcept {
		isRed = false;
	}
	int Height() const {
		if (pLeft == nullptr && pRight == nullptr) {
			return 0;
		}
		int heightLeft = (pLeft == nullptr) ? 0 : pLeft->Height();
		int heightRight = (pRight == nullptr) ? 0 : pRight->Height();
		return 1 + ((heightLeft > heightRight) ? heightLeft : heightRight);
	}
	Node* GetChild(bool isLeftChild) noexcept {
		return (isLeftChild == true) ? pLeft : pRight;
	}
};