import math

width = 128
height = 64
frames = 10
screen = [[0 for x in range(width) ] for y in range(height * frames)]

for frame in range(frames):
    for x in range(width):
        for y in range(height):
            if (x - width // 2) ** 2 + (y - height // 2) ** 2 < (100 + 5 * 140 * math.sin((frame / 9) * 3.14)):
                screen[y + frame * height][x] = 1

           
for frame in range(frames):
    for i in range(height):
        for j in range(width):
            print(int(screen[i + frame * height][j]), end="")
        print()
    print()

with open("scripts/circle2.txt", "w") as file:
    for frame in range(frames):
        for i in range(height // 8):
            for j in range(width):
                byte = 0
                for z in range(8):
                    byte += screen[(i % 4) * 16 + int(i > 3) + z * 2 + frame * height][j] * 2 ** z
                print(hex(byte), end=", ", file=file)
        
            print(file=file)
        print(file=file)
