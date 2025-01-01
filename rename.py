import os
import shutil
import sys
import stat

# Global variable for the project name to replace
PROJECT_NAME = "OpenGL_project"

def make_writable(func, path, exc_info):
    """
    Callback to handle permission errors by making the file writable and retrying.
    """
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        func(path)

def replace_in_file(file_path, old_text, new_text):
    """
    Replace all occurrences of old_text with new_text in the specified file,
    reporting changed lines with line numbers.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()

        changes_made = False
        updated_lines = []
        for i, line in enumerate(lines):
            if old_text in line:
                changes_made = True
                updated_lines.append((i + 1, line.strip(), line.replace(old_text, new_text).strip()))
                lines[i] = line.replace(old_text, new_text)

        with open(file_path, 'w', encoding='utf-8') as file:
            file.writelines(lines)

        if changes_made:
            print(f"Updated content in: {file_path}")
            for line_num, before, after in updated_lines:
                print(f" Line {line_num}: '{before}' -> '{after}'")
        else:
            print(f"No occurrences of '{old_text}' found in: {file_path}")
    except Exception as e:
        print(f"Failed to update content in {file_path}: {e}")

def clean_directory(new_name):
    """
    Perform the following tasks:
    1. Remove the .vs folder from the current directory (if it exists).
    2. Remove the OpenGL_project folder (if it exists).
    3. Remove the x64 folder (if it exists).
    4. Rename all files starting with the global PROJECT_NAME to use the new name while keeping their extensions.
    5. Replace all occurrences of PROJECT_NAME with the new name in the renamed files.
    6. Replace all occurrences of PROJECT_NAME with the new name in "main.cpp".
    7. Report if no files with the name PROJECT_NAME were found.
    """
    # Get the current directory
    current_dir = os.getcwd()

    # Define the items to be removed
    items_to_remove = [".vs", PROJECT_NAME, "x64"]

    # Remove specified files and folders if they exist
    for item in items_to_remove:
        item_path = os.path.join(current_dir, item)
        if os.path.exists(item_path):
            try:
                if os.path.isdir(item_path):
                    shutil.rmtree(item_path, onerror=make_writable)
                    print(f"Removed directory: {item}")
                else:
                    os.remove(item_path)
                    print(f"Removed file: {item}")
            except PermissionError as e:
                print(f"PermissionError: Could not remove {item}: {e}")
        else:
            print(f"Item does not exist: {item}")

    # Rename all files starting with PROJECT_NAME
    files_found = False
    for file in os.listdir(current_dir):
        if file.startswith(PROJECT_NAME):
            files_found = True
            old_path = os.path.join(current_dir, file)
            new_filename = file.replace(PROJECT_NAME, new_name, 1)
            new_path = os.path.join(current_dir, new_filename)
            try:
                os.rename(old_path, new_path)
                print(f"Renamed {file} to {new_filename}")

                # Replace content in the renamed file
                replace_in_file(new_path, PROJECT_NAME, new_name)
            except Exception as e:
                print(f"Failed to rename {file}: {e}")

    if not files_found:
        print(f"No files with the name '{PROJECT_NAME}' were found in the current directory.")

    # Replace occurrences in "main.cpp"
    main_cpp_path = os.path.join(current_dir, "main.cpp")
    if os.path.exists(main_cpp_path):
        replace_in_file(main_cpp_path, PROJECT_NAME, new_name)
    else:
        print("main.cpp does not exist in the current directory.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <new_name>")
        sys.exit(1)

    new_name = sys.argv[1]
    clean_directory(new_name)
    input("\nPress any key to close...")
