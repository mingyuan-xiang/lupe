def convert_to_flat_c_array(file_path):
    with open(file_path, 'r') as file:
        data = file.read()

    data = data.strip().replace('[', '').replace(']', '').replace(',', '')
    elements = list(map(int, data.split()))

    c_array = ""
    c_array += "    " + ', '.join(map(str, elements)) + "\n"
    c_array += "};"

    print(c_array)

file_path = 'log'
convert_to_flat_c_array(file_path)

