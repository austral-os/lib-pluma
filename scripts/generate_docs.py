#!/usr/bin/env python3
import os
import subprocess
import sys
import shutil

def main():
    # Ensure we are in the project root
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    os.chdir(project_root)

    print("=== Pluma Documentation Generator ===")

    # Check if doxygen is installed
    if not shutil.which("doxygen"):
        print("Error: 'doxygen' is not installed or not in PATH.")
        print("Please install doxygen first (e.g., 'sudo apt-get install doxygen').")
        sys.exit(1)

    print("Cleaning old documentation...")
    docs_dir = os.path.join(project_root, "docs")
    if os.path.exists(docs_dir):
        shutil.rmtree(docs_dir)
    os.makedirs(docs_dir)

    print("Generating Doxygen XML documentation...")
    try:
        # Run doxygen
        result = subprocess.run(["doxygen", "Doxyfile"], check=True, capture_output=True, text=True)
        print("Doxygen executed successfully.")
    except subprocess.CalledProcessError as e:
        print("Doxygen execution failed!")
        print(e.stderr)
        sys.exit(1)

    xml_dir = os.path.join(docs_dir, "xml")
    if os.path.exists(xml_dir):
        files = os.listdir(xml_dir)
        print(f"Success! Generated {len(files)} XML files in docs/xml/")
        
        print("Generating Markdown API Reference...")
        xml2md_script = os.path.join(script_dir, "xml_to_md.py")
        md_output = os.path.join(docs_dir, "API_REFERENCE.md")
        
        try:
            subprocess.run([sys.executable, xml2md_script, xml_dir, md_output], check=True)
            print("Cleaning up intermediate XML files...")
            shutil.rmtree(xml_dir)
            print("Cleanup complete. Only Markdown files remain.")
        except subprocess.CalledProcessError as e:
            print("Markdown generation failed!")
            sys.exit(1)
            
    else:
        print("Error: XML output directory not found.")
        sys.exit(1)

if __name__ == "__main__":
    main()
