#!/usr/bin/env python3
"""
Sistema Integrado de Compressão de Imagens com Bibliotecas Python
"""

import os
import sys
import time
import json
from pathlib import Path
import numpy as np
from PIL import ImageFile, Image, ImageFilter
import cv2

# ImageFile.LOAD_TRUNCATED_IMAGES = True

def calculate_psnr(img1, img2):
    # Garante que ambas têm 3 canais
    if img1.ndim == 3 and img2.ndim == 2:
        img2 = np.stack([img2]*3, axis=-1)
    elif img1.ndim == 2 and img2.ndim == 3:
        img1 = np.stack([img1]*3, axis=-1)
    mse = np.mean((img1.astype(np.float64) - img2.astype(np.float64)) ** 2)
    return 20 * np.log10(255.0 / np.sqrt(mse)) if mse != 0 else float('inf')

def calculate_ssim(img1, img2):
    # Converte ambas para escala de cinza
    if img1.ndim == 3:
        img1 = np.dot(img1[...,:3], [0.2989, 0.5870, 0.1140])
    if img2.ndim == 3:
        img2 = np.dot(img2[...,:3], [0.2989, 0.5870, 0.1140])
    mu1 = img1.mean()
    mu2 = img2.mean()
    sigma1 = img1.var()
    sigma2 = img2.var()
    sigma12 = ((img1 - mu1) * (img2 - mu2)).mean()
    C1 = (0.01 * 255) ** 2
    C2 = (0.03 * 255) ** 2
    ssim = ((2 * mu1 * mu2 + C1) * (2 * sigma12 + C2)) / ((mu1**2 + mu2**2 + C1) * (sigma1 + sigma2 + C2))
    return ssim

class PythonImageCompressor:
    def __init__(self, input_path):
        self.input_path = Path(input_path)
        self.original = Image.open(input_path)
        self.original_array = np.array(self.original)
        self.original_size = os.path.getsize(input_path)
        self.results = []

    def run_compression(self, output_dir="compressed"):
        output_dir = Path(output_dir)
        output_dir.mkdir(exist_ok=True)

        methods = [
            self._compress_pillow_png,
            self._compress_webp_lossless,
            self._compress_jpeg_high_quality,
            self._quantize_adaptive,
            self._resize_half
        ]

        for method in methods:
            start_time = time.time()
            result = method(output_dir)
            if result:
                result['processing_time'] = time.time() - start_time
                self.results.append(result)

        self._generate_report(output_dir)

    def _compress_pillow_png(self, output_dir):
        path = output_dir / "pillow_png.png"
        self.original.save(
            path,
            format='PNG',
            compress_level=9,
            optimize=True,
            dictionary=b'\x00' * 32768)
        return self._create_result(path, "PNG Pillow Max")

    def _compress_webp_lossless(self, output_dir):
        path = output_dir / "webp_lossless.webp"
        self.original.save(
            path,
            format='WEBP',
            lossless=True,
            quality=100,
            method=6)
        return self._create_result(path, "WebP Lossless")

    def _compress_jpeg_high_quality(self, output_dir):
        path = output_dir / "jpeg_high.jpg"
        self.original.convert('RGB').save(
            path,
            format='JPEG',
            quality=95,
            subsampling=0,
            qtables='web_high')
        return self._create_result(path, "JPEG Quasi-Lossless")

    def _quantize_adaptive(self, output_dir):
        path = output_dir / "quantized.png"
        quantized = self.original.quantize(
            colors=256,
            method=Image.MEDIANCUT,
            dither=Image.NONE)
        quantized.save(
            path,
            optimize=True,
            compress_level=9)
        return self._create_result(path, "PNG Quantizado")

    def _resize_half(self, output_dir):
        path = output_dir / "resized_half.png"
        new_size = tuple(dim//2 for dim in self.original.size)
        resized = self.original.resize(new_size, Image.Resampling.LANCZOS)
        resized.save(path)
        return self._create_result(path, "Redução 50% LANCZOS")

    def _create_result(self, path, method_name):
        compressed_size = path.stat().st_size
        compressed_img = Image.open(path)
        
        return {
            'method': method_name,
            'path': str(path),
            'original_size': self.original_size,
            'compressed_size': compressed_size,
            'compression_ratio': self.original_size / compressed_size,
            'size_reduction': (1 - compressed_size/self.original_size) * 100,
            'psnr': calculate_psnr(self.original_array, np.array(compressed_img)),
            'ssim': calculate_ssim(
                cv2.cvtColor(self.original_array, cv2.COLOR_RGB2GRAY),
                cv2.cvtColor(np.array(compressed_img), cv2.COLOR_RGB2GRAY))
        }

    def _generate_report(self, output_dir):
        report = {
            'input_image': str(self.input_path),
            'original_size': self.original_size,
            'compression_results': sorted(
                self.results,
                key=lambda x: x['compression_ratio'],
                reverse=True)
        }

        report_path = output_dir / "compression_report.json"
        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2)

        print("\n=== Relatório de Compressão ===")
        print(f"Imagem original: {self.input_path}")
        print(f"Tamanho original: {self.original_size:,} bytes\n")
        
        for result in report['compression_results']:
            print(f"{result['method']}:")
            print(f"  Tamanho: {result['compressed_size']:,} bytes")
            print(f"  Redução: {result['size_reduction']:.1f}%")
            print(f"  PSNR: {result['psnr']:.2f} dB")
            print(f"  SSIM: {result['ssim']:.4f}")
            print(f"  Tempo: {result['processing_time']:.2f}s\n")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python compress.py <caminho_da_imagem>")
        sys.exit(1)

    compressor = PythonImageCompressor(sys.argv[1])
    compressor.run_compression()
