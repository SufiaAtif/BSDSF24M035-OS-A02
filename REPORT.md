# Final Report — ls Assignment (v1.1.0–v1.6.0)

## 1. Difference Between `stat()` and `lstat()`

`stat()` gets information about the file a symbolic link points to. `lstat()` gets information about the symbolic link itself. In `ls`, `lstat()` is useful when we need to identify symbolic links correctly.

## 2. Understanding `st_mode` and Bitwise Operators

The `st_mode` field stores file type and permission information. Macros such as `S_ISDIR(mode)` check whether a file is a directory. Bitwise AND (`&`) checks permission bits, such as `mode & S_IRUSR` for the owner's read permission.

## 3. Down-Then-Across Column Display

First, calculate the number of rows and columns. Then print entries by moving down each column before moving to the next column. A single loop is insufficient because the entries must be arranged by rows and columns rather than printed in simple list order.

## 4. Purpose of `ioctl()`

`ioctl()` with `TIOCGWINSZ` obtains the terminal width. Without detecting the width, a fixed 80-column setting may cause output to wrap incorrectly or leave too much empty space on terminals of different sizes.

## 5. Vertical vs. Horizontal Display

Vertical display requires calculating the number of columns and rows and mapping entries to positions. Horizontal display is simpler because it prints entries in order and wraps when the terminal width is reached. Vertical display needs more pre-calculation.

## 6. Managing Display Modes

The program uses command-line options to select the display function. `-l` calls `do_long_ls()`, `-x` calls `do_x_ls()`, and the default calls `do_ls()`. The `-R` option enables recursive listing.

## 7. Why Store Entries Before Sorting?

All directory names are stored in a dynamically allocated array so `qsort()` can sort them before printing. For millions of files, this can use substantial memory and take additional time. Memory allocation may also fail.

## 8. Purpose of the `qsort()` Comparison Function

The comparison function compares two filenames and returns a negative value, zero, or a positive value. Its signature is:

`int compare_names(const void *a, const void *b)`

It uses `const void *` because `qsort()` is a general-purpose sorting function that can sort different data types. The pointers are converted to the correct type inside the function.

## 9. ANSI Escape Codes

ANSI escape codes change terminal text colors and styles. The code `\033[0;32m` starts green text, and `\033[0m` resets the style.

Example:

`printf("\033[0;32mHello\033[0m");`

## 10. Executable Permission Bits

The executable permission bits are `S_IXUSR`, `S_IXGRP`, and `S_IXOTH`. They indicate whether the owner, group, or others have execute permission. Bitwise AND (`&`) can check each bit.

## 11. Base Case in Recursion

A base case is the condition that stops recursion. In recursive `ls`, recursion stops when a directory cannot be opened or when no further subdirectories are found to visit. The program also skips `.` and `..` to avoid repeatedly visiting the same directories.

## 12. Constructing Full Paths

A full path identifies a subdirectory relative to its parent, such as `parent_dir/subdir`. Without the parent path, `do_ls("subdir")` may look for a directory relative to the current working directory instead of the intended parent. Constructing the full path ensures the correct directory is visited.
# Operating Systems Assignment 02
## Feature 2: Long Listing (v1.1.0)

### 1. Difference between stat() and lstat()

The stat() system call retrieves information about a file. If the given path is a symbolic link, stat() follows the link and retrieves information about the file to which it points.

The lstat() system call retrieves information about the symbolic link itself instead of following it.

For an ls -l implementation, lstat() is useful because it allows the program to identify symbolic links and display the link's own permissions and file type.

### 2. Understanding st_mode and bitwise operators

The st_mode field of struct stat stores information about both the file type and its permissions.

Bitwise AND (&) is used with predefined macros to check individual bits.

For example:

- S_ISDIR(st_mode) checks whether the file is a directory.
- S_ISREG(st_mode) checks whether the file is a regular file.
- st_mode & S_IRUSR checks whether the owner has read permission.
- st_mode & S_IWUSR checks whether the owner has write permission.
- st_mode & S_IXUSR checks whether the owner has execute permission.
- st_mode & S_ISUID checks whether the set-user-ID bit is enabled.
- st_mode & S_ISGID checks whether the set-group-ID bit is enabled.
- st_mode & S_ISVTX checks whether the sticky bit is enabled.

The program uses these checks to construct a permission string such as rwxr-xr-x. The first character represents the file type, while the remaining nine characters represent read, write, and execute permissions for the owner, group, and others.

### 3. Feature implementation

The program uses getopt() to recognize the -l option. Without -l, it displays the basic directory listing. With -l, it uses lstat() to retrieve metadata, getpwuid() to resolve the owner, getgrgid() to resolve the group, and ctime() to format the modification time. A custom function formats file permissions, including special permission bits.

### 4. Testing

The program was built using make and tested with:

- ./bin/ls
- ./bin/ls -l
- ./bin/ls -l /etc

The output was compared with the system ls -l command.
