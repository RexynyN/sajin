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
using Vector1D = std::vector<uint8_t>;

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


// mean()
double meanVector1D(const Vector1D& vec) {
    double sum = 0;
    for (size_t i = 0; i < vec.size(); i++){
        sum += vec[i];
    }
    
    return sum / vec.size();
}

double meanVector2D(const Vector2D& vec) {
    size_t rows = vec.size(), columns = vec[0].size();
    double sum = 0;

    for (size_t i = 0; i < rows; i++){
        for (size_t j = 0; j < columns; j++) {
            sum += vec[i][j];
        }
    }
    
    return sum / (columns * rows);
}

double meanVector3D(const Vector3D& vec) {
    size_t rows = vec.size(), columns = vec[0].size();
    size_t channels = vec[0][0].size();
    double sum = 0;

    for (size_t i = 0; i < rows; i++){
        for (size_t j = 0; j < columns; j++) {
            for (size_t k = 0; k < channels; k++) {
                sum += vec[i][j][k];
            }
        }
    }
    
    return sum / (columns * rows * channels);
}

// median()
double medianVector1D(const Vector1D& flatVec) {
    std::sort(flatVec.begin(), flatVec.end());  

    size_t len = flatVec.size(); 
    if (flatVec.size() % 2 != 0) { // Odd number of items
        return flatVec[floor(len / 2)]; 
    }

    size_t midpoint = (size_t) floor(len / 2); // Mean of the two central values if even
    return (flatVec[midpoint] + flatVec[midpoint - 1]);
}


double medianVector2D(const Vector2D& vec) {
    return medianVector1D(flattenVector2D(vec));
}

double medianVector3D(const Vector3D& vec) {
    return medianVector1D(flattenVector3D(vec));
}


// flatten()
Vector1D flattenVector2D(const Vector2D& vec) {
    size_t rows = vec.size(), columns = vec[0].size();
    Vector1D flattened(rows * columns);

    for(size_t i = 0; i < rows; i++){
        for (size_t j = 0; j < columns; j++) {
            flattened[i + j] = vec[i][j];
        }
    }
    return flattened; 
}

Vector1D flattenVector3D(const Vector3D& vec) {
    size_t rows = vec.size(), columns = vec[0].size();
    size_t channels = vec[0][0].size();
    Vector1D flattened(rows * columns);

    for (size_t i = 0; i < rows; i++){
        for (size_t j = 0; j < columns; j++) {
            for (size_t k = 0; k < channels; k++) {
                flattened[i + j + k] = vec[i][j][k];
            }
        }
    }

    return flattened; 
}


// log2()

// transpose() 

// invert()

// full(shape, fill_value) -> Return a new array of given shape and type, filled with fill_value.
Vector1D fullVector1D(size_t columns, uint8_t fillValue) {
    Vector1D vec(columns);
    for(size_t i = 0; i < columns; i++)
        vec[i] = fillValue;

    return vec;
}

Vector2D fullVector2D(size_t rows, size_t cols, uint8_t fillValue) {
    Vector2D vec(rows, std::vector<uint8_t>(cols));
    for(size_t i = 0; i < rows; i++){
        for(size_t j = 0; j < cols; j++){
            vec[i][j] = fillValue;
        }
    }

    return vec;
}

Vector3D fullVector3D(size_t rows, size_t cols, size_t channels, uint8_t fillValue) {
    Vector3D vec(rows, std::vector<std::vector<uint8_t>>(cols, std::vector<uint8_t>(channels)));
    for(size_t i = 0; i < rows; i++){
        for(size_t j = 0; j < cols; j++){
            for(size_t k = 0; k < channels; k++){
                vec[i][j][k] = fillValue;
            }
        }
    }

    return vec;
}

// zeros(shape) -> create an array of shape filled with zeros
Vector1D zeroesVector1D(size_t columns) {
    return fullVector1D(columns, 0);
}

Vector2D zeroesVector2D(size_t rows, size_t cols) {
    return fullVector2D(rows, cols, 0);
}

Vector3D zeroesVector3D(size_t rows, size_t cols, size_t channels) {
    return fullVector3D(rows, cols, channels, 0);
}

// nonzero() -> Return the indices of the elements that are non-zero.

// reshape(shape) -> reshape the tensor to shape



// logical_and() -> logical and operator

// bitwise_and() -> bitwise and

// count_nonzero() -> counts how many non-zeros the tensor has

// array_equal() -> True if two arrays have the same shape and elements, False otherwise.

// dct() -> Return the Discrete Cosine Transform of arbitrary type sequence x.

// linspace() -> Return evenly spaced numbers over a specified interval.

// histogram(data, bins) 