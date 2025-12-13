#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>

// using namespace std;
// using namespace cv;

// 3D Vector 
using Vector3D = std::vector<std::vector<std::vector<uint8_t>>>;

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

int main(int argc, char* argv[]){
    // cv::Mat imagem;
    // imagem = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    // cv::namedWindow("Minha Imagem", cv::WINDOW_NORMAL);
    // cv::resizeWindow("Minha Imagem", 600,600);
    // imshow("Minha Imagem", imagem);
    // cv::waitKey(0);

    // imagem = cv::imread(argv[1], cv::IMREAD_COLOR);
    // cv::namedWindow("Minha Imagem", cv::WINDOW_NORMAL);
    // cv::resizeWindow("Minha Imagem", 600,600);
    // imshow("Minha Imagem", imagem);
    // cv::waitKey(0); 

    cv::Mat image = readImageMat(argv[1]);
    Vector3D imagemVector = matToVector3D(image);

    // Teste de verificação: Imprimir o pixel central
    int meioY = image.rows / 2;
    int meioX = image.cols / 2;

    std::cout << "Dimensoes: " << imagemVector.size() << "x" 
              << imagemVector[0].size() << "x" 
              << imagemVector[0][0].size() << std::endl;

    // Lembra-te: OpenCV carrega em BGR, não RGB.
    // Índice 0 = Blue, 1 = Green, 2 = Red
    std::cout << "Pixel Central (RGB): [" 
              << (int)imagemVector[meioY][meioX][0] << ", "
              << (int)imagemVector[meioY][meioX][1] << ", "
              << (int)imagemVector[meioY][meioX][2] << "]" << std::endl;

    return 0;
}

