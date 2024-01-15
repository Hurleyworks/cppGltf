#pragma once

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <queue>
#include <stack>
#include <fstream>
#include <set>
#include <vector>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <string>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include <limits>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include <any>
#include <filesystem>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <variant>
#include <future>
#include <semaphore>
#include <concepts>
#include <numbers>
#include <variant>

#ifdef __clang__
#include <experimental/coroutine>
#define COROUTINE_NAMESPACE std::experimental
#else
#include <coroutine>
#define COROUTINE_NAMESPACE std
#endif

// eigen math
#include <linalg/eigen34/Eigen/Dense>

using Eigen::Matrix;
using Pose = Eigen::Affine3f;
using Scale = Eigen::Vector3f;

using Float = float;
using MatrixXc = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXf = Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3u = Eigen::Matrix<uint32_t, 3, 1>;
using Vector4u = Eigen::Matrix<uint32_t, 4, 1>;
using Vector2u = Eigen::Matrix<uint32_t, 2, 1>;
// using Matrix3f = Eigen::Matrix<Float, 3, 3>;
using Matrix43f = Eigen::Matrix<Float, 4, 3>;
using MatrixRowMajor34f = Eigen::Matrix<Float, 3, 4, Eigen::RowMajor>;
using Matrix4f = Eigen::Matrix<Float, 4, 4>;
using MatrixXu16 = Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>;
using VectorXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, 1>;
using VectorXb = Eigen::Matrix<bool, Eigen::Dynamic, 1>;
using MatrixXi = Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic>;

using PathList = std::vector<std::filesystem::path>;

// g3log
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

// json
#include <json/json.hpp>
using nlohmann::json;

// less typing
namespace fs = std::filesystem;

#include "excludeFromBuild/basics/Util.h"

namespace mace
{
    // basics
    #include "excludeFromBuild/basics/StringUtil.h"

} // namespace mace
