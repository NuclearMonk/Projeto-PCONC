import subprocess
import os


executable_locations = ["simples", "complexo", "dinamico", "pipeline"]
thread_counts = ["1","2","4","8"]
datasets = [1,2]
for dataset in datasets:
    for program in executable_locations:
        for thread_count in thread_counts:
            subprocess.run(["make","remove-images"])
            subprocess.run([f"ap-paralelo-{program}/ap-paralelo-{program}",f"dataset{dataset}/", thread_count])
            os.rename(f"dataset{dataset}/stats.csv",f"dataset{dataset}/stats-dataset{dataset}-{program}-{thread_count}.csv")

subprocess.run(["make", "zip-stats"])