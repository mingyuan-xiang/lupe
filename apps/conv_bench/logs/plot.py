import re

freq = 32767

def parse_data(data):
    pattern = r"total cycles \((?P<frequency>\d+ Hz)\), repeat for (?P<repeat>\d+) times: (?P<time>\d+) \(kernel size: (?P<kernel_size>\d+), in_channels: (?P<in_channels>\d+), out_channels: (?P<out_channels>\d+), input size: (?P<input_size>\d+)\)"
    matches = re.findall(pattern, data, flags=re.DOTALL)
    results = []
    for match in matches:
        result = {
            "repeat": int(match[1]),
            "time": int(match[2]),
            "kernel size": int(match[3]),
            "in_channels": int(match[4]),
            "out_channels": int(match[5]),
            "input size": int(match[6])
        }
        results.append(result)
    return results

with open('mac_conv.log') as f:
    data = f.read()
    mac_data = parse_data(data)

with open('fir_conv.log') as f:
    data = f.read()
    fir_data = parse_data(data)


print(mac_data)
print(fir_data)