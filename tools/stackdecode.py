import subprocess as sp
import os
project_dir = os.path.join(os.path.dirname(__file__), "..")
pio_dir = os.path.join(os.path.expanduser("~"), ".platformio")
addr2line = os.path.join(pio_dir, "packages", "toolchain-xtensa", "bin", "xtensa-lx106-elf-addr2line.exe")
elf_file = os.path.join(project_dir, ".pio", "build", "d1_mini", "firmware.elf")
java = "java"
jar = os.path.join(os.path.dirname(__file__), "EspStackTraceDecoder.jar")
dump = os.path.join(os.path.dirname(__file__), "dump.txt")

if __name__ == '__main__':
    sp.run([java, "-jar", jar, addr2line, elf_file, dump])