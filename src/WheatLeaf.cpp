#include "Geometry.h"
#include "constants.h"

using namespace std;

float G(float m) {
  float tmp = sqrtf(1 + m * m);
  return logf(m + tmp) + m * tmp;
}

Vertex LeafMesh(float u, float v, LeafParameters params) {
  // 参考LeafMesh_H.m实现
  Vertex vt;

  // double Rx, Ry, Rz;
  glm::vec3 rPos;
  glm::vec3 rNorm;
  {
    // 顶点位置
    double b = 1.0 / tan(params.rho);
    double y0 = 2.0;
    for (uint32_t i = 0; i < 10; i++)
      y0 = y0 - (G(2 * params.a * y0 + b) - G(b) - 4 * params.a * params.H) /
                    (4 * params.a * sqrt(1 + pow(2 * params.a * y0 + b, 2.0)));
    double y = y0 * v;
    double tan_beta = 2 * params.a * y + b;
    double tan_beta_2 = tan_beta * tan_beta;
    double s = (G(tan_beta) - G(b)) / 4.0 / params.a / params.H;
    double w = 1 - pow((s - params.phi) / (1 - params.phi), 2.0);

    double Hx = (u - 0.5) * params.W * w;
    double Hy = y;
    double Hz = params.a * pow(y, 2.0) + b * y;

    double sTheta = params.theta0 + params.theta * s;
    // double sqr = sqrt(1 + tan_beta_2);
    double beta = atan(tan_beta);
    double cos_beta = cos(beta);
    double sin_beta = sin(beta);
    double sec_beta = 1.0 / cos_beta;

    rPos = glm::vec3(Hx * cos(sTheta), Hy + sin_beta * Hx * sin(sTheta),
                     Hz - cos_beta * Hx * sin(sTheta));
    // 顶点法方向
    double sTheta_v = y0 * params.theta / params.H * sec_beta;
    double dSinRhov_dv = 2 * params.a * y0 * pow(cos_beta, 3.0);
    double dCosRhov_dv = -2 * params.a * y0 * pow(cos_beta, 2.0)*sin_beta;
    double Hx_v = params.W * (1 - 2 * u) * (s - params.phi) * y0 /
                  (params.H * pow(1 - params.phi, 2.0) * cos_beta);
    double Hy_v = y0;
    double Hz_v = 2 * params.a * y0 * y0 * v + b * y0;
    glm::vec3 dr_du(cos(sTheta), sin(sTheta) * sin(beta),
                    -sin(sTheta) * cos(beta));
    dr_du = glm::normalize(dr_du);

    glm::vec3 dr_dv(Hx_v * cos(sTheta) - Hx * sTheta_v * sin(sTheta),
                    Hy_v + Hx_v * sin(sTheta) * sin(beta) +
                        Hx * sTheta_v * cos(sTheta) * sin(beta) +
                        Hx * sin(sTheta) * dSinRhov_dv,
                    Hz_v - Hx_v * sin(sTheta) * cos(beta) -
                        Hx * sTheta_v * cos(sTheta) * cos(beta) -
                        Hx * sin(sTheta) * dCosRhov_dv);
    dr_dv = glm::normalize(dr_dv);

     rNorm = glm::cross(dr_du, dr_dv);
  }

  vt.x = rPos.x;
  vt.y = rPos.z;
  vt.z = rPos.y;

  vt.nx = rNorm.x;
  vt.ny = rNorm.z;
  vt.nz = rNorm.y;

  vt.color[0] = 0.0f;
  vt.color[1] = 1.0f;
  vt.color[2] = 0.0f;

  vt.u = u;
  vt.v = v;

  return vt;
}

shared_ptr<Mesh> Mesh::Leaf(LeafParameters params, uint32_t uNum,
                            uint32_t vNum) {
  auto mesh = make_shared<Mesh>(uNum, vNum);
  auto updater = [mesh](float u, float v) {
    float W = std::get<float>(mesh->parameters["W"]->getProp());
    float H = std::get<float>(mesh->parameters["H"]->getProp());
    float phi = std::get<float>(mesh->parameters["phi"]->getProp());
    float a = std::get<float>(mesh->parameters["a"]->getProp());
    float rho = std::get<float>(mesh->parameters["rho"]->getProp());
    float theta = std::get<float>(mesh->parameters["theta"]->getProp());
    float theta0 = std::get<float>(mesh->parameters["theta0"]->getProp());
    return LeafMesh(u, v, LeafParameters{W, H, phi, a, rho, theta, theta0});
  };
  mesh->setUpdater(updater);
  auto W_val = make_shared<ReflectValue>("W", params.W, EPS, 10);
  auto H_val = make_shared<ReflectValue>("H", params.H, EPS, FLT_MAX);
  auto phi_val = make_shared<ReflectValue>("phi", params.phi, EPS, 0.5);
  auto a_val = make_shared<ReflectValue>("a", params.a, -2,-0.001);
  auto rho_val = make_shared<ReflectValue>("rho", params.rho, 0.1, PI-0.1);
  auto theta_val = make_shared<ReflectValue>("theta", params.theta, -4*PI ,4*PI);
  auto theta0_val = make_shared<ReflectValue>("theta0", params.theta0, -PI, PI);

  W_val->addObserver(mesh);
  H_val->addObserver(mesh);
  phi_val->addObserver(mesh);
  a_val->addObserver(mesh);
  rho_val->addObserver(mesh);
  theta_val->addObserver(mesh);
  theta0_val->addObserver(mesh);

  mesh->parameters["W"] = W_val;
  mesh->parameters["H"] = H_val;
  mesh->parameters["phi"] = phi_val;
  mesh->parameters["a"] = a_val;
  mesh->parameters["rho"] = rho_val;
  mesh->parameters["theta"] = theta_val;
  mesh->parameters["theta0"] = theta0_val;
  mesh->parameters["uNum"]->addObserver(mesh);
  mesh->parameters["vNum"]->addObserver(mesh);
  // 进行一次初始的更新
  mesh->update();
  return mesh;
}

// shared_ptr<Mesh> Mesh::Leaf(LeafParameters params, uint32_t VNum,
//                             uint32_t HNum) {

// }
