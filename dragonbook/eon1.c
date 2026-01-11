#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

typedef ptrdiff_t ssize;

void print_usage(void);
void parse(const char* code, const ssize code_length);

int
main(const int number_of_args,
     const char* args[])
{
    if (number_of_args != 2)
    {
        print_usage();
        return 1;
    }

    const char* filename = args[1];

    printf("Opening '%s'\n", filename);

    const int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        printf("Failed to open '%s'", filename);
        perror("");
        return 1;
    }

    struct stat file_stats = {0};
    fstat(fd, &file_stats);

    ssize content_length = file_stats.st_size;

    if (content_length == 0)
    {
        printf("Error: file '%s' is empty.\n"
               "  Suggestion: add 'main: () = {}' to be able generate an empty program that does nothing.\n",
               filename);
        return 0;
    }

    char* content = (char*) calloc(sizeof(char),
                                   content_length + 1);

    read(fd, content, content_length);
    close(fd);

    parse(content, content_length);

    free(content);

    return 0;
}

void
print_usage(void)
{
    puts("usage: eon <filename>");
}

void parse(const char* code, const ssize code_length)
{
    printf("File content:\n'%s'\n", code);
}
