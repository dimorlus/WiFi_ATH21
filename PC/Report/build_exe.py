"""
Build script for creating standalone executable
"""

import PyInstaller.__main__
import os

# Build configuration for PyInstaller
PyInstaller.__main__.run([
    'main.py',
    '--onefile',                    # Create single executable file
    '--name=TempHumidityReporter',  # Name of the executable
    '--icon=NONE',                  # No icon (can add .ico file later)
    '--add-data=template.docx;.',   # Include template file
    '--hidden-import=docx',         # Ensure docx module is included
    '--hidden-import=pandas',       # Ensure pandas is included
    '--hidden-import=openpyxl',     # Excel support for pandas
    '--console',                    # Keep console window
    '--clean',                      # Clean cache
])

print("Build complete! Check 'dist' folder for TempHumidityReporter.exe")