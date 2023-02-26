//
// Created by 刘洋 on 2023/2/23.
//

#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <vector>
#include <valarray>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>

template<typename K, typename V>
class KVNode {
 public:
  KVNode() = default;
  KVNode(K key, V value) {
	  this->key = key, this->value = value;
  }
  K getKey() const { return key; }
  V getValue() const { return value; }
  void setKey(K k) { key = k; }
  void setValue(V v) { value = v; }
 private:
  K key;
  V value;
};

template<typename K, typename V>
class QuadNode {
 public:
  QuadNode() = default;
  QuadNode(K key, V value);
  QuadNode(K key, V value, int level);
  QuadNode<K, V> *right() { return rightNode; }
  QuadNode<K, V> *left() { return leftNode; }
  QuadNode<K, V> *downStairs() {
	  return lowerNode;
  }
  QuadNode<K, V> *toLevel(int curLevel, int tarLevel = 0) {
	  QuadNode<K, V> *tmp = this;
	  while (tmp && (curLevel - tarLevel)) {
		  tmp = tmp->lowerNode;
		  --curLevel;
	  }
	  return tmp;
  }
  void createLink(QuadNode<K, V> *left, QuadNode<K, V> *right);
  void createLink(QuadNode<K, V> *lower);
  void deleteLink(QuadNode<K, V> *left, QuadNode<K, V> *right);

  void setGuarder(bool g = true) { guarder = g; }
  bool isGuarder() const { return guarder; }
  K getKey() const { return node.getKey(); }
  V getValue() const { return node.getValue(); }
  int getLevel() const { return level; }

 private:
  //info
  KVNode<K, V> node;
  QuadNode<K, V> *upperNode = nullptr, *lowerNode = nullptr, *leftNode = nullptr, *rightNode = nullptr;
  bool guarder = false;
  int level = 0;
};

template<typename K, typename V>
class QuadNodeList {
 public:
  QuadNodeList();
  QuadNode<K, V> *start() { return head; }
  QuadNode<K, V> *end() { return tail; }

 private:
  QuadNode<K, V> *head, *tail;
};

// SkipList
template<typename K, typename V>
class SkipList {
 public:
  explicit SkipList(int maxHeight = 0, double p = 0.5);
  V search(K key);
  V search(K key, int &times);
  bool insert(K key, V value);
  void deleteNode(K key);
  int getCurLevel() const { return curMaxLevel; }
  int getMaxLevel() const { return maxLevel; }
  int getDataSize() const { return dataSize; }

 private:
  std::vector<QuadNodeList<K, V>> vector_;
  double p = 0.5;
  int maxLevel = 1, dataSize = 0;
  int curMaxLevel = 0;
  bool hasSetHeight = false;

  QuadNode<K, V> *searchUtil(K key, int &times, int bottomLevel = 0);
  void higher(K key, V value, int tarLevel, QuadNode<K, V> *&lowerNode);
  bool shouldEmplaceList(int tarLevel) {
	  if (tarLevel > curMaxLevel && tarLevel <= maxLevel) {
		  curMaxLevel = tarLevel;
		  return true;
	  }
	  return false;
  }
  bool canGetHigher(int curLevel) {
	  if (curLevel >= maxLevel) return false;
	  auto r = (rand() % 1000) / 1000.0;
	  if (r < p) return true;
	  return false;
  }
  void update() {
	  ++dataSize;
	  maxLevel = (int) (log(dataSize) / log(1 / p) + 1 / (1 - p));
  }
};

// QuadNode
template<typename K, typename V>
QuadNode<K, V>::QuadNode(K key, V value) {
	node.setKey(key), node.setValue(value);
}

template<typename K, typename V>
QuadNode<K, V>::QuadNode(K key, V value, int level) {
	node.setKey(key), node.setValue(value);
	this->level = level;
}

template<typename K, typename V>
void QuadNode<K, V>::createLink(QuadNode<K, V> *left, QuadNode<K, V> *right) {
	if (left) {
		left->rightNode = this;
		this->leftNode = left;
	}
	if (right) {
		this->rightNode = right;
		right->leftNode = this;
	}
}

template<typename K, typename V>
void QuadNode<K, V>::createLink(QuadNode<K, V> *lower) {
	this->lowerNode = lower;
	lower->upperNode = this;
}
template<typename K, typename V>
void QuadNode<K, V>::deleteLink(QuadNode<K, V> *left, QuadNode<K, V> *right) {
	left->rightNode = right;
	right->leftNode = left;
}

