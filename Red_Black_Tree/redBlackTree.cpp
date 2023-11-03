#include "redBlackTree.h"

void RedBlackTree::CreatePrintQueueByDepth(Node* root, std::queue<Node*>& printQueue, int depth) const
{
	if (depth == 0) {
		printQueue.push(root);
	}
	else if (root == nullptr) {
		CreatePrintQueueByDepth(nullptr, printQueue, depth - 1);
		CreatePrintQueueByDepth(nullptr, printQueue, depth - 1);
	}
	else {
		if (pRoot->GetLeft() == nullptr && root->GetRight() == nullptr) {
			CreatePrintQueueByDepth(nullptr, printQueue, depth - 1);
			CreatePrintQueueByDepth(nullptr, printQueue, depth - 1);
		}
		else {
			CreatePrintQueueByDepth(root->GetLeft(), printQueue, depth - 1);
			CreatePrintQueueByDepth(root->GetRight(), printQueue, depth - 1);
		}
	}
}

std::queue<Node*> RedBlackTree::CreatePrintQueue() const
{
	if (pRoot == nullptr) {
		return std::queue<Node*>();
	}

	std::queue<Node*> result;
	const int height = pRoot->Height();
	for (int depth = 0; depth <= height; depth++) {
		CreatePrintQueueByDepth(pRoot, result, depth);
	}

	return result;
}