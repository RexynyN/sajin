// ---- phash.go ----
// Implementação do Perceptual Hash (pHash).
package hashes

import (
	"fmt"
	"image"
	"math"
)

// dct1d realiza a Transformada Discreta de Cosseno (Tipo II) em um slice 1D.
// Esta é a base para a DCT 2D.
func dct1d(data []float64) []float64 {
	n := len(data)
	result := make([]float64, n)
	factor := math.Pi / float64(n)
	scale := math.Sqrt(2.0 / float64(n))

	for i := 0; i < n; i++ {
		var sum float64
		for j := 0; j < n; j++ {
			sum += data[j] * math.Cos(float64(i)*factor*(float64(j)+0.5))
		}
		result[i] = sum
	}

	for i := 1; i < n; i++ {
		result[i] *= scale
	}
	result[0] *= math.Sqrt(1.0 / float64(n))

	return result
}

// dct2d realiza a DCT 2D em uma matriz.
// Replica scipy.fftpack.dct(scipy.fftpack.dct(pixels, axis=0), axis=1)
func dct2d(data [][]float64) [][]float64 {
	h := len(data)
	w := len(data[0])

	// DCT nas linhas
	rowsDct := make([][]float64, h)
	for r := 0; r < h; r++ {
		rowsDct[r] = dct1d(data[r])
	}

	// DCT nas colunas
	colsDct := make([][]float64, h)
	for r := 0; r < h; r++ {
		colsDct[r] = make([]float64, w)
	}

	for c := 0; c < w; c++ {
		col := make([]float64, h)
		for r := 0; r < h; r++ {
			col[r] = rowsDct[r][c]
		}
		dctCol := dct1d(col)
		for r := 0; r < h; r++ {
			colsDct[r][c] = dctCol[r]
		}
	}
	return colsDct
}

// PHash calcula o Perceptual Hash de uma imagem.
func PHash(img image.Image, hashSize, highfreqFactor int) (*ImageHash, error) {
	if hashSize < 2 {
		return nil, fmt.Errorf("o tamanho do hash deve ser maior ou igual a 2")
	}

	imgSize := uint(hashSize * highfreqFactor)
	pixels := preprocessImage(img, imgSize, imgSize)
	floatPixels := pixelsToFloat64(pixels)

	// Calcula a DCT 2D.
	dct := dct2d(floatPixels)

	// Extrai o canto superior esquerdo de baixa frequência da DCT.
	dctLowFreq := make([][]float64, hashSize)
	for r := 0; r < hashSize; r++ {
		dctLowFreq[r] = make([]float64, hashSize)
		for c := 0; c < hashSize; c++ {
			dctLowFreq[r][c] = dct[r][c]
		}
	}

	// Calcula a mediana dos coeficientes da DCT.
	med := medianFloat64(dctLowFreq)

	// O hash é a comparação de cada coeficiente com a mediana.
	bits := make([][]bool, hashSize)
	for r := 0; r < hashSize; r++ {
		bits[r] = make([]bool, hashSize)
		for c := 0; c < hashSize; c++ {
			bits[r][c] = dctLowFreq[r][c] > med
		}
	}

	return NewImageHash(bits), nil
}
