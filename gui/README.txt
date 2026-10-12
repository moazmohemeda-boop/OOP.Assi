========================================================================
PROJECT RUNNING GUIDE (README)
Course : CS213 - Object Oriented Programming
Assignment : Assignment 1 (Version 8.2) - Part 2 (Beast Level)
Faculty : Faculty of Computing and AI (FCAI)
University : Cairo University
Year : 2026-2027 - First Semester
Supervisor : Dr. Mohammad El-Ramly
========================================================================

   1. TEAM DETAILS & WORK BREAKDOWN
   ========================================================================
   Section: [lab 19_20]

Team Members:

   1. Moaz Mohammed - ID: 20251409
   2. Abdelrahman Elsayed - ID: 20250352
   3. Abdullah Khaled - ID: 20250381
   4. Mohammed Mahmoud - ID: 20251345

Work Breakdown:

 Abdelrahman Elsayed : Implemented Filters 1, 5, 9, 13
 Abdullah Khaled     : Implemented Filters 2, 6, 10, 14
 Mohammed Mahmoud    : Implemented Filters 3, 7, 11, 15
 Moaz Mohammed       : Implemented Filters 4, 8, 12, 16 + [Menu / GUI / GitHub]
 Bonus (Task 4)      : All Team Members (Filters 17, 18 + GUI)

========================================================================
2. TOOLS AND VERSIONS USED

* Operating System : Windows 11
* IDE / Editor     : VS Code
* Compiler         : g++ (MinGW-w64) supporting C++17 standard
* GUI Framework    : Course Provided Graphics Library (FCAI Library)
* Image Libraries  : Image_Class.h, stb_image.h, stb_image_write.h

========================================================================
3. HOW TO SET UP AND RUN THE CODE
To set up and run this project, ensure that your g++ compiler is correctly configured in your environment variables.
Steps to build and run:

   1. Extract the project zip file into a dedicated folder.
   2. Make sure all library files (Image_Class.h, stb_image.h, stb_image_write.h)
   are in the same folder as your source code.
   3. Open the project folder using VS Code or your preferred IDE.
   4. Open the terminal inside the project directory and run the compilation command:g++ -std=c++17 main.cpp -o  ImageProcessor
   5. Run the generated executable file:
   ./ImageProcessor

Note: The Graphical User Interface (GUI) menu will launch automatically,
allowing you to continuously load images, apply any of the 18 filters,
and save the final output.
========================================================================

 