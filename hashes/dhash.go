// ---- dhash.go ----
// Implementação do Difference Hash (dHash).
package hashes

import (
	"fmt"
	"image"
)

// DHash calcula o Difference Hash (horizontal).
func DHash(img image.Image, hashSize int) (*ImageHash, error) {
	if hashSize < 2 {
		return nil, fmt.Errorf("o tamanho do hash deve ser maior ou igual a 2")
	}

	// Redimensiona para (hashSize+1, hashSize) e converte para escala de cinza.
	pixels := preprocessImage(img, uint(hashSize+1), uint(hashSize))

	// Calcula as diferenças entre colunas adjacentes.
	bits := make([][]bool, hashSize)
	for r := 0; r < hashSize; r++ {
		bits[r] = make([]bool, hashSize)
		for c := 0; c < hashSize; c++ {
			bits[r][c] = pixels[r][c+1] > pixels[r][c]
		}
	}

	return NewImageHash(bits), nil
}

// DHashVertical calcula o Difference Hash (vertical).
func DHashVertical(img image.Image, hashSize int) (*ImageHash, error) {
	if hashSize < 2 {
		return nil, fmt.Errorf("o tamanho do hash deve ser maior ou igual a 2")
	}

	// Redimensiona para (hashSize, hashSize+1) e converte para escala de cinza.
	pixels := preprocessImage(img, uint(hashSize), uint(hashSize+1))

	// Calcula as diferenças entre linhas adjacentes.
	bits := make([][]bool, hashSize)
	for r := 0; r < hashSize; r++ {
		bits[r] = make([]bool, hashSize)
		for c := 0; c < hashSize; c++ {
			bits[r][c] = pixels[r+1][c] > pixels[r][c]
		}
	}

	return NewImageHash(bits), nil
}