// QuadNodeList
template<typename K, typename V>
QuadNodeList<K, V>::QuadNodeList() {
	head = new QuadNode<K, V>;
	tail = new QuadNode<K, V>;
	head->setGuarder(), tail->setGuarder();
	tail->createLink(head, nullptr);
}

// SkipList
/// maxHeight : the maxHeight, default mode is growing dynamically
/// p : probability of growing higher, default probability is 0.5
template<typename K, typename V>
SkipList<K, V>::SkipList(int maxHeight, double p) {
	this->p = p;
//	maxLevel = size ? (int) (log(size) / log(1 / p) + 1 / (1 - p)) : (int) (1 + 1 / (1 - p));
	if (maxHeight) {
		maxLevel = maxHeight;
		hasSetHeight = true;
	} else maxLevel = (int) (1 + 1 / (1 - p));
	std::srand(time(nullptr));
	vector_.template emplace_back(QuadNodeList<K, V>());
}

/// insert a node, if it success, return true
template<typename K, typename V>
bool SkipList<K, V>::insert(K key, V value) {
	int times = 0;
	QuadNode<K, V> *quad_node = searchUtil(key, times);
	quad_node->toLevel(quad_node->getLevel(), 0);
	// already had the node or something gets wrong, return false
	if (!quad_node || quad_node == vector_[0].end()) return false;
	if (quad_node->getKey() == key) return false;
	// create a new node
	int curLevel = 0;
	auto *nNode = new QuadNode<K, V>(key, value, 0);
	QuadNode<K, V> *opNode = nNode;
	nNode->createLink(quad_node, quad_node->right());
	if (!hasSetHeight) update(); // update related data
	// judge whether to add a higher level
	while (canGetHigher(curLevel)) {
		higher(key, value, curLevel + 1, opNode);
		curLevel++;
	}
	return true;
}

/// if the key exists, return the value, if not, throw an error
template<typename K, typename V>
V SkipList<K, V>::search(K key) {
	int times = 0;
	return search(key, times);
}
/// times can record the steps to find the key
template<typename K, typename V>
V SkipList<K, V>::search(K key, int &times) {
	QuadNode<K, V> *quad_node = searchUtil(key, times);
	if (!quad_node || quad_node->getKey() != key) {
		throw std::invalid_argument("the key doesn't exist!");
	} else {
		return quad_node->getValue();
	}
}

template<typename K, typename V>
QuadNode<K, V> *SkipList<K, V>::searchUtil(K key, int &times, int bottomLevel) {
	int curLevel = vector_.size() - 1;
	QuadNodeList<K, V> *curVec = &vector_[curLevel];
	QuadNode<K, V> *curNode = curVec->start();
	while (curLevel >= bottomLevel) {
		curVec = &vector_[curLevel];
		++times;
		if (curNode->right() == curVec->end()) {
			// meet the guarder, go downward or return
			if (curLevel == bottomLevel) return curNode;
			else {
				curNode = curNode->downStairs();
				curLevel--;
				continue;
			}
		} else if (curNode->right()->getKey() < key) {
			// go rightward
			curNode = curNode->right();
		} else if (curNode->right()->getKey() > key) {
			// go downward
			curNode = curNode->downStairs();
			curLevel--;
		} else if (curNode->right()->getKey() == key) {
			// find and return
			return curNode->right();
		}
	}
	return curNode;
}

template<typename K, typename V>
void SkipList<K, V>::higher(K key, V value, int tarLevel, QuadNode<K, V> *&lowerNode) {
	if (shouldEmplaceList(tarLevel)) {
		// emplace back a new nodeList, and create links
		auto newList = QuadNodeList<K, V>();
		newList.start()->createLink(vector_[tarLevel - 1].start());
		newList.end()->createLink(vector_[tarLevel - 1].end());
		vector_.template emplace_back(newList);
	}
	// insert the new node and create links
	int times = 0;
	auto *newNode = new QuadNode<K, V>(key, value, tarLevel);
	auto *leftNode = searchUtil(key, times, tarLevel);
	newNode->createLink(leftNode, leftNode->right());
	newNode->createLink(lowerNode);
	lowerNode = newNode;
}
template<typename K, typename V>
void SkipList<K, V>::deleteNode(K key) {
	int times = 0;
	auto delNode = searchUtil(key, times);
	if (!delNode || delNode->getKey() != key) return;
	else {
		while (delNode) {
			// link its left and right
			delNode->deleteLink(delNode->left(), delNode->right());
			// move downstairs
			auto t = delNode;
			delNode = delNode->downStairs();
			delete t;
		}
	}
}

#endif//SKIPLIST_SKIPLIST_H
