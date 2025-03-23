#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>
#include <mutex>
#include <stdexcept>

/**
 * @class ConsistentHash
 * @brief 实现一致性哈希算法的类。
 *
 * 一致性哈希是一种分布式哈希技术，旨在最小化在节点添加或移除时键的重新分配。
 * 常用于分布式缓存系统和分布式数据库分片等场景。
   通常以哈希环和虚拟节点为特征
 */
class ConsistentHash {
public:
    /**
     * @brief 构造函数
     * @param numReplicas 每个物理节点的虚拟节点数量，增加虚拟节点可改善负载均衡效果，越多负载越均衡。
     * @param hashFunc 可选的自定义哈希函数，默认为 std::hash。
		std::hash<std::string>：
			size_t hash = 0;
			for (char c : key) {
			hash = hash * 31 + c;  // 31 是一个常见的质数，用于减少碰撞
		size_t是一个unsigned long long 
}

     */
    ConsistentHash(size_t numReplicas, std::function<size_t(const std::string&)> hashFunc = std::hash<std::string>())
        : numReplicas_(numReplicas), hashFunction_(hashFunc) {}

    /**
     * @brief 向哈希环中添加一个存储节点。
     *
     * 每个节点会被复制为若干个虚拟节点。每个虚拟节点通过 `node + index` 计算出唯一的哈希值。
     * 这些哈希值存储在哈希环上，并进行排序以便高效查找。
     *
     * @param node 要添加的节点名称（如服务器地址）。
     */
    void addNode(const std::string& node, size_t index) {
        std::lock_guard<std::mutex> lock(mtx_); // 确保多线程安全
        for (size_t i = 0; i < numReplicas_; ++i) {
            // 为每个虚拟节点计算唯一哈希值
            size_t hash = hashFunction_(node +"_0"+std::to_string(i));
            circle_[hash] = index;         // 哈希值映射到外层loop下标
            sortedHashes_.push_back(hash); // 添加到排序列表
        }
        // 对哈希值进行排序
        std::sort(sortedHashes_.begin(), sortedHashes_.end());
    }

    /**
     * @brief 从哈希环中移除一个存储节点。
     *
     * 删除该节点的所有虚拟节点及其对应的哈希值。
     *
     * @param node 要移除的节点名称。
     */
    void removeNode(const std::string& node) {
        std::lock_guard<std::mutex> lock(mtx_); // 确保多线程安全
        for (size_t i = 0; i < numReplicas_; ++i) {
            // 计算虚拟节点的哈希值
            size_t hash = hashFunction_(node + std::to_string(i));
            circle_.erase(hash); // 从哈希环中删除该哈希
            auto it = std::find(sortedHashes_.begin(), sortedHashes_.end(), hash);
            if (it != sortedHashes_.end()) {
                sortedHashes_.erase(it); // 从排序列表中删除
            }
        }
    }

    /**
     * @brief 查找负责处理给定键的节点。定位存储节点，返回存储节点的下表
     *
     * 根据键的哈希值在哈希环中查找第一个大于等于该值的节点。
     * 如果没有找到（即超出哈希环最大值），则回绕到第一个节点。
     *
     * @param key 要查找的键（如数据的标识符）。
     * @return 负责处理该键的节点名称。
     * @throws std::runtime_error 如果哈希环为空（没有节点）。
     */

    size_t getNode(const std::string& key) {
		std::lock_guard<std::mutex> lock(mtx_); // 确保多线程安全
		if (circle_.empty()) {
			throw std::runtime_error("No nodes in consistent hash"); // 环为空时抛出异常
		}
		size_t hash = hashFunction_(key); // 计算键的哈希值
		// 在已排序的哈希列表中找到第一个大于键哈希值的位置
		auto it = std::upper_bound(sortedHashes_.begin(), sortedHashes_.end(), hash);
		if (it == sortedHashes_.end()) {
			// 如果超出环最大值，则回绕到第一个节点
			it = sortedHashes_.begin();
		}
		return circle_[*it]; // 哈希值对应的loop下标
    }

private:
    size_t numReplicas_; // 每个物理节点的虚拟节点数量
    std::function<size_t(const std::string&)> hashFunction_; // 用户自定义或默认的哈希函数
    std::unordered_map<size_t, size_t> circle_; // 哈希值到外层loop_下标的映射，用于返回外层loop_可使用的下标
    std::vector<size_t> sortedHashes_; // 排序的哈希值列表，用于高效查找
    std::mutex mtx_; // 保护哈希环的互斥锁，确保多线程安全
};
