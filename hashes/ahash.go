package hashes

import (
	"fmt"
	"image"
)

// AverageHash calcula o Average Hash de uma imagem.
func AverageHash(img image.Image, hashSize int) (*ImageHash, error) {
	if hashSize < 2 {
		return nil, fmt.Errorf("o tamanho do hash deve ser maior ou igual a 2")
	}

	// Reduz o tamanho e a complexidade, depois converte para escala de cinza.
	pixels := preprocessImage(img, uint(hashSize), uint(hashSize))

	// Converte para float64 para cálculo da média.
	floatPixels := pixelsToFloat64(pixels)

	// Encontra o valor médio dos pixels.
	avg := meanFloat64(floatPixels)

	// Cria os bits comparando cada pixel com a média.
	bits := make([][]bool, hashSize)
	for r := 0; r < hashSize; r++ {
		bits[r] = make([]bool, hashSize)
		for c := 0; c < hashSize; c++ {
			bits[r][c] = floatPixels[r][c] > avg
		}
	}

	return NewImageHash(bits), nil
}
