 Process Manager – CLI Version (Without GUI)
Description:

This C-based Process Manager provides a lightweight command-line interface for interacting with and managing Linux processes. Users can perform essential process-related operations using a text-based menu system. Key features include:

Listing all running processes using ps aux.

Filtering processes by user, showing PID, CPU/memory usage, and command.

Displaying all active PIDs in the system.

Running new processes in the background via system commands.

Stopping processes gracefully using SIGTERM.

Sending various signals (e.g., SIGKILL, SIGSTOP) to target processes with manual signal entry support.

The tool provides basic validation (e.g., checking if a PID exists before killing) and allows repeated interaction via a looped menu until the user exits.

Tools & Concepts Used:

C standard library

UNIX system calls (kill, system)

Signals and process management

Basic user interaction via scanf/fgets and console clearing

🔹 Process Manager – GUI Version (With GTK4)
Description:

This GTK4-based Process Manager offers an interactive graphical interface for visualizing and managing system processes on Linux. Built using C and GTK4, it enables users to execute key process management operations with button clicks and text views, enhancing usability and readability over a terminal.

Key Features:

List All Processes: Retrieves and displays each process’s PID and command by reading from /proc/[PID]/comm.

List Processes by User: Shows the user associated with each process using UID parsing and getpwuid.

Display PID List: Lists only process IDs from /proc.

Send Signals to Processes: A popup dialog allows users to enter a PID and signal number (e.g., 1234 9). Includes robust error handling for invalid inputs, permissions, or nonexistent processes.

All output is rendered inside a GTK TextView with monospace formatting for clarity, and real-time feedback is provided with color-coded or symbolic indicators (e.g., ✓/✗) for success or failure.

Tools & Concepts Used:

GTK4 for building GUI (buttons, dialogs, text views)

POSIX process handling and signals

Directory reading and file parsing from /proc

User-friendly feedback and error handling

PID and UID mapping to user names using getpwuid
