import numpy as np
import os
import matplotlib.pyplot as plt

def plot_stats():
    thread_configs = [1, 4, 8] 
    plt.figure(figsize=(12, 8))
    
    for threads in thread_configs:
        stats_file = f"results/stats_threads_{threads}.txt"
        if not os.path.exists(stats_file):
            print(f"Файл {stats_file} не найден")
            continue
        
        sizes = []
        times = []
        
        with open(stats_file, "r") as f:
            for line in f:
                size, time = line.strip().split()
                sizes.append(int(size))
                times.append(float(time))
        
        plt.plot(sizes, times, marker='o', label=f"{threads} потоков")
    
    plt.xlabel('Размер матрицы')
    plt.ylabel('Время выполнения (мс)')
    plt.title('Сравнение времени умножения матриц при разном числе потоков')
    plt.legend()
    plt.grid(True)
    plt.savefig("results/performance_comparison.png")
    plt.show()

if __name__ == "__main__":
    plot_stats()