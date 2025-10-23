package main

import (
	"encoding/csv"
	"fmt" // Importa o pacote que criamos
	"image"
	_ "image/jpeg" // Necessário para decodificar frames .jpg
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sajin/hashes"
	"strconv"
	"strings"
	"sync"
	"time"
)

// --- Configurações Globais ---
const (
	// CWD é o diretório raiz a ser verificado
	CWD = "/mnt/d/back-up/downloads/folder/porterting/chunky"
	// DATA_PATH é onde os arquivos CSV de saída serão salvos
	DATA_PATH = "csv_output_go"
	// FRAME_INTERVAL_MS define o intervalo de extração de quadros em milissegundos
	FRAME_INTERVAL_MS = 1000
	// NUM_WORKERS define o número de vídeos a serem processados em paralelo
	NUM_WORKERS = 30
)

// EXT_WHITELIST define as extensões de vídeo permitidas
var EXT_WHITELIST = map[string]bool{
	".mp4": true,
	".m4v": true,
	".mkv": true,
	".mov": true,
}

// HashRecord armazena os hashes calculados e metadados para um único quadro
type HashRecord struct {
	File       string
	FrameOrder int
	AvgHash    string
	CropHash   string
	WHash      string
	PHash      string
	DHash      string
	ColorHash  string
}

// logMutex garante que a escrita no arquivo de log seja segura entre goroutines
var logMutex sync.Mutex

// --- Funções Principais ---

// getAllFiles varre o diretório CWD recursivamente e retorna uma lista de caminhos de vídeo válidos.
func getAllFiles(root string) ([]string, error) {
	var files []string
	err := filepath.WalkDir(root, func(path string, d os.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if !d.IsDir() {
			ext := strings.ToLower(filepath.Ext(path))
			if EXT_WHITELIST[ext] {
				files = append(files, path)
			}
		}
		return nil
	})
	if err != nil {
		return nil, fmt.Errorf("erro ao varrer diretório: %w", err)
	}
	return files, nil
}

// computeHashes calcula todos os hashes necessários para um único quadro de imagem.
func computeHashes(frame image.Image) (*HashRecord, error) {
	// Usamos goroutines para calcular os hashes em paralelo para um mesmo quadro,
	// acelerando ainda mais o processo.
	var wg sync.WaitGroup
	var record HashRecord
	var errs = make(chan error, 6) // Canal para coletar erros

	wg.Add(6)

	go func() {
		defer wg.Done()
		if h, err := hashes.AverageHash(frame, 8); err == nil {
			record.AvgHash = h.String()
		} else {
			errs <- fmt.Errorf("avg_hash: %w", err)
		}
	}()

	go func() {
		defer wg.Done()
		if h, err := hashes.DHash(frame, 8); err == nil {
			record.DHash = h.String()
		} else {
			errs <- fmt.Errorf("dhash: %w", err)
		}
	}()

	go func() {
		defer wg.Done()
		if h, err := hashes.PHash(frame, 8, 4); err == nil {
			record.PHash = h.String()
		} else {
			errs <- fmt.Errorf("phash: %w", err)
		}
	}()

	go func() {
		defer wg.Done()
		if h, err := hashes.WHash(frame, 8); err == nil {
			record.WHash = h.String()
		} else {
			errs <- fmt.Errorf("whash: %w", err)
		}
	}()

	go func() {
		defer wg.Done()
		if h, err := hashes.ColorHash(frame, 3); err == nil {
			record.ColorHash = h.String()
		} else {
			errs <- fmt.Errorf("colorhash: %w", err)
		}
	}()

	go func() {
		defer wg.Done()
		// Parâmetros padrão do Python: hash_func=dhash, limit=0, threshold=128, min_size=500, size=300
		if h, err := hashes.CropResistantHash(frame, nil, 0, 128, 500, 300); err == nil {
			record.CropHash = h.String()
		} else {
			errs <- fmt.Errorf("crophash: %w", err)
		}
	}()

	wg.Wait()
	close(errs)

	// Verifica se algum erro ocorreu durante o cálculo dos hashes
	for err := range errs {
		if err != nil {
			// Retorna o primeiro erro encontrado
			return nil, err
		}
	}

	return &record, nil
}

// ffmpegExtractFrames usa o ffmpeg para extrair quadros de um vídeo.
func ffmpegExtractFrames(videoPath, outputDir string) error {
	// A taxa de quadros é 1 / (intervalo em segundos)
	fps := fmt.Sprintf("fps=1/%.1f", float64(FRAME_INTERVAL_MS)/1000.0)
	outputPath := filepath.Join(outputDir, "%06d.jpg")

	cmd := exec.Command("ffmpeg", "-i", videoPath, "-vf", fps, "-hide_banner", "-loglevel", "error", outputPath)

	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("ffmpeg falhou para '%s': %s, %w", videoPath, string(output), err)
	}
	return nil
}

