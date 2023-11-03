#pragma once
#include "node.h"
#include <iostream>//std::cerr
#include <queue>//print queue
#include <string>//to_string

#define MULTI_THREADS

class RedBlackTree {
	int size;
	Node* pRoot;

private:
	void CreatePrintQueueByDepth(Node* root, std::queue<Node*>& printQueue, int depth) const;
	std::queue<Node*> CreatePrintQueue() const;
	std::mutex root_insert_mtx;
	std::shared_mutex root_rot_mtx;
	//std::mutex print_mtx;

public:
	RedBlackTree() :size{}, pRoot{ new Node{} } {}
	~RedBlackTree() { delete pRoot; pRoot = nullptr; }
	bool Search(int value) {
		bool found{ false };
		Node* node{ pRoot };
		Node* parent{ nullptr };
		std::shared_lock<std::shared_mutex> rotateLock(node->rot_mtx);
		//while (node != nullptr && found == false) {
		while (node != nullptr && found == false) {
			parent = node;
			if (value < node->GetValue()) {
				node = node->GetLeft();
			}
			else if (value > node->GetValue()) {
				node = node->GetRight();
			}
			else {
				found = true;
			}
			//std::unique_lock<std::mutex> nextSearchLock(node->rot_mtx);//hand-over-hand locking
			//rotateLock.unlock();
			//rotateLock = std::move(nextSearchLock);
			if (node != nullptr && node != parent) {
				std::shared_lock<std::shared_mutex> nextSearchLock(node->rot_mtx); // hand-over-hand locking
				rotateLock.unlock();
				rotateLock = std::move(nextSearchLock);
			}
		}
		rotateLock.unlock();
		return found;
	}

