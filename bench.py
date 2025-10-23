import imagehash as ih
from PIL import Image


def compute_hashes(frame: Image):
    return {
        "avg": ih.average_hash(frame),
        "crop": ih.crop_resistant_hash(frame),
        "whash": ih.whash(frame),
        "phash": ih.phash(frame),
        "dhash": ih.dhash(frame),
        "color": ih.colorhash(frame)
    }



image = Image.open("example.png")


hashes = compute_hashes(image)
for name, hash_value in hashes.items():
    print(f"{name}: {hash_value}")