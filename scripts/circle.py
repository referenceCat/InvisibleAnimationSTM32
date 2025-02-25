screen = [[0 for x in range(128) ] for y in range(64)]

for x in range(128):
    for y in range(64):
        if (x - 64) ** 2 + (y - 32) ** 2 < 400:
            screen[y][x] = 1
            print(y,x)


for i in range(64):
    for j in range(128):
        print(int(screen[i][j]), end="")
    
    print()

with open("scripts/circle.txt", "w") as file:
    for i in range(8):
        for j in range(128):
            byte = 0
            for z in range(8):
                byte += screen[(i % 4) * 16 + int(i > 3) + z * 2][j] * 2 ** z
            print(hex(byte), end=", ", file=file)
        
        print(file=file)
