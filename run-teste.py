import subprocess
import os


executable_locations = ["simples", "complexo", "dinamico"]
thread_counts = ["1","2","4","8"]

for program in executable_locations:
    for thread_count in thread_counts:
        subprocess.run(["make","remove-images"])
        subprocess.run([f"ap-paralelo-{program}/ap-paralelo-{program}","Images/Small", thread_count])
        os.rename("Images/Small/stats.csv",f"Images/Small/stats-{program}-{thread_count}.csv")

subprocess.run(["make", "zip-stats"])