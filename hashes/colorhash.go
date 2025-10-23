// ---- colorhash.go ----
package hashes

import (
	"image"
	"math"
)

// rgbToHsv converte um pixel RGB para HSV.
// h, s, v estão no intervalo [0, 255].
func rgbToHsv(r, g, b uint32) (h, s, v uint8) {
	fr, fg, fb := float64(r)/255.0, float64(g)/255.0, float64(b)/255.0
	max := math.Max(math.Max(fr, fg), fb)
	min := math.Min(math.Min(fr, fg), fb)
	delta := max - min

	var hue float64
	if delta == 0 {
		hue = 0
	} else {
		switch max {
		case fr:
			hue = 60 * math.Mod(((fg-fb)/delta), 6)
		case fg:
			hue = 60 * (((fb - fr) / delta) + 2)
		case fb:
			hue = 60 * (((fr - fg) / delta) + 4)
		}
	}
	if hue < 0 {
		hue += 360
	}

	var saturation, value float64
	if max == 0 {
		saturation = 0
	} else {
		saturation = delta / max
	}
	value = max

	return uint8(hue / 360 * 255), uint8(saturation * 255), uint8(value * 255)
}

// ColorHash calcula o Color Hash de uma imagem.
func ColorHash(img image.Image, binbits int) (*ImageHash, error) {
	bounds := img.Bounds()
	width, height := bounds.Dx(), bounds.Dy()
	totalPixels := float64(width * height)

	intensity := make([]uint8, 0, int(totalPixels))
	hues := make([]uint8, 0, int(totalPixels))
	sats := make([]uint8, 0, int(totalPixels))

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, _ := img.At(x, y).RGBA()
			// Converte de [0, 65535] para [0, 255]
			r8, g8, b8 := uint8(r>>8), uint8(g>>8), uint8(b>>8)

			l := (0.299*float64(r8) + 0.587*float64(g8) + 0.114*float64(b8))
			intensity = append(intensity, uint8(l))

			h, s, _ := rgbToHsv(r, g, b)
			hues = append(hues, h)
			sats = append(sats, s)
		}
	}

	// Cria máscaras
	maskBlack := make([]bool, len(intensity))
	maskGray := make([]bool, len(sats))
	maskColors := make([]bool, len(sats))
	maskFaintColors := make([]bool, len(sats))
	maskBrightColors := make([]bool, len(sats))

	var blackCount, grayCount, colorsCount, faintCount, brightCount float64
	for i := 0; i < len(intensity); i++ {
		maskBlack[i] = intensity[i] < (256 / 8)
		if maskBlack[i] {
			blackCount++
		}
		maskGray[i] = sats[i] < (256 / 3)
		if !maskBlack[i] && maskGray[i] {
			grayCount++
		}
		maskColors[i] = !maskBlack[i] && !maskGray[i]
		if maskColors[i] {
			colorsCount++
			if sats[i] < (256 * 2 / 3) {
				maskFaintColors[i] = true
				faintCount++
			} else {
				maskBrightColors[i] = true
				brightCount++
			}
		}
	}

	fracBlack := blackCount / totalPixels
	fracGray := grayCount / totalPixels

	hueBins := []float64{0, 42.5, 85, 127.5, 170, 212.5, 255}
	hFaintCounts := make([]int, 6)
	hBrightCounts := make([]int, 6)

	for i := 0; i < len(hues); i++ {
		if maskFaintColors[i] {
			for j := 0; j < 6; j++ {
				if float64(hues[i]) >= hueBins[j] && float64(hues[i]) < hueBins[j+1] {
					hFaintCounts[j]++
					break
				}
			}
		}
		if maskBrightColors[i] {
			for j := 0; j < 6; j++ {
				if float64(hues[i]) >= hueBins[j] && float64(hues[i]) < hueBins[j+1] {
					hBrightCounts[j]++
					break
				}
			}
		}
	}

	maxvalue := float64(uint(1) << binbits)
	values := make([]int, 0, 14)
	values = append(values, int(math.Min(maxvalue-1, fracBlack*maxvalue)))
	values = append(values, int(math.Min(maxvalue-1, fracGray*maxvalue)))

	c := math.Max(1, colorsCount)
	for _, count := range hFaintCounts {
		values = append(values, int(math.Min(maxvalue-1, float64(count)*maxvalue/c)))
	}
	for _, count := range hBrightCounts {
		values = append(values, int(math.Min(maxvalue-1, float64(count)*maxvalue/c)))
	}

	bitarray := make([]bool, 0, 14*binbits)
	for _, v := range values {
		for i := 0; i < binbits; i++ {
			bit := (v>>(binbits-1-i))&1 == 1
			bitarray = append(bitarray, bit)
		}
	}

	bits := make([][]bool, 14)
	for i := 0; i < 14; i++ {
		bits[i] = make([]bool, binbits)
		copy(bits[i], bitarray[i*binbits:(i+1)*binbits])
	}

	return NewImageHash(bits), nil
}
