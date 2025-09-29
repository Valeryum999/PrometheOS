def hex_to_mib(hex_str):
    value = int(hex_str, 16)  # parse hex
    return value / (1024 * 1024)

def mib_to_hex(mib):
    value = int(mib * 1024 * 1024)
    return hex(value)

def hex_to_gib(hex_str):
    value = int(hex_str, 16)  # parse hex
    return value / (1024 * 1024 * 1024)

def gib_to_hex(gib):
    value = int(gib * 1024 * 1024 * 1024)
    return hex(value)

def main():
    while True:
        s = input("Enter value (hex like 0x100000 or MiB like 4M): ").strip()
        if not s:
            break

        if s.lower().endswith("m"):  # MiB to hex
            mib = float(s[:-1])
            print(f"{mib} MiB = {mib_to_hex(mib)} bytes")
        elif s.lower().endswith("m"):  # MiB to hex
            mib = float(s[:-1])
            print(f"{mib} GiB = {gib_to_hex(mib)} bytes")
        elif s.startswith("0x"):  # hex to MiB
            print(f"{s} = {hex_to_mib(s):.2f} MiB")
            print(f"{s} = {hex_to_gib(s):.2f} GiB")
            print(f"Used {hex_to_gib(s)/hex_to_gib("0x100000000")*100 :.2f}% of RAM")
        else:
            print("Invalid format. Use 0xHEX or <number>M")

if __name__ == "__main__":
    main()