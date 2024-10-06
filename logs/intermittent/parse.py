import os
import re
import json

restart_time_pattern = r"Restart times: (\d+)"
elapsed_time_pattern = r"Elapsed time: ([\d.]+)"
rng_pattern = r"\[([0-9]+),\s*([0-9]+)\]"




dir_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')

print(dir_path)
for file_name in os.listdir(dir_path):
    if file_name.endswith('.log'):
        with open(os.path.join(dir_path, file_name), 'r') as file:
            content = file.read()

            restart_time = re.search(restart_time_pattern, content).group(1)
            elapsed_time = re.search(elapsed_time_pattern, content).group(1)
            rng_match = re.search(rng_pattern, content)

            rng_lower = rng_match.group(1)
            rng_upper = rng_match.group(2)



            print(restart_time)
            print(elapsed_time)
            print(rng_lower)
            print(rng_upper)