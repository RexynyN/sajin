#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <functional>
#include <iostream>
#include <stdexcept> 
#include <vector>

#include "vectorOps.hpp"

std::string averageHash(const cv::Mat& image, int hashSize=8, std::function<double(Vector2D)> meanFunc=meanVector2D) {
    if(hashSize < 2) 
        throw std::invalid_argument("The hash size must be >= 2");

    cv::Mat grayscale; 
    cv::cvtColor(image, grayscale, cv::COLOR_RGB2GRAY);

    cv::Mat resizeImage; 
    cv::resize(grayscale, resizeImage, cv::Size(hashSize, hashSize), 0.0, 0.0, cv::INTER_LANCZOS4);

    int height = grayscale.rows;
    int width = grayscale.cols;
    int channels = grayscale.channels();
    std::cout << "Height: " << height << ", Width: " << width << ", Channels: " << channels << std::endl;

    std::cout << "Height: " << resizeImage.rows << ", Width: " << resizeImage.cols << ", Channels: " << resizeImage.channels() << std::endl;


    cv::imshow("Pringles", resizeImage);
    int a = cv::waitKey(20);

    Vector2D pixels = matGSToVector2D(resizeImage);
    for (size_t i = 0; i < hashSize; i++) {
        std::cout << "[";  
        for (size_t j = 0; j < hashSize; j++) {
            std::cout << (int)(pixels[i][j]) << ","; 
        }
        std::cout << "]" << std::endl;  
    }
    
    double meanFuncReturn = meanFunc(pixels); // Average of pixels in the resized image

    std::vector<std::vector<bool>> diff (hashSize, std::vector<bool>(hashSize)); 
    std::cout << "[";  
    // diff = pixels > avg
    for (size_t i = 0; i < hashSize; i++) {
        std::cout << "[";  
        for (size_t j = 0; j < hashSize; j++) {
            diff[i][j] = pixels[i][j] > meanFuncReturn;
            std::cout << diff[i][j] << ","; 
        }
        std::cout << "]" << std::endl;  
    }
    std::cout << "]";  

    std::string hex = vector1DToHex(flattenVector2D(diff));
    std::cout << hex << endl; 
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

