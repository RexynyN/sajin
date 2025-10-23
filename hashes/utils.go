// ---- utils.go ----
// Contém funções auxiliares para processamento de imagem e cálculos.
package hashes

import (
	"image"
	"image/color"
	"sort"

	"github.com/nfnt/resize"
	"gonum.org/v1/gonum/stat"
)

// preprocessImage redimensiona a imagem, converte para escala de cinza e retorna os pixels como um array 2D.
func preprocessImage(img image.Image, width, height uint) [][]uint8 {
	// Redimensiona usando o filtro Lanczos3, que é o equivalente ao ANTIALIAS/LANCZOS do Pillow.
	resizedImg := resize.Resize(width, height, img, resize.Lanczos3)

	// Converte para escala de cinza.
	bounds := resizedImg.Bounds()
	w, h := bounds.Max.X, bounds.Max.Y
	grayImg := image.NewGray(bounds)

	pixels := make([][]uint8, h)
	for y := 0; y < h; y++ {
		pixels[y] = make([]uint8, w)
		for x := 0; x < w; x++ {
			grayColor := color.GrayModel.Convert(resizedImg.At(x, y)).(color.Gray)
			pixels[y][x] = grayColor.Y
			grayImg.SetGray(x, y, grayColor)
		}
	}
	return pixels
}

// pixelsToFloat64 converte um array 2D de uint8 para float64.
func pixelsToFloat64(pixels [][]uint8) [][]float64 {
	h := len(pixels)
	w := len(pixels[0])
	floats := make([][]float64, h)
	for r := 0; r < h; r++ {
		floats[r] = make([]float64, w)
		for c := 0; c < w; c++ {
			floats[r][c] = float64(pixels[r][c])
		}
	}
	return floats
}

// meanFloat64 calcula a média de um array 2D de float64.
func meanFloat64(matrix [][]float64) float64 {
	var flat []float64
	for _, row := range matrix {
		flat = append(flat, row...)
	}
	return stat.Mean(flat, nil)
}

// medianFloat64 calcula a mediana de um array 2D de float64.
func medianFloat64(matrix [][]float64) float64 {
	var flat []float64
	for _, row := range matrix {
		flat = append(flat, row...)
	}
	sort.Float64s(flat) // gonum/stat/sort.go também usa sort.Float64s
	return stat.Quantile(0.5, stat.Empirical, flat, nil)
}

// --- Funções de filtro simples ---

// BoxBlur aplica um filtro de desfoque simples.
// Usado como uma aproximação para GaussianBlur/MedianFilter.
func boxBlur(pixels [][]float32, radius int) [][]float32 {
	if radius <= 0 {
		return pixels
	}
	height := len(pixels)
	width := len(pixels[0])
	blurred := make([][]float32, height)

	for r := 0; r < height; r++ {
		blurred[r] = make([]float32, width)
		for c := 0; c < width; c++ {
			var sum float32
			var count int
			for dr := -radius; dr <= radius; dr++ {
				for dc := -radius; dc <= radius; dc++ {
					nr, nc := r+dr, c+dc
					if nr >= 0 && nr < height && nc >= 0 && nc < width {
						sum += pixels[nr][nc]
						count++
					}
				}
			}
			if count > 0 {
				blurred[r][c] = sum / float32(count)
			}
		}
	}
	return blurred
}
