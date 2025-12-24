#ifndef VECTOROPS_HPP
#define VECTOROPS_HPP

#include <opencv2/core/core.hpp>
#include <vector>
#include <string>
#include <cstdint>

using Vector3D = std::vector<std::vector<std::vector<uint8_t>>>;
using Vector2D = std::vector<std::vector<uint8_t>>;
using Vector1D = std::vector<uint8_t>;

// Conversion functions
Vector3D matToVector3D(const cv::Mat& img);
cv::Mat vector3DToMat(const Vector3D& vec);

// I/O functions
cv::Mat readImageMat(const std::string path);
Vector3D readImageVector(const std::string path);

// Showcase
int setShowcase();

// Statistical functions
double meanVector1D(const Vector1D& vec);
double meanVector2D(const Vector2D& vec);
double meanVector3D(const Vector3D& vec);

double medianVector1D(const Vector1D& flatVec);
double medianVector2D(const Vector2D& vec);
double medianVector3D(const Vector3D& vec);

// Flattening functions
Vector1D flattenVector2D(const Vector2D& vec);
Vector1D flattenVector3D(const Vector3D& vec);

// Fill functions
Vector1D fullVector1D(size_t columns, uint8_t fillValue);
Vector2D fullVector2D(size_t rows, size_t cols, uint8_t fillValue);
Vector3D fullVector3D(size_t rows, size_t cols, size_t channels, uint8_t fillValue);

Vector1D zeroesVector1D(size_t columns);
Vector2D zeroesVector2D(size_t rows, size_t cols);
Vector3D zeroesVector3D(size_t rows, size_t cols, size_t channels);

#endif // VECTOROPS_HPP