#pragma once
namespace {
float m_clamp(const float &val, const float &min_v, const float &max_v) {
  if (val < min_v)
    return min_v;
  else if (val > max_v)
    return max_v;
  else
    return val;
  ;
}
} // namespace
