// ---- crophashes.go ----
package hashes

import (
	"fmt"
	"image"
	"image/color"
	"math"
	"sort"
	"strings"

	"github.com/nfnt/resize"
)

// ImageMultiHash armazena uma lista de hashes para diferentes segmentos de uma imagem.
type ImageMultiHash struct {
	SegmentHashes []*ImageHash
}

func (mh *ImageMultiHash) String() string {
	var parts []string
	for _, h := range mh.SegmentHashes {
		parts = append(parts, h.String())
	}
	return strings.Join(parts, ",")
}

// HashDiff calcula a diferença entre dois multi-hashes.
// Retorna o número de segmentos correspondentes e a soma das distâncias de Hamming.
func (mh *ImageMultiHash) HashDiff(other *ImageMultiHash, hammingCutoff float64) (int, int) {
	var distances []int
	for _, segmentHash := range mh.SegmentHashes {
		lowestDistance := math.MaxInt32
		for _, otherSegmentHash := range other.SegmentHashes {
			dist, err := segmentHash.Distance(otherSegmentHash)
			if err == nil && dist < lowestDistance {
				lowestDistance = dist
			}
		}
		if float64(lowestDistance) <= hammingCutoff {
			distances = append(distances, lowestDistance)
		}
	}
	sumDist := 0
	for _, d := range distances {
		sumDist += d
	}
	return len(distances), sumDist
}

// Matches verifica se dois multi-hashes correspondem.
func (mh *ImageMultiHash) Matches(other *ImageMultiHash, regionCutoff int, bitErrorRate float64) bool {
	if len(mh.SegmentHashes) == 0 {
		return false
	}
	hashLen := len(mh.SegmentHashes[0].flatten())
	hammingCutoff := float64(hashLen) * bitErrorRate
	matches, _ := mh.HashDiff(other, hammingCutoff)
	return matches >= regionCutoff
}

// point é uma coordenada 2D simples.
type point struct{ x, y int }

// findRegion implementa uma busca em largura (flood fill) para encontrar uma região contígua.
func findRegion(pixels [][]bool, start point, alreadySegmented map[point]bool) map[point]bool {
	height := len(pixels)
	width := len(pixels[0])
	targetValue := pixels[start.y][start.x]

	inRegion := make(map[point]bool)
	queue := []point{start}

	// Marca o ponto de partida como visitado
	alreadySegmented[start] = true

	for len(queue) > 0 {
		p := queue[0]
		queue = queue[1:]

		inRegion[p] = true

		neighbors := []point{
			{p.x + 1, p.y}, {p.x - 1, p.y},
			{p.x, p.y + 1}, {p.x, p.y - 1},
		}

		for _, n := range neighbors {
			if n.x >= 0 && n.x < width && n.y >= 0 && n.y < height {
				if !alreadySegmented[n] && pixels[n.y][n.x] == targetValue {
					alreadySegmented[n] = true
					queue = append(queue, n)
				}
			}
		}
	}
	return inRegion
}

// findAllSegments encontra todas as regiões claras e escuras na imagem.
func findAllSegments(pixels [][]float32, segmentThreshold int, minSegmentSize int) []map[point]bool {
	height := len(pixels)
	width := len(pixels[0])

	thresholdPixels := make([][]bool, height)
	for r := 0; r < height; r++ {
		thresholdPixels[r] = make([]bool, width)
		for c := 0; c < width; c++ {
			thresholdPixels[r][c] = pixels[r][c] > float32(segmentThreshold)
		}
	}

	var segments []map[point]bool
	alreadySegmented := make(map[point]bool)

	process := func(targetValue bool) {
		for r := 0; r < height; r++ {
			for c := 0; c < width; c++ {
				p := point{x: c, y: r}
				if thresholdPixels[r][c] == targetValue && !alreadySegmented[p] {
					segment := findRegion(thresholdPixels, p, alreadySegmented)
					if len(segment) > minSegmentSize {
						segments = append(segments, segment)
					}
				}
			}
		}
	}

	// Encontra todas as regiões "claras" (hills)
	process(true)
	// Encontra todas as regiões "escuras" (valleys)
	process(false)

	return segments
}

// CropResistantHash calcula o hash resistente a recortes.
func CropResistantHash(
	img image.Image,
	hashFunc func(image.Image, int) (*ImageHash, error),
	limitSegments int,
	segmentThreshold int,
	minSegmentSize int,
	segmentationImageSize int,
) (*ImageMultiHash, error) {

	// O hashFunc padrão é dhash com hash_size=8
	if hashFunc == nil {
		hashFunc = func(i image.Image, hs int) (*ImageHash, error) {
			return DHash(i, 8)
		}
	}

	origImage := img
	// Converte para escala de cinza, redimensiona e aplica filtros
	resized := resize.Resize(uint(segmentationImageSize), uint(segmentationImageSize), origImage, resize.Lanczos3)

	bounds := resized.Bounds()
	w, h := bounds.Dx(), bounds.Dy()
	pixels := make([][]float32, h)
	for y := 0; y < h; y++ {
		pixels[y] = make([]float32, w)
		for x := 0; x < w; x++ {
			gray := color.GrayModel.Convert(resized.At(x+bounds.Min.X, y+bounds.Min.Y)).(color.Gray)
			pixels[y][x] = float32(gray.Y)
		}
	}

	// Emula os filtros do Pillow. Um box blur simples é usado aqui.
	pixels = boxBlur(pixels, 1) // Aproximação de GaussianBlur
	pixels = boxBlur(pixels, 1) // Aproximação de MedianFilter

	segments := findAllSegments(pixels, segmentThreshold, minSegmentSize)

	if len(segments) == 0 {
		fullSegment := make(map[point]bool)
		for r := 0; r < h; r++ {
			for c := 0; c < w; c++ {
				fullSegment[point{c, r}] = true
			}
		}
		segments = append(segments, fullSegment)
	}

	if limitSegments > 0 && len(segments) > limitSegments {
		sort.Slice(segments, func(i, j int) bool {
			return len(segments[i]) > len(segments[j])
		})
		segments = segments[:limitSegments]
	}

	var hashes []*ImageHash
	for _, segment := range segments {
		minX, minY := math.MaxInt32, math.MaxInt32
		maxX, maxY := 0, 0
		for p := range segment {
			if p.x < minX {
				minX = p.x
			}
			if p.y < minY {
				minY = p.y
			}
			if p.x > maxX {
				maxX = p.x
			}
			if p.y > maxY {
				maxY = p.y
			}
		}

		origW, origH := origImage.Bounds().Dx(), origImage.Bounds().Dy()
		scaleW := float64(origW) / float64(segmentationImageSize)
		scaleH := float64(origH) / float64(segmentationImageSize)

		cropMinX := int(float64(minX) * scaleW)
		cropMinY := int(float64(minY) * scaleH)
		cropMaxX := int(float64(maxX+1) * scaleW)
		cropMaxY := int(float64(maxY+1) * scaleH)

		// SubImage é mais eficiente que Crop, pois não copia os pixels
		sub, ok := origImage.(interface {
			SubImage(r image.Rectangle) image.Image
		})
		if !ok {
			return nil, fmt.Errorf("a imagem não suporta SubImage")
		}

		boundingBox := sub.SubImage(image.Rect(cropMinX, cropMinY, cropMaxX, cropMaxY))

		h, err := hashFunc(boundingBox, 8)
		if err != nil {
			// Ignora segmentos que falham ao gerar hash
			continue
		}
		hashes = append(hashes, h)
	}

	return &ImageMultiHash{SegmentHashes: hashes}, nil
}
