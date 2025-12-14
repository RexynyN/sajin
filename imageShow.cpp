#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "vectorOps.hpp"

// using namespace std;
// using namespace cv;


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

