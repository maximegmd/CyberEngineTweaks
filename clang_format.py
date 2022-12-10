import os
import time
import subprocess

def format_cpp_file(file_path, clang_format_path):
  # Run clang-format on the file to format it
  subprocess.run(["clang-format", "-i", file_path, "-style=file:" + clang_format_path, "-fallback-style=none"])

def recursive_directory_iteration(directory, clang_format_path):
  # Iterate over all of the files in the directory
  for filename in os.listdir(directory):
    # Get the full path of the file
    file_path = os.path.join(directory, filename)
    
    # If the file is a directory, recursively iterate over it
    if os.path.isdir(file_path):
      recursive_directory_iteration(file_path, clang_format_path)
    # If the file is a cpp file, format it
    elif file_path.endswith(".cpp") or file_path.endswith(".h") or file_path.endswith(".hpp"):
      format_cpp_file(file_path, clang_format_path)

print("Formatting C++ files...")

path = os.getcwd() + "\\.clang-format"
if not os.path.exists(path):
  print("Could not find clang format config at " + path)
  exit()

print("Found clang format config at " + path)

start_time = time.time()
# Start the recursive iteration from the current directory
recursive_directory_iteration(os.getcwd() + "/src", path)

print("Done formatting C++ files after " + str(time.time() - start_time) + " seconds")