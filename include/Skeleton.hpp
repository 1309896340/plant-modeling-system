#pragma once

#include <cstdint>
#include <cstdio>
#include <deque>
// #include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Geometry.hpp"
#include "Transform.hpp"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

class SkNode {
    friend class Skeleton;

private:
    // 当前节点相对于上一个节点的位置姿态偏移
    Transform transform;
    Transform abs_transform;   // 当前节点的起始位置与姿态
public:
    vector<SkNode*> children;
    SkNode*         parent{nullptr};

    shared_ptr<Geometry> obj{nullptr};   // 绑定的几何体

    SkNode() = default;
    SkNode(vec3 position, float rot_angle, vec3 rot_axis) {
        this->transform.translate(position);
        this->transform.rotate(rot_angle, rot_axis);
    }

    void bindGeometry(shared_ptr<Geometry> obj) {
        // 绑定几何体对象用于可视化表示
        this->obj = obj;
    }

    void setAttitude(float angle, vec3 axis) {
        quat attitude = glm::angleAxis(angle, axis);
        this->transform.setAttitude(attitude);
    }

    void setAttitude(quat attitude) { this->transform.setAttitude(attitude); }

    void setPosition(vec3 position) { this->transform.setPosition(position); }

    void addChild(SkNode* child) {
        if (child == nullptr)
            throw invalid_argument("SkNode::addChild cannot pass nullptr!");
        // cout << this << " 加入" << child << endl;
        child->parent = this;
        this->children.push_back(child);
    }

    mat4 getAbsModel() const { return this->abs_transform.getModel(); }

    Transform getAbsTransform() const { return this->abs_transform; }
    Transform getTransform() const { return this->transform; }
    void      setTransform(const Transform& transform) { this->transform = transform; }
};

class Skeleton {
    // 目前没有任何添加节点的方法，添加节点的过程是完全暴露的
private:
public:
    SkNode* root{nullptr};
    Skeleton()
        : root(new SkNode()) {
            // 默认生成一个空的根节点
        };
    ~Skeleton() {
        if (this->root == nullptr)
            return;
        // 广度优先遍历释放
        deque<SkNode*> buf{this->root};
        while (!buf.empty()) {
            SkNode* cur = buf.front();
            buf.pop_front();
            buf.insert(buf.end(), cur->children.begin(), cur->children.end());
            delete cur;
        }
    }

    void update() {
        deque<SkNode*> buf{this->root};
        while (!buf.empty()) {
            SkNode* cur = buf.front();
            buf.pop_front();
            buf.insert(buf.end(), cur->children.begin(), cur->children.end());

            if (cur->parent == nullptr) {   // 根节点
                cur->abs_transform = cur->transform;
                continue;
            }

            cur->abs_transform = cur->transform * cur->parent->abs_transform;

            // vec3 ps = cur->abs_transform.getPosition();
            // printf("节点%p 起点位置：(%.4f, %.4f, %.4f)\n", cur, ps.x, ps.y, ps.z);
        }
    }

    uint32_t calcNodeSize() {
        uint32_t num = 0;
        this->traverse([&num](SkNode* node) { num++; });
        return num;
    }

    void traverse(function<void(SkNode*)> visit) {
        // 广度优先遍历
        deque<SkNode*> buf{this->root};
        while (!buf.empty()) {
            SkNode* cur = buf.front();
            buf.pop_front();
            if (cur != this->root)
                visit(cur);
            if (cur->children.size() > 0)
                buf.insert(buf.end(), cur->children.begin(), cur->children.end());
        }
    }
};

}   // namespace
