// ---- whash.go ----
// Implementação do Wavelet Hash (wHash).
// ATENÇÃO: A implementação de Wavelets é complexa e pode não produzir
// resultados idênticos a `pywt` devido a diferenças sutis na implementação
// dos filtros e tratamento de bordas. Esta é uma implementação "best-effort"
// do wHash usando a transformada de Haar.
package hashes

import (
	"fmt"
	"image"
	"math"
)

// wavedec2Haar realiza uma decomposição de wavelet 2D usando o filtro Haar.
func wavedec2Haar(data [][]float64, level int) ([][][]float64, error) {
	if level < 1 {
		return nil, fmt.Errorf("o nível de decomposição deve ser pelo menos 1")
	}

	coeffs := make([][]float64, len(data))
	for i := range data {
		coeffs[i] = make([]float64, len(data[i]))
		copy(coeffs[i], data[i])
	}

	var result [][][]float64
	for i := 0; i < level; i++ {
		h := len(coeffs)
		w := len(coeffs[0])

		if h < 2 || w < 2 {
			break // Não é possível decompor mais
		}

		newH, newW := h/2, w/2
		ll := make([][]float64, newH)
		lh := make([][]float64, newH)
		hl := make([][]float64, newH)
		hh := make([][]float64, newH)

		for r := 0; r < newH; r++ {
			ll[r] = make([]float64, newW)
			lh[r] = make([]float64, newW)
			hl[r] = make([]float64, newW)
			hh[r] = make([]float64, newW)
		}

		// Transformada 1D nas linhas
		temp := make([][]float64, h)
		for r := 0; r < h; r++ {
			temp[r] = make([]float64, w)
			for c := 0; c < newW; c++ {
				a := coeffs[r][2*c]
				b := coeffs[r][2*c+1]
				temp[r][c] = (a + b) / math.Sqrt2
				temp[r][c+newW] = (a - b) / math.Sqrt2
			}
		}

		// Transformada 1D nas colunas
		for c := 0; c < w; c++ {
			for r := 0; r < newH; r++ {
				a := temp[2*r][c]
				b := temp[2*r+1][c]

				val1 := (a + b) / math.Sqrt2
				val2 := (a - b) / math.Sqrt2

				// Distribui para LL, LH, HL, HH
				if c < newW { // LL ou HL
					if r < newH {
						ll[r][c] = val1
						hl[r][c] = val2
					}
				} else { // LH ou HH
					if r < newH {
						lh[r][c-newW] = val1
						hh[r][c-newW] = val2
					}
				}
			}
		}

		coeffs = ll
		result = append([][][]float64{lh, hl, hh}, result...)
	}

	result = append([][][]float64{coeffs}, result...) // Adiciona a aproximação final (LL)

	return result, nil
}

// WHash calcula o Wavelet Hash de uma imagem.
func WHash(img image.Image, hashSize int) (*ImageHash, error) {
	if hashSize < 2 || (hashSize&(hashSize-1)) != 0 {
		return nil, fmt.Errorf("o tamanho do hash deve ser uma potência de 2")
	}

	// Determina a escala da imagem (a maior potência de 2 menor que o tamanho da imagem)
	minSize := img.Bounds().Dx()
	if img.Bounds().Dy() < minSize {
		minSize = img.Bounds().Dy()
	}
	imageScale := uint(math.Pow(2, math.Floor(math.Log2(float64(minSize)))))
	if imageScale < uint(hashSize) {
		imageScale = uint(hashSize)
	}

	pixels := preprocessImage(img, imageScale, imageScale)
	floatPixels := pixelsToFloat64(pixels)

	// Normaliza os pixels para o intervalo [0, 1]
	for r := range floatPixels {
		for c := range floatPixels[r] {
			floatPixels[r][c] /= 255.0
		}
	}

	// Nível de decomposição para chegar ao hashSize
	llMaxLevel := int(math.Log2(float64(imageScale)))
	level := int(math.Log2(float64(hashSize)))
	dwtLevel := llMaxLevel - level

	// Decompõe a imagem usando a transformada de Haar
	coeffs, err := wavedec2Haar(floatPixels, dwtLevel)
	if err != nil {
		return nil, fmt.Errorf("falha na decomposição de wavelet: %w", err)
	}

	// O primeiro elemento é a banda LL de baixa frequência
	dwtLow := coeffs[0]

	med := medianFloat64(dwtLow)

	bits := make([][]bool, len(dwtLow))
	for r := 0; r < len(dwtLow); r++ {
		bits[r] = make([]bool, len(dwtLow[0]))
		for c := 0; c < len(dwtLow[0]); c++ {
			bits[r][c] = dwtLow[r][c] > med
		}
	}

	return NewImageHash(bits), nil
}
