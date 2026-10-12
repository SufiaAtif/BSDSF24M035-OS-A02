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