	void ConnectLeftChild(Node* parent,Node* child) {
		if (parent == nullptr) {
			//std::cerr << "ConnectLeftChild::null node\n";
			return;//throw?
		}
		if (child != nullptr) {
			child->SetParent(parent);
		}
		parent->SetLeft(child);
	}
	void ConnectRightChild(Node* parent, Node* child) {
		if (parent == nullptr) {
			//std::cerr << "ConnectRightChild::null node\n";
			return;//throw?
		}
		if (child != nullptr) {
			child->SetParent(parent);
		}
		parent->SetRight(child);
	}
	bool Add(int value){
		bool found{ false };
		Node* node{ pRoot };
		std::unique_lock<std::mutex> add_lk(pRoot->insert_mtx);
		Node* add_locked{node};

		Node* parent{ nullptr };
		while (node != nullptr && found==false) {
			parent = node;
			if (value < node->GetValue()) {
				node = node->GetLeft();
			}
			else if (value > node->GetValue()) {
				node = node->GetRight();
			}
			else {
				found=true;
			}
			if (node !=nullptr && found==false){
				if (node->IsRed() == false && parent->IsRed() == false && parent != add_locked) {
					std::unique_lock<std::mutex> new_lk(parent->insert_mtx);

					add_lk.unlock();
					add_lk = std::move(new_lk);
					//parent->insert_mtx.lock();
					//add_locked->insert_mtx.unlock();
					add_locked = parent;
				}
			}
		}
		if (found == false) {//need to add
			//newNode->SetParent(parent);
			node = new Node(value);
			std::unique_lock<std::shared_mutex> node_lk(node->rot_mtx);
			if (value < parent->GetValue()) {
				ConnectLeftChild(parent, node);
			}
			else {
				ConnectRightChild(parent, node);
			}
			node_lk.unlock();

			//node->PaintRed();
			//node->SetValue(value);
			//----------------------------
			//if (left!=nullptr) {
			//	left->PaintBlack();
			//}
			//if (right != nullptr) {
			//	right->PaintBlack();
			//}
			//ConnectLeftChild(node,left);
			//ConnectRightChild(node, right);
			//-------------------------------
			while (node!=pRoot 
				&& node->IsRed() == true
				&& node->GetParent()->IsRed() == true){
				parent = node->GetParent();
				Node* grandParent = parent->GetParent();
				Node* uncle{ nullptr };
				if (grandParent == nullptr) {
					pRoot->PaintBlack();//not sure
					return !found;//not sure
				}
				if (parent == grandParent->GetLeft()) {//pParent, uncle
					uncle = grandParent->GetRight();
					if (uncle != nullptr && uncle->IsRed() == true) {
						uncle->PaintBlack();
						parent->PaintBlack();
						grandParent->PaintRed();
						node = grandParent;
						parent = node->GetParent();
					}
					else if (node == parent->GetLeft()) {
						parent->PaintBlack();
						grandParent->PaintRed();
						Node* sibling = parent->GetRight();
						Node* greatGrandParent = grandParent->GetParent();
						//ConnectRightChild(parent, grandParent);
						//ConnectLeftChild(grandParent, sibling);
						if (grandParent == pRoot) {
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							//grandParent->rot_mtx.lock();
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							//parent->rot_mtx.lock();
							if (sibling != nullptr) {
								sibling->rot_mtx.lock();
							}
							pRoot = parent;
							ConnectRightChild(parent, grandParent);
							ConnectLeftChild(grandParent, sibling);
							if (sibling != nullptr) {
								sibling->rot_mtx.unlock();
							}
							p_lk.unlock();
							gp_lk.unlock();
							//parent->rot_mtx.unlock();
							//grandParent->rot_mtx.unlock();
						}
						else {
							std::unique_lock<std::shared_mutex> ggp_lk(greatGrandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							//greatGrandParent->rot_mtx.lock();
							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							if (sibling != nullptr) {
								sibling->rot_mtx.lock();
							}
							if (grandParent == greatGrandParent->GetLeft()) {
								ConnectLeftChild(greatGrandParent,parent);
							}
							else {
								ConnectRightChild(greatGrandParent, parent);
							}
							ConnectRightChild(parent, grandParent);
							ConnectLeftChild(grandParent, sibling);
							if (sibling != nullptr) {
								sibling->rot_mtx.unlock();
							}
							p_lk.unlock();
							//parent->rot_mtx.unlock();
							gp_lk.unlock();
							//grandParent->rot_mtx.unlock();
							ggp_lk.unlock();
							//greatGrandParent->rot_mtx.unlock();
						}
					}
					else {
						node->PaintBlack();
						grandParent->PaintRed();
						Node* left = node->GetLeft();
						Node* right = node->GetRight();
						Node* greatGrandParent = grandParent->GetParent();
						//ConnectLeftChild(node, parent);
						//ConnectRightChild(node, grandParent);
						//ConnectRightChild(parent, left);
						//ConnectLeftChild(grandParent, right);
						if (grandParent == pRoot) {
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							//grandParent->rot_mtx.lock();
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							//parent->rot_mtx.lock();
							std::unique_lock<std::shared_mutex> lk(node->rot_mtx);
							//node->rot_mtx.lock();
							if (left != nullptr) {
								left->rot_mtx.lock();
							}
							if (right != nullptr) {
								right->rot_mtx.lock();
							}
							pRoot = node;
							ConnectLeftChild(node, parent);
							ConnectRightChild(node, grandParent);
							ConnectRightChild(parent, left);
							ConnectLeftChild(grandParent, right);
							if (right != nullptr) {
								right->rot_mtx.unlock();
							}
							if (left != nullptr) {
								left->rot_mtx.unlock();
							}
							lk.unlock();
							//node->rot_mtx.unlock();
							p_lk.unlock();
							//parent->rot_mtx.unlock();
							gp_lk.unlock();
							//grandParent->rot_mtx.unlock();
						}
						else {
							std::unique_lock<std::shared_mutex> ggp_lk(greatGrandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							std::unique_lock<std::shared_mutex> n_lk(node->rot_mtx);
							//greatGrandParent->rot_mtx.lock();
							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							//node->rot_mtx.lock();
							if (left != nullptr) {
								left->rot_mtx.lock();
							}
							if (right != nullptr) {
								right->rot_mtx.lock();
							}

							if (grandParent == greatGrandParent->GetLeft()) {
								ConnectLeftChild(greatGrandParent, node);
							}
							else {
								ConnectRightChild(greatGrandParent, node);
							}
							ConnectLeftChild(node, parent);
							ConnectRightChild(node, grandParent);
							ConnectRightChild(parent, left);
							ConnectLeftChild(grandParent, right);
							if (right != nullptr) {
								right->rot_mtx.unlock();
							}
							if (left != nullptr) {
								left->rot_mtx.unlock();
							}
							n_lk.unlock();
							//node->rot_mtx.unlock();
							p_lk.unlock();
							//parent->rot_mtx.unlock();
							gp_lk.unlock();
							//grandParent->rot_mtx.unlock();
							ggp_lk.unlock();
							//greatGrandParent->rot_mtx.unlock();
						}
					}
				}
				else {//uncle, pParent
					uncle = grandParent->GetLeft();
					if (uncle!=nullptr && uncle->IsRed() == true) {
						uncle->PaintBlack();
						parent->PaintBlack();
						grandParent->PaintRed();
						node = grandParent;
						parent = node->GetParent();
					}
					else if (node == parent->GetRight()) {
						parent->PaintBlack();
						grandParent->PaintRed();
						Node* sibling = parent->GetLeft();
						Node* greatGrandParent = grandParent->GetParent();
						ConnectLeftChild(parent, grandParent);
						ConnectRightChild(grandParent, sibling);
						if (grandParent == pRoot) {
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							if (sibling != nullptr) {
								sibling->rot_mtx.lock();
							}

							pRoot = parent;
							ConnectLeftChild(parent, grandParent);
							ConnectRightChild(grandParent, sibling);
							if (sibling != nullptr) {
								sibling->rot_mtx.unlock();
							}
							p_lk.unlock();
							gp_lk.unlock();
							//parent->rot_mtx.unlock();
							//grandParent->rot_mtx.unlock();
						}
						else {
							std::unique_lock<std::shared_mutex> ggp_lk(greatGrandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							//greatGrandParent->rot_mtx.lock();
							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							if (sibling != nullptr) {
								sibling->rot_mtx.lock();
							}

							if (grandParent == greatGrandParent->GetLeft()) {
								ConnectLeftChild(greatGrandParent, parent);
							}
							else {
								ConnectRightChild(greatGrandParent, parent);
							}
							ConnectLeftChild(parent, grandParent);
							ConnectRightChild(grandParent, sibling);
							if (sibling != nullptr) {
								sibling->rot_mtx.unlock();
							}
							p_lk.unlock();
							gp_lk.unlock();
							ggp_lk.unlock();
							//parent->rot_mtx.unlock();
							//grandParent->rot_mtx.unlock();
							//greatGrandParent->rot_mtx.unlock();
						}
					}
					else { //node is black && pLeft child of pParent
						node->PaintBlack();
						grandParent->PaintRed();
						Node* left = node->GetLeft();
						Node* right = node->GetRight();
						Node* greatGrandParent = grandParent->GetParent();

						//ConnectRightChild(node, parent);
						//ConnectLeftChild(node, grandParent);
						//ConnectLeftChild(parent,right);
						//ConnectRightChild(grandParent,left);
						if (grandParent == pRoot) {
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							std::unique_lock<std::shared_mutex> n_lk(node->rot_mtx);

							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							//node->rot_mtx.lock();
							if (left != nullptr) {
								left->rot_mtx.lock();
							}
							if (right != nullptr) {
								right->rot_mtx.lock();
							}

							pRoot = node;
							ConnectRightChild(node, parent);
							ConnectLeftChild(node, grandParent);
							ConnectLeftChild(parent,right);
							ConnectRightChild(grandParent,left);

							if (right != nullptr) {
								right->rot_mtx.unlock();
							}
							if (left != nullptr) {
								left->rot_mtx.unlock();
							}
							n_lk.unlock();
							//node->rot_mtx.unlock();
							p_lk.unlock();
							//parent->rot_mtx.unlock();
							gp_lk.unlock();
							//grandParent->rot_mtx.unlock();
						}
						else {
							std::unique_lock<std::shared_mutex> ggp_lk(greatGrandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> gp_lk(grandParent->rot_mtx);
							std::unique_lock<std::shared_mutex> p_lk(parent->rot_mtx);
							std::unique_lock<std::shared_mutex> n_lk(node->rot_mtx);

							//greatGrandParent->rot_mtx.lock();
							//grandParent->rot_mtx.lock();
							//parent->rot_mtx.lock();
							//node->rot_mtx.lock();
							if (left != nullptr) {
								left->rot_mtx.lock();
							}
							if (right != nullptr) {
								right->rot_mtx.lock();
							}

							if (grandParent == greatGrandParent->GetLeft()) {
								ConnectLeftChild(greatGrandParent, node);
							}
							else{
								ConnectRightChild(greatGrandParent, node);
							}
							ConnectRightChild(node, parent);
							ConnectLeftChild(node, grandParent);
							ConnectLeftChild(parent, right);
							ConnectRightChild(grandParent, left);

							if (right != nullptr) {
								right->rot_mtx.unlock();
							}
							if (left != nullptr) {
								left->rot_mtx.unlock();
							}
							n_lk.unlock();
							p_lk.unlock();
							gp_lk.unlock();
							ggp_lk.unlock();
							//node->rot_mtx.unlock();
							//parent->rot_mtx.unlock();
							//grandParent->rot_mtx.unlock();
							//greatGrandParent->rot_mtx.unlock();
						}
					}
				}
			}
		}
		pRoot->PaintBlack();
		add_lk.unlock();
		//add_locked->insert_mtx.unlock();

		return !found;
	}
	void PrintTree() {

		if (pRoot != nullptr) {
			std::queue<Node*> printQueue = CreatePrintQueue();

			int nodeRowCount = 0;
			int nodeRowTarget = 1;
			int coutWidth = 4 << pRoot->Height();
			int offset{};
			while (printQueue.empty() == false) {
				if (nodeRowCount == 0) {
					std::cout.width(coutWidth / 2);
				}
				else {
					std::cout.width(coutWidth - 1 + offset);
				}

				if (printQueue.front() != nullptr) {
					Node* node = printQueue.front();
					std::cout << 
						((node->GetValue() == std::numeric_limits<int>::min()) ? "X" : std::to_string(node->GetValue()))
						<< "(";
					if (node->IsRed() == false) {
						std::cout << "B)";
					}
					else {
						std::cout << "R)";
					}
					offset = -3;
				}
				else {
					std::cout << "X";
					offset = 0;
				}

				// check if we are done with the current row
				if (++nodeRowCount == nodeRowTarget) {
					std::cout << std::endl;
					nodeRowTarget *= 2;
					coutWidth /= 2;
					nodeRowCount = 0;
				}
				else {
					std::cout << ' ';
				}
				printQueue.pop();
			}
		}
		else {
			std::cout << "Tree is Empty" << std::endl;
		}
		std::cout << std::endl;
	}
	//void LockTest(Node* node) {
	//	Node* left{ nullptr };
	//	Node* right{ nullptr };
	//	while (node != nullptr) {
	//		left = node->pLeft;
	//		LockTest(left);
	//		right = node->pRight;
	//		LockTest(left);
	//	}
	//}
};