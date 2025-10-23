package main

import (
	"fmt"
	"image"
	"log"
	"os"
	"sajin/hashes"
)

func getAllHashes() {
	// Verifique se o caminho do arquivo foi fornecido
	if len(os.Args) < 2 {
		log.Fatalf("Uso: go run . /caminho/para/sua/imagem.jpg")
	}
	filePath := os.Args[1]

	// Abrir o arquivo de imagem
	file, err := os.Open(filePath)
	if err != nil {
		log.Fatalf("Falha ao abrir o arquivo: %v", err)
	}
	defer file.Close()

	// Decodificar a imagem
	img, format, err := image.Decode(file)
	if err != nil {
		log.Fatalf("Falha ao decodificar a imagem: %v", err)
	}
	fmt.Printf("Imagem carregada: %s, Formato: %s\n\n", filePath, format)

	hashSize := 8

	// Calcular e imprimir os hashes
	fmt.Println("--- Calculando Hashes ---")

	// Average Hash
	aHash, err := hashes.AverageHash(img, hashSize)
	if err != nil {
		log.Printf("Erro ao calcular aHash: %v", err)
	} else {
		fmt.Printf("Average Hash (aHash)      : %s\n", aHash)
	}

	// Difference Hash (Horizontal)
	dHash, err := hashes.DHash(img, hashSize)
	if err != nil {
		log.Printf("Erro ao calcular dHash: %v", err)
	} else {
		fmt.Printf("Difference Hash (dHash)   : %s\n", dHash)
	}

	// Perceptual Hash
	pHash, err := hashes.PHash(img, hashSize, 4)
	if err != nil {
		log.Printf("Erro ao calcular pHash: %v", err)
	} else {
		fmt.Printf("Perceptual Hash (pHash)   : %s\n", pHash)
	}

	// Wavelet Hash
	wHash, err := hashes.WHash(img, hashSize)
	if err != nil {
		log.Printf("Erro ao calcular wHash: %v", err)
	} else {
		fmt.Printf("Wavelet Hash (wHash)      : %s\n", wHash)
	}

	// Color Hash
	cHash, err := hashes.ColorHash(img, 3)
	if err != nil {
		log.Printf("Erro ao calcular colorHash: %v", err)
	} else {
		fmt.Printf("Color Hash (colorHash)    : %s\n", cHash)
	}

	// Crop Resistant Hash
	crHash, err := hashes.CropResistantHash(img, nil, 0, 128, 500, 300)
	if err != nil {
		log.Printf("Erro ao calcular cropResistantHash: %v", err)
	} else {
		fmt.Printf("Crop Resistant Hash       : %s\n", crHash)
	}

	// --- Exemplo de comparação de multi-hash ---
	if len(os.Args) > 2 && crHash != nil {
		fmt.Println("\n--- Comparando Crop Resistant Hash com a segunda imagem ---")
		filePath2 := os.Args[2]
		file2, err := os.Open(filePath2)
		if err != nil {
			log.Fatalf("Falha ao abrir a segunda imagem: %v", err)
		}
		defer file2.Close()
		img2, _, err := image.Decode(file2)
		if err != nil {
			log.Fatalf("Falha ao decodificar a segunda imagem: %v", err)
		}

		crHash2, _ := hashes.CropResistantHash(img2, nil, 0, 128, 500, 300)

		// Compara os dois hashes
		// regionCutoff=1 (pelo menos 1 segmento correspondente)
		// bitErrorRate=0.25 (segmentos podem ser até 25% diferentes)
		matches := crHash.Matches(crHash2, 1, 0.25)

		fmt.Printf("As imagens '%s' e '%s' correspondem? %t\n", filePath, filePath2, matches)
	}
}
