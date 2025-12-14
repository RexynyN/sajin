#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <set>
#include <string>

// 3D Vector 
using Vector3D = std::vector<std::vector<std::vector<uint8_t>>>;
using Vector2D = std::vector<std::vector<uint8_t>>;

// RGB-Mat to Vector3D
Vector3D matToVector3D(const cv::Mat& img) {
    if (img.empty()) {
        std::cerr << "ERROR: Empty Image!" << std::endl;
        return {};
    }

    int rows = img.rows;
    int cols = img.cols;
    int channels = img.channels();

    // Estrutura: [Altura][Largura][Canal]
    Vector3D vec(rows, std::vector<std::vector<uint8_t>>(cols, std::vector<uint8_t>(channels)));

    for (int i = 0; i < rows; ++i) {
        // Obtém o ponteiro para o início da linha i
        const uint8_t* rowPtr = img.ptr<uint8_t>(i);
        for (int j = 0; j < cols; ++j) {
            for (int c = 0; c < channels; ++c) {
                // A memória no OpenCV é intercalada: B G R B G R ...
                // O índice linear é: (coluna * numero_canais) + canal_atual
                vec[i][j][c] = rowPtr[j * channels + c];
            }
        }
    }

    return vec;
}

// Vector3D to RGB-Mat
cv::Mat vector3DToMat(const Vector3D& vec) {
    if (vec.empty() || vec[0].empty() || vec[0][0].empty()) 
        return cv::Mat();

    int rows = vec.size();
    int cols = vec[0].size();
    int channels = 3; 
    cv::Mat img(rows, cols, CV_8UC3);

    for (int i = 0; i < rows; ++i) {
        uint8_t* rowPtr = img.ptr<uint8_t>(i);
        for (int j = 0; j < cols; ++j) {
            // OpenCV Channel 0 (Blue) -> Vector Channel 2 (Blue)
            rowPtr[j * 3 + 0] = vec[i][j][2]; 
            // OpenCV Channel 1 (Green) -> Vector Channel 1 (Green)
            rowPtr[j * 3 + 1] = vec[i][j][1]; 
            // OpenCV Channel 2 (Red) -> Vector Channel 0 (Red)
            rowPtr[j * 3 + 2] = vec[i][j][0]; 
        }
    }

    return img;
}

cv::Mat readImageMat(const std::string path) {
    cv::Mat image = cv::imread(path, cv::COLOR_BGR2RGB);
    if (image.empty()) {
        std::cerr << "ERROR: Empty Image!" << std::endl;
        return {};
    }

    return image;
}

Vector3D readImageVector(const std::string path) {
    cv::Mat image = cv::imread(path, cv::COLOR_BGR2RGB);
    if (image.empty()) {
        std::cerr << "ERROR: Empty Image!" << std::endl;
        return {};
    }

    return matToVector3D(image);
}


int setShowcase() {
    std::set<std::string> unique_cars = {"Volvo", "BMW", "Ford", "BMW", "Mazda"};
    unique_cars.insert("Tesla");
    unique_cars.insert("VW");

    std::cout << "Unique cars in sorted order:" << std::endl;
    for (const std::string& car : unique_cars) {
        std::cout << car << std::endl;
    }

    if (unique_cars.count("Ford")) {
        std::cout << "\nFord is in the set." << std::endl;
    }

    unique_cars.erase("Volvo");

    std::cout << "\nAfter removing Volvo, size is: " << unique_cars.size() << std::endl;
    
    return 0;
}

// sqrt()

// ceil()

// mean()

double mean(Vector2D vec) {

    size_t rows = vec.size();
    size_t columns = vec[0].size();
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t i = 0; i < columns; i++)
        
    }
    
}

// median()

// flatten()

// log2()

// transpose() 

// invert()

// full(shape, fill_value) -> Return a new array of given shape and type, filled with fill_value.

// nonzero() -> Return the indices of the elements that are non-zero.

// reshape(shape) -> reshape the tensor to shape

// zeros(shape) -> create an array of shape filled with zeros

// logical_and() -> logical and operator

// bitwise_and() -> bitwise and

// count_nonzero() -> counts how many non-zeros the tensor has

// array_equal() -> True if two arrays have the same shape and elements, False otherwise.

// dct() -> Return the Discrete Cosine Transform of arbitrary type sequence x.

// linspace() -> Return evenly spaced numbers over a specified interval.

// histogram(data, bins) 