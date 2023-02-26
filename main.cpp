#include <iostream>
#include <fstream>
#include <cmath>
#include "SkipList.h"

double cal(int i, double j);

int main() {
	srand(time(nullptr));
	int len[6] = {50, 100, 200, 500, 1000, 2000};
	double pro[4] = {0.5, 1 / exp(1), 0.25, 0.125};
	std::ofstream ofstream;
	ofstream.open("../res.txt", std::ios::out);
	ofstream.clear();
	for (int i : len) {
		for (double j : pro) {
			SkipList<int, int> skip_list(48, j);
			for (int k = 0; k < i; ++k) {
//				if (!skip_list.insert(k, k)) exit(1);
				if (k & 1)
					skip_list.insert(k, k);
				else
					skip_list.insert(k + 1, k + 1);
			}
//			ofstream << "length : " << i << " p : " << j
//					 << " height : " << skip_list.getCurLevel()
//					 << " " << skip_list.getMaxLevel();
			int times = 0;
			for (int k = 0; k < 10; ++k) {
				int key = 2 * k + 1;
				try {
					auto f = skip_list.search(key);
					skip_list.deleteNode(key);
					// delete successfully, throw an error
					f = skip_list.search(key);
				} catch (std::invalid_argument &e) {
					std::cerr << key << " " << e.what() << std::endl;
					ofstream.close();
					return -1;
				}
			}
//			ofstream << "\ncost : " << (int) (times / 10000) << " times\n";
//			ofstream << (int) (times / 10000) << " ";
		}
		ofstream << "\n";
	}
	ofstream.close();
	ofstream.open("../cal.txt", std::ios::in);

	for (int i : len) {
		for (double j : pro) {
			ofstream << cal(i, j) << " ";
		}
		ofstream << "\n";
	}
	return 0;
}

double cal(int i, double p) {
	return (double) ((log(i) / log(1 / p) - 1) / p);
}
