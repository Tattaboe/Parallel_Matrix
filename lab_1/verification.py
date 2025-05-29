import numpy as np
import os
import matplotlib.pyplot as plt

def verif() -> None:
    counts = [10, 30, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000]
    
    for i in counts:
        try:
            matrix1 = np.loadtxt(f"matrices/1-{i}.txt", dtype=int)
            matrix2 = np.loadtxt(f"matrices/2-{i}.txt", dtype=int)
            result = np.dot(matrix1, matrix2)
            cpp_result = np.loadtxt(f"results/result-{i}.txt")
            
            if np.allclose(cpp_result, result, rtol=1e-5, atol=1e-8):
                print(f"Проверка для матрицы {i}x{i} успешна")
            else:
                print(f"Ошибка: результаты не совпадают для размера {i}x{i}")
                
        except FileNotFoundError:
            print(f"Файлы для размера {i}x{i} не найдены")
            continue

def plot_stats():
    if not os.path.exists("results/stats.txt"):
        print("Файл со статистикой не найден")
        return
    
    sizes = []
    times = []
    
    with open("results/stats.txt", "r") as f:
        for line in f:
            size, time = line.strip().split()
            sizes.append(int(size))
            times.append(float(time))
    
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, times, 'bo-')
    plt.xlabel('Размер матрицы')
    plt.ylabel('Время выполнения (мс)')
    plt.title('Зависимость времени умножения матриц от их размера')
    plt.grid(True)
    plt.savefig("results/performance_plot.png")
    plt.show()

if __name__ == "__main__":
    verif()
    plot_stats()