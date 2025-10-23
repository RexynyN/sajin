package hashes

import (
	"fmt"
	_ "image/jpeg" // Suporte para decodificar imagens JPEG
	_ "image/png"  // Suporte para decodificar imagens PNG
	"math"
	"math/big"
	"strings"
)

// ImageHash encapsula o hash de uma imagem.
// A estrutura armazena um array bidimensional de booleanos.
type ImageHash struct {
	Bits [][]bool
}

// NewImageHash cria uma nova instância de ImageHash.
func NewImageHash(bits [][]bool) *ImageHash {
	return &ImageHash{Bits: bits}
}

// binaryArrayToHex converte um array de booleanos para uma string hexadecimal.
// Esta função replica o comportamento da função _binary_array_to_hex do Python.
func binaryArrayToHex(bits [][]bool) string {
	var sb strings.Builder
	for _, row := range bits {
		for _, bit := range row {
			if bit {
				sb.WriteString("1")
			} else {
				sb.WriteString("0")
			}
		}
	}

	bitString := sb.String()
	if len(bitString) == 0 {
		return ""
	}

	// Usa big.Int para lidar com strings de bits de qualquer tamanho
	i := new(big.Int)
	i.SetString(bitString, 2)

	// Calcula o comprimento necessário para o preenchimento com zeros
	width := int(math.Ceil(float64(len(bitString)) / 4.0))

	return fmt.Sprintf("%0*x", width, i)
}

// String retorna a representação hexadecimal do hash.
func (h *ImageHash) String() string {
	return binaryArrayToHex(h.Bits)
}

// flatten achata a matriz de bits em um único slice.
func (h *ImageHash) flatten() []bool {
	var flat []bool
	for _, row := range h.Bits {
		flat = append(flat, row...)
	}
	return flat
}

// Distance calcula a distância de Hamming entre dois hashes.
// Retorna o número de bits que são diferentes.
func (h *ImageHash) Distance(other *ImageHash) (int, error) {
	selfFlat := h.flatten()
	otherFlat := other.flatten()

	if len(selfFlat) != len(otherFlat) {
		return 0, fmt.Errorf("os hashes devem ter o mesmo tamanho. Self: %d, Other: %d", len(selfFlat), len(otherFlat))
	}

	distance := 0
	for i := range selfFlat {
		if selfFlat[i] != otherFlat[i] {
			distance++
		}
	}
	return distance, nil
}

// HexToHash converte uma string hexadecimal de volta para um ImageHash.
// Assume que o hash é um array quadrado.
func HexToHash(hexStr string) (*ImageHash, error) {
	if hexStr == "" {
		return nil, fmt.Errorf("a string hexadecimal não pode estar vazia")
	}

	totalBits := len(hexStr) * 4
	hashSize := int(math.Sqrt(float64(totalBits)))

	if hashSize*hashSize != totalBits {
		return nil, fmt.Errorf("formato de hash inválido, não é um quadrado perfeito")
	}

	i := new(big.Int)
	_, success := i.SetString(hexStr, 16)
	if !success {
		return nil, fmt.Errorf("string hexadecimal inválida")
	}

	binaryStr := fmt.Sprintf("%0*b", totalBits, i)

	bits := make([][]bool, hashSize)
	for r := 0; r < hashSize; r++ {
		bits[r] = make([]bool, hashSize)
		for c := 0; c < hashSize; c++ {
			idx := r*hashSize + c
			if idx < len(binaryStr) && binaryStr[idx] == '1' {
				bits[r][c] = true
			}
		}
	}
	return NewImageHash(bits), nil
}

// HexToFlatHash converte uma string hexadecimal para um hash plano (1D).
// Usado principalmente pelo ColorHash.
func HexToFlatHash(hexstr string, hashsize int) (*ImageHash, error) {
	if hexstr == "" {
		return nil, fmt.Errorf("a string hexadecimal não pode estar vazia")
	}
	totalBits := len(hexstr) * 4
	numRows := totalBits / hashsize

	i := new(big.Int)
	_, success := i.SetString(hexstr, 16)
	if !success {
		return nil, fmt.Errorf("string hexadecimal inválida")
	}

	binaryStr := fmt.Sprintf("%0*b", totalBits, i)

	bits := make([][]bool, numRows)
	for r := 0; r < numRows; r++ {
		bits[r] = make([]bool, hashsize)
		for c := 0; c < hashsize; c++ {
			idx := r*hashsize + c
			if idx < len(binaryStr) && binaryStr[idx] == '1' {
				bits[r][c] = true
			}
		}
	}
	return NewImageHash(bits), nil
}
