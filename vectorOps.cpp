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


Vector2D matGSToVector2D(const cv::Mat& inputRaw) {
    if (inputRaw.empty()) 
        return {};

    cv::Mat img;
    // Tratamento de segurança: Se vier colorida, converte para cinza automaticamente
    if (inputRaw.channels() == 3) {
        cv::cvtColor(inputRaw, img, cv::COLOR_BGR2GRAY);
    } else if (inputRaw.channels() == 4) {
        cv::cvtColor(inputRaw, img, cv::COLOR_BGRA2GRAY);
    } else {
        img = inputRaw; // Já é 1 canal, apenas copia o header (shallow copy)
    }

    int rows = img.rows;
    int cols = img.cols;

    Vector2D vec(rows, std::vector<uint8_t>(cols));
    for (int i = 0; i < rows; ++i) {
        // Ponteiro para o início da linha na Mat
        const uint8_t* rowPtr = img.ptr<uint8_t>(i);
        
        // Ponteiro para o início da linha no Vector
        // vec[i].data() retorna o ponteiro para o array interno do vector
        uint8_t* vecPtr = vec[i].data(); 

        // OTIMIZAÇÃO: Copia a linha inteira de memória de uma vez.
        // Isso é muito mais rápido que um loop "for (int j...)"
        std::copy(rowPtr, rowPtr + cols, vecPtr);
    }

    return vec;
}

