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



} // namespace mace
