
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>
#include <sys/ioctl.h>

void do_ls(const char *dir);
void do_long_ls(const char *dir);
void print_permissions(mode_t mode);

int main(int argc, char *argv[])
{
    int option;
    int long_format = 0;

    while ((option = getopt(argc, argv, "l")) != -1)
    {
        if (option == 'l')
            long_format = 1;
        else
        {
            fprintf(stderr, "Usage: %s [-l] [directory ...]\n", argv[0]);
            return 1;
        }
    }

    if (optind == argc)
    {
        if (long_format)
            do_long_ls(".");
        else
            do_ls(".");
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s :\n", argv[i]);

            if (long_format)
                do_long_ls(argv[i]);
            else
                do_ls(argv[i]);

            puts("");
        }
    }

    return 0;
}


int compare_names(const void *a, const void *b)
{
    const char *name_a = *(const char **)a;
    const char *name_b = *(const char **)b;
    return strcmp(name_a, name_b);
}

void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    char **names = NULL;
    size_t count = 0;
    size_t capacity = 0;
    size_t max_length = 0;
    size_t columns;
    size_t rows;
    size_t column_width;
    size_t row;
    size_t col;
    size_t index;
    size_t next_index;
    int terminal_width = 80;
    struct winsize window;

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == 0
        && window.ws_col > 0)
    {
        terminal_width = window.ws_col;
    }

    errno = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        char *name;
        size_t length;

        if (entry->d_name[0] == '.')
            continue;

        length = strlen(entry->d_name);
        name = malloc(length + 1);

        if (name == NULL)
        {
            perror("malloc");
            break;
        }

        strcpy(name, entry->d_name);

        if (count == capacity)
        {
            size_t new_capacity = (capacity == 0) ? 16 : capacity * 2;
            char **new_names = realloc(
                names, new_capacity * sizeof(char *));

            if (new_names == NULL)
            {
                free(name);
                perror("realloc");
                break;
            }

            names = new_names;
            capacity = new_capacity;
        }

        names[count++] = name;

        if (length > max_length)
            max_length = length;
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    if (count == 0)
    {
        free(names);
        return;
    }

    qsort(names, count, sizeof(char *), compare_names);

    column_width = max_length + 2;
    columns = (size_t)terminal_width / column_width;

    if (columns < 1)
        columns = 1;

    if (columns > count)
        columns = count;

    rows = (count + columns - 1) / columns;

    /* Print down first, then across. */
    for (row = 0; row < rows; row++)
    {
        for (col = 0; col < columns; col++)
        {
            index = col * rows + row;

            if (index >= count)
                continue;

            printf("%s", names[index]);

            next_index = (col + 1) * rows + row;

            if (col + 1 < columns && next_index < count)
            {
                size_t length = strlen(names[index]);
                size_t spaces = column_width - length;

                while (spaces-- > 0)
                    putchar(' ');
            }
        }

        putchar('\n');
    }

    for (index = 0; index < count; index++)
        free(names[index]);

    free(names);
}

void print_permissions(mode_t mode)
{
    char permissions[11];

    if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISLNK(mode))
        permissions[0] = 'l';
    else if (S_ISCHR(mode))
        permissions[0] = 'c';
    else if (S_ISBLK(mode))
        permissions[0] = 'b';
    else if (S_ISFIFO(mode))
        permissions[0] = 'p';
    else if (S_ISSOCK(mode))
        permissions[0] = 's';
    else
        permissions[0] = '-';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    if (mode & S_ISUID)
        permissions[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        permissions[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        permissions[9] = (mode & S_IXOTH) ? 't' : 'T';

    permissions[10] = '\0';

    printf("%s", permissions);
}

void do_long_ls(const char *dir)
{
    struct dirent *entry;
    struct stat file_info;
    DIR *dp = opendir(dir);
    char path[PATH_MAX];
    char *modified_time;
    struct passwd *user;
    struct group *group;
    long total_blocks = 0;

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    /*
       First pass: calculate total allocated blocks.
    */
    errno = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (snprintf(path, sizeof(path), "%s/%s",
                     dir, entry->d_name) >= (int)sizeof(path))
            continue;

        if (lstat(path, &file_info) == 0)
            total_blocks += file_info.st_blocks;
    }

    printf("total %ld\n", total_blocks / 2);

    /*
       Second pass: print each entry.
    */
    rewinddir(dp);
    errno = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (snprintf(path, sizeof(path), "%s/%s",
                     dir, entry->d_name) >= (int)sizeof(path))
            continue;

        if (lstat(path, &file_info) == -1)
        {
            perror(entry->d_name);
            continue;
        }

        user = getpwuid(file_info.st_uid);
        group = getgrgid(file_info.st_gid);

        print_permissions(file_info.st_mode);

        printf(" %2lu %-8s %-8s %8lld ",
               (unsigned long)file_info.st_nlink,
               user ? user->pw_name : "?",
               group ? group->gr_name : "?",
               (long long)file_info.st_size);

        modified_time = ctime(&file_info.st_mtime);

        if (modified_time != NULL)
        {
            modified_time[strlen(modified_time) - 1] = '\0';
            printf("%.12s ", modified_time + 4);
        }
        else
        {
            printf("unknown time ");
        }

        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}
