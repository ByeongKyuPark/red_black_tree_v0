#include "redBlackTree.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <cassert>
constexpr int Size = 50;
int Num_Threads = 2;
constexpr int Num_Test_Itrs = 5;

void GenerateUniqueRandomValues(int* array, int size, int seed) {
	std::mt19937 rand(seed);
	std::uniform_int_distribution<> dist(0, size*3);
	std::set<int> set;
	for (int i = 0; i < size;) {
		int val = dist(rand);
		if (set.find(val) == set.end()) {
			set.insert(val);
			array[i++] = val;
		}
	}
}

void InsertRandom(RedBlackTree& tree, int chunkStart, int chunkEnd,const int* values) {
	for (int i{chunkStart}; i <chunkEnd; ++i) {
		tree.Add(values[i]);
	}
}

void Insert_Multi_Threads() {
	std::cout << "==================== Insert ====================" << std::endl;
	for (int i{}; i < Num_Test_Itrs; i++) {
		int seed = i + 1;

		int values[Size];
		GenerateUniqueRandomValues(values, Size, seed);

		RedBlackTree tree;

		std::vector<std::thread> threads;

		int chunkSize = Size / Num_Threads;
		auto startMulti = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < Num_Threads; ++i) {
			if (i == Num_Threads - 1) {
				threads.push_back(std::thread(InsertRandom, std::ref(tree), chunkSize * i, std::max(chunkSize * (i + 1),Size), values));
			}
			else {
				threads.push_back(std::thread(InsertRandom, std::ref(tree), chunkSize*i, chunkSize*(i+1), values));
			}
		}
		for (auto& th : threads) {
			th.join();
		}
		auto endMulti = std::chrono::high_resolution_clock::now();

		//checks
		for (int i{}; i < Size; ++i) {
			assert(tree.Search(values[i]));
		}

		std::chrono::duration<double> elapsedMulti = endMulti - startMulti;

		std::cout << "[Seed = " << seed << "]\n";
		std::cout << "  Multi-threaded took " << elapsedMulti.count() << " seconds" << std::endl;
	}
}

//Searches for random values in the tree using multiple threads and asserts if they are present
void SearchRandomValuesMultiThreaded(RedBlackTree& tree, int chunkStart, int chunkEnd, const int* values) {
	for (int i{chunkStart}; i < chunkEnd; ++i) {
		assert(tree.Search(values[i]));
	}
}
void MultiThreaded_Contains_Random() {
	std::cout << "==================== " << "Contains Test" << " ====================" << std::endl;
	for (int i{}; i < Num_Test_Itrs; i++) {
		int values[Size];
		int seed = i + 1;
		GenerateUniqueRandomValues(values, Size, seed);

		RedBlackTree tree;
		for (int j{}; j < Size; ++j) {
			tree.Add(values[j]);
		}

		std::vector<std::thread> threads;

		auto startMulti = std::chrono::high_resolution_clock::now();
		const int chunkSize = Size / Num_Threads;
		for (int i = 0; i < Num_Threads; ++i) {
			if (i == Num_Threads - 1) {
				threads.push_back(std::thread(SearchRandomValuesMultiThreaded, std::ref(tree), chunkSize * i, std::max(chunkSize * (i + 1), Size), values));
			}
			else {
				threads.push_back(std::thread(SearchRandomValuesMultiThreaded, std::ref(tree), chunkSize * i, chunkSize * (i + 1), values));
			}

		}


		for (auto& th : threads) {
			th.join();
		}
		auto endMulti = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsedMulti = endMulti - startMulti;

		std::cout << "[Seed = " << seed << "]\n";
		std::cout << "  Multi-threaded took " << elapsedMulti.count() << " seconds" << std::endl;
	}
}

int main() {

	//Search_Basic();
	//Search_Random_Seed1();
	//Search_Random_Seed2();
	//Search_Random_Seed3();
	//Insert_Basic();

	//Insert_Multi_Threads();
	//Search_Multi_Threads();
	std::cout << "# threads = " << Num_Threads << '\n';

	void(*tests[])() = {
		//SIGNLE_THREAD
			////insert
			//Insert_Basic,
			//Insert_Random_Seed1,
			//Insert_Random_Seed2,
			//Insert_Random_Seed3,
			////search
			//Search_Basic,
			//Search_Random_Seed1,
			//Search_Random_Seed2,
			//Search_Random_Seed3,
		//MULTI_THREADS
			//insert
			Insert_Multi_Threads, //TODO
			MultiThreaded_Contains_Random
			//search
			//Search_Multi_Threads,  //TODO
			//Search_Insert_Mixed_Multi_Threads
		};

	
		//DELETION
		//Ordered_Delete_Seed1,
		//UnOrdered_Delete_Seed1,

		//Ordered_Delete_Seed2,
		//UnOrdered_Delete_Seed2,

		//Ordered_Delete_Seed3,
		//UnOrdered_Delete_Seed3,

		//Ordered_Delete_Seed4,
		//UnOrdered_Delete_Seed4,

		//Ordered_Delete_Seed5,
		//UnOrdered_Delete_Seed5,

		////INSERTION(string)
		//Insert_String,

		////DELETION(string)
		//Delete_String,
		//Delete_String_Ordered,

		////DELETING EXCEPTION
		//Delete_None_Existing,

		////DUPLICATE
		//Insert_Duplicate,
		//Delete_Duplicate,
	//};
	int numTests = static_cast<int>(sizeof(tests) / sizeof(*tests));
	for (int i = 0; i < numTests; i++) {
		tests[i]();
		std::cout << std::endl << std::endl;
	}

	return 0;
}