#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "Geometry.hpp"
#include "constants.h"

namespace {
using namespace std;
using glm::mat3;
using glm::mat4;
using glm::vec3;

class Arrow : public Geometry{
    private:
        const float body_ratio = 0.85f;
        const float radius_ratio = 0.85f;
    public:
    Arrow(float radius, float length){
        Cone arrow(0.2f,1-body_ratio);
        Cylinder body(radius_ratio*radius, body_ratio*length);
        

    }
};

// class Arrow {
// private:
//   const uint32_t uNum = 1;
//   const uint32_t vNum = 36;
//   const float radius = 0.2f;
//   const float height = 1.0f;

// public:
//   vector<vec3> vertices;
//   vector<uint32_t> surfaces;

//   Arrow(vec3 rot_axis, float rot_angle) {
//     // 旋转矩阵
//     mat3 rot_mat = glm::mat4(glm::rotate(rot_angle, glm::normalize(rot_axis)));

//     // 生成箭头网格
//     const float body_ratio = 0.85f; // 箭身的占比

//     vector<vec3> positions_body((uNum + 1) * (vNum + 1)); // 箭头仅需要坐标即可
//     vector<uint32_t> indices_body(uNum * vNum * 6);

//     // 箭身不考虑转弯，仅需圆周细分即可
//     for (uint32_t i = 0; i <= uNum; i++) {
//       for (uint32_t j = 0; j <= vNum; j++) {
//         vec3 pos = {radius * body_ratio * cos(static_cast<float>(j) / vNum),
//                     radius * body_ratio * sin(static_cast<float>(j) / vNum),
//                     height * body_ratio * static_cast<float>(i) / uNum};
//         positions_body[j + i * (vNum + 1)] = rot_mat * pos;
//         if (i != uNum && j != vNum) {
//           uint32_t idx = (j + i * vNum) * 6;
//           indices_body[idx + 0] = j + i * (vNum + 1);
//           indices_body[idx + 1] = 1 + j + i * (vNum + 1);
//           indices_body[idx + 2] = 1 + j + (i + 1) * (vNum + 1);
//           indices_body[idx + 3] = j + i * (vNum + 1);
//           indices_body[idx + 4] = 1 + j + (i + 1) * (vNum + 1);
//           indices_body[idx + 5] = j + (i + 1) * (vNum + 1);
//           printf("i=%d, j=%d, idx=%d", i, j, idx);
//         }
//       }
//     }

//     // 箭头头部
//     uint32_t offset = positions_body.size();
//     vector<vec3> positions_arrow((uNum + 1) * (vNum + 1));
//     vector<uint32_t> indices_arrow(uNum * vNum * 6);

//     for (uint32_t i = 0; i <= uNum; i++) {
//       for (uint32_t j = 0; j <= vNum; j++) {
//         float u = static_cast<float>(i) / uNum;
//         float v = static_cast<float>(j) / vNum;
//         vec3 pos = {radius * (1 - v) * body_ratio * cos(2 * PI * v),
//                     radius * (1 - v) * body_ratio * sin(2 * PI * v),
//                     height * (body_ratio * (u - 1) + 1)};
//         positions_arrow[j + i * (vNum + 1)] = rot_mat * pos;
//         if (i != uNum && j != vNum) {
//           uint32_t idx = (j + i * vNum) * 6;
//           indices_arrow[idx + 0] = offset + j + i * (vNum + 1);
//           indices_arrow[idx + 1] = offset + 1 + j + i * (vNum + 1);
//           indices_arrow[idx + 2] = offset + 1 + j + (i + 1) * (vNum + 1);
//           indices_arrow[idx + 3] = offset + j + i * (vNum + 1);
//           indices_arrow[idx + 4] = offset + 1 + j + (i + 1) * (vNum + 1);
//           indices_arrow[idx + 5] = offset + j + (i + 1) * (vNum + 1);
//         }
//       }
//     }
//     // 拼接
//     this->vertices.insert(this->vertices.end(), positions_body.begin(),
//                           positions_body.end());
//     this->vertices.insert(this->vertices.end(), positions_arrow.begin(),
//                           positions_arrow.end());
//     this->surfaces.insert(this->surfaces.begin(), indices_body.begin(),
//                           indices_body.end());
//     this->surfaces.insert(this->surfaces.begin(), indices_arrow.begin(),
//                           indices_arrow.end());
//   }
// };
} // namespace
