#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "vectorOps.hpp"

std::string averageHash(const cv::Mat& image, int hashSize=8, std::string method="mean") {
    cv::Mat resizeImage; 
    cv::resize(image, resizeImage, cv::Size(hashSize, hashSize), 0.0, 0.0, cv::INTER_LANCZOS4);



    cv::Mat grayscale; 

    cv::cvtColor(resizeImage, grayscale, cv::COLOR_RGB2GRAY);



    
    int height = grayscale.rows;
    int width = grayscale.cols;
    int channels = grayscale.channels();
    std::cout << "Height: " << height << ", Width: " << width << ", Channels: " << channels << std::endl;

    std::cout << "Height: " << image.rows << ", Width: " << image.cols << ", Channels: " << image.channels() << std::endl;

    return "Suck it, nerd";
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

    // cv::Mat image = readImageMat(argv[1]);
    // Vector3D imagemVector = matToVector3D(image);

    cv::Mat image = readImageMat("data/0a.jpg");
    averageHash(image);


    return 0;
}