cv::Mat vector2DToMatGS(const Vector2D& vec) {
    if (vec.empty() || vec[0].empty()) return cv::Mat();

    int rows = vec.size();
    int cols = vec[0].size();

    // CV_8UC1 = 8-bit Unsigned, 1 Channel
    cv::Mat img(rows, cols, CV_8UC1);
    for (int i = 0; i < rows; ++i) {
        // Validação de segurança para vetores irregulares (jagged arrays)
        if (vec[i].size() != cols) {
            std::cerr << "Erro: Linha " << i << " tem tamanho diferente." << std::endl;
            continue; 
        }

        const uint8_t* vecPtr = vec[i].data();
        uint8_t* rowPtr = img.ptr<uint8_t>(i);

        // Copia a linha inteira de volta para a Mat
        std::copy(vecPtr, vecPtr + cols, rowPtr);
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


// Função auxiliar para converter 0-15 em caractere Hex
char nibbleToHex(uint8_t nibble) {
    return (nibble < 10) ? ('0' + nibble) : ('a' + (nibble - 10));
}

std::string binaryMatToHex(const cv::Mat& binaryImg) {
    if (binaryImg.empty()) return "";

    // 1. Validar e garantir que é uma matriz contínua para iteração rápida
    // Se não for contínua, fazemos um clone para garantir
    cv::Mat flat = binaryImg.isContinuous() ? binaryImg : binaryImg.clone();
    
    // Total de bits disponíveis
    int totalBits = flat.total(); // rows * cols
    
    // 2. Calcular alinhamento
    // Em Python: ceil(len / 4). Precisamos agrupar de 4 em 4 bits.
    // Se tivermos 6 bits (ex: 11 1111), precisamos adicionar 2 zeros à esquerda (0011 1111)
    // para que a matemática de grupos de 4 funcione corretamente.
    int remainder = totalBits % 4;
    int padding = (remainder == 0) ? 0 : (4 - remainder);

    std::string hexString;
    hexString.reserve((totalBits + padding) / 4);

    uint8_t currentNibble = 0;
    int bitsProcessed = 0;

    // 3. Adicionar Padding (Zeros virtuais à esquerda)
    // Isso simula o comportamento do int(string, 2) do Python que não se importa com zeros à esquerda
    for (int i = 0; i < padding; ++i) {
        currentNibble = (currentNibble << 1) | 0;
        bitsProcessed++;
        // Nota: Como padding < 4, nunca vamos completar um nibble aqui dentro
        // a menos que o padding fosse 4 (o que a lógica acima impede, seria 0)
    }

    // 4. Iterar pelos pixels da imagem
    const uint8_t* data = flat.ptr<uint8_t>(0);
    for (int i = 0; i < totalBits; ++i) {
        // Normaliza para 0 ou 1 (caso a imagem tenha 255)
        uint8_t bit = (data[i] > 0) ? 1 : 0;

        // Shift Left e adiciona o bit (Operação: acumulador * 2 + bit)
        currentNibble = (currentNibble << 1) | bit;
        bitsProcessed++;

        // Se completamos 4 bits, temos um dígito Hex
        if (bitsProcessed == 4) {
            hexString += nibbleToHex(currentNibble);
            currentNibble = 0;
            bitsProcessed = 0;
        }
    }

    return hexString;
}

char nibbleVecToHex(uint8_t nibble) {
    static const char* hexLookup = "0123456789abcdef";
    return hexLookup[nibble & 0x0F];
}

// Funciona para vector<uint8_t>, vector<int>, etc.
std::string vector1DToHex(const Vector1D& binaryVec) {
    if (binaryVec.empty()) return "";

    size_t totalBits = binaryVec.size();
    
    // 1. Calcular quantos zeros virtuais precisamos adicionar à esquerda
    // Exemplo: 5 bits. 5 % 4 = 1 sobra. Precisamos de (4-1) = 3 zeros de padding.
    // Se 8 bits. 8 % 4 = 0 sobra. Padding = 0.
    size_t remainder = totalBits % 4;
    size_t padding = (remainder == 0) ? 0 : (4 - remainder);

    std::string hexString;
    // Reserva memória para evitar realocações (Total de nibbles)
    hexString.reserve((totalBits + padding) / 4);

    uint8_t currentNibble = 0;
    int bitsProcessed = 0;

    // 2. Processar Padding (Zeros à esquerda)
    for (size_t i = 0; i < padding; ++i) {
        currentNibble = (currentNibble << 1) | 0;
        bitsProcessed++;
    }

    // 3. Processar o Vetor Real
    for (const auto& val : binaryVec) {
        // Normaliza para 0 ou 1
        uint8_t bit = (val > 0) ? 1 : 0;

        currentNibble = (currentNibble << 1) | bit;
        bitsProcessed++;

        // A cada 4 bits, despeja um caractere Hex
        if (bitsProcessed == 4) {
            hexString += nibbleVecToHex(currentNibble);
            currentNibble = 0;
            bitsProcessed = 0;
        }
    }

    return hexString;
}



// int setShowcase() {
//     std::set<std::string> unique_cars = {"Volvo", "BMW", "Ford", "BMW", "Mazda"};
//     unique_cars.insert("Tesla");
//     unique_cars.insert("VW");

//     std::cout << "Unique cars in sorted order:" << std::endl;
//     for (const std::string& car : unique_cars) {
//         std::cout << car << std::endl;
//     }

//     if (unique_cars.count("Ford")) {
//         std::cout << "\nFord is in the set." << std::endl;
//     }

//     unique_cars.erase("Volvo");

//     std::cout << "\nAfter removing Volvo, size is: " << unique_cars.size() << std::endl;
    
//     return 0;
// }


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
    Vector1D sorted = flatVec;
    std::sort(sorted.begin(), sorted.end());  

    size_t len = sorted.size(); 
    if (sorted.size() % 2 != 0) { // Odd number of items
        return sorted[floor(len / 2)]; 
    }

    size_t midpoint = (size_t) floor(len / 2); // Mean of the two central values if even
    return (sorted[midpoint] + sorted[midpoint - 1]);
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



// double medianVector2D(const Vector2D& vec) {
//     return medianVector1D(flattenVector2D(vec));
// }

// double medianVector3D(const Vector3D& vec) {
//     return medianVector1D(flattenVector3D(vec));
// }


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