// processVideoHashes extrai quadros, calcula hashes e retorna uma lista de registros.
func processVideoHashes(path string) ([]*HashRecord, error) {
	// Cria um diretório temporário para os quadros
	tempDir, err := os.MkdirTemp("", "frames-*")
	if err != nil {
		return nil, fmt.Errorf("falha ao criar diretório temporário: %w", err)
	}
	// Garante que o diretório temporário seja removido no final
	defer os.RemoveAll(tempDir)

	// Extrai os quadros do vídeo
	if err := ffmpegExtractFrames(path, tempDir); err != nil {
		return nil, err
	}

	// Lê os quadros extraídos
	frameFiles, err := filepath.Glob(filepath.Join(tempDir, "*.jpg"))
	if err != nil {
		return nil, fmt.Errorf("falha ao listar quadros: %w", err)
	}

	if len(frameFiles) == 0 {
		return []*HashRecord{}, nil
	}

	var hashes []*HashRecord
	for i, framePath := range frameFiles {
		file, err := os.Open(framePath)
		if err != nil {
			log.Printf("Aviso: falha ao abrir o quadro %s: %v", framePath, err)
			continue
		}

		img, _, err := image.Decode(file)
		file.Close() // Fecha o arquivo logo após o decode
		if err != nil {
			log.Printf("Aviso: falha ao decodificar o quadro %s: %v", framePath, err)
			continue
		}

		hashRecord, err := computeHashes(img)
		if err != nil {
			log.Printf("Aviso: falha ao calcular hash para o quadro %s: %v", framePath, err)
			continue
		}

		hashRecord.File = filepath.Base(path)
		hashRecord.FrameOrder = i // A ordem é garantida pelo nome dos arquivos (%06d.jpg)
		hashes = append(hashes, hashRecord)
	}

	return hashes, nil
}

// worker é a função que cada goroutine executa. Ela processa vídeos de um canal de "trabalhos".
func worker(id int, jobs <-chan string, wg *sync.WaitGroup) {
	defer wg.Done()

	for path := range jobs {
		startTime := time.Now()

		fileName := strings.TrimSuffix(filepath.Base(path), filepath.Ext(path)) + ".csv"
		csvPath := filepath.Join(DATA_PATH, fileName)

		if _, err := os.Stat(csvPath); err == nil {
			log.Printf("Worker %d: %s já processado, pulando.", id, fileName)
			continue
		}

		log.Printf("Worker %d: Processando %s", id, path)

		hashes, err := processVideoHashes(path)
		if err != nil {
			log.Printf("ERRO ao processar %s: %v", path, err)
			continue
		}

		if len(hashes) == 0 {
			log.Printf("Aviso: Nenhum hash gerado para %s", path)
			continue
		}

		// Salva os hashes em um arquivo CSV
		file, err := os.Create(csvPath)
		if err != nil {
			log.Printf("ERRO: Falha ao criar CSV para %s: %v", path, err)
			continue
		}

		writer := csv.NewWriter(file)
		// Escreve o cabeçalho
		header := []string{"file", "frame_order", "avg", "crop", "whash", "phash", "dhash", "color"}
		writer.Write(header)

		// Escreve os dados
		for _, record := range hashes {
			row := []string{
				record.File,
				strconv.Itoa(record.FrameOrder),
				record.AvgHash,
				record.CropHash,
				record.WHash,
				record.PHash,
				record.DHash,
				record.ColorHash,
			}
			writer.Write(row)
		}

		writer.Flush()
		file.Close()

		// Registra o tempo de processamento em um arquivo de log
		elapsedTime := time.Since(startTime)
		fileInfo, _ := os.Stat(path)
		fileSizeMB := float64(fileInfo.Size()) / (1024 * 1024)

		logMsg := fmt.Sprintf("%s: %.2fmb -> %d frames in %.2f seconds\n", path, fileSizeMB, len(hashes), elapsedTime.Seconds())

		// Usa um mutex para evitar condição de corrida ao escrever no arquivo de log
		logMutex.Lock()
		logFile, err := os.OpenFile("log_time_go.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err == nil {
			logFile.WriteString(logMsg)
			logFile.Close()
		}
		logMutex.Unlock()
	}
}

func main() {
	// Garante que o diretório de saída exista
	if err := os.MkdirAll(DATA_PATH, os.ModePerm); err != nil {
		log.Fatalf("Falha ao criar diretório de dados: %v", err)
	}

	// Obtém a lista de todos os vídeos a serem processados
	videoPaths, err := getAllFiles(CWD)
	if err != nil {
		log.Fatalf("Falha ao obter lista de arquivos: %v", err)
	}
	log.Printf("Encontrados %d vídeos para processar.", len(videoPaths))

	// Configura o worker pool
	jobs := make(chan string, len(videoPaths))
	var wg sync.WaitGroup

	// Inicia os workers
	for w := 1; w <= NUM_WORKERS; w++ {
		wg.Add(1)
		go worker(w, jobs, &wg)
	}

	// Envia todos os caminhos dos vídeos para o canal de trabalhos
	for _, path := range videoPaths {
		jobs <- path
	}
	close(jobs) // Fecha o canal para indicar que não há mais trabalhos

	// Aguarda todos os workers terminarem
	wg.Wait()

	log.Println("Processamento concluído.")
}
