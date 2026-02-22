/* file-renamer.c - Batch file extension renamer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_PATH_LEN 4096
#define MAX_FILES 10000

/* Global flags */
static int skip_confirmation = 0;
static int recursive = 0;

/* File list storage */
static char *files_to_rename[MAX_FILES];
static int file_count = 0;

void print_usage_and_exit(const char *message) {
    if (message && message[0] != '\0') {
        fprintf(stderr, "%s\n", message);
    }
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "  ./file-renamer -p <directory_path> -f <from_extension> -t <to_extension> [-y] [-r]\n");
    fprintf(stderr, "\nArguments:\n");
    fprintf(stderr, "  -p, --path <path>      : Sets the directory path to scan\n");
    fprintf(stderr, "  -f, --from <ext>       : Sets the extension to rename from\n");
    fprintf(stderr, "  -t, --to <ext>         : Sets the extension to rename to\n");
    fprintf(stderr, "  -y                     : Skips the confirmation prompt\n");
    fprintf(stderr, "  -r, --recursive        : Recursively scan subdirectories\n");
    fprintf(stderr, "  -h, --help             : Displays this help message\n");
    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "  ./file-renamer -p /home/user/documents -f cpp -t txt\n");
    fprintf(stderr, "  ./file-renamer -p /home/user/documents -f cpp -t txt -r\n");
    exit(1);
}

/* Case-insensitive string comparison */
int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Check if path is a directory */
int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

/* Check if path is a regular file */
int is_regular_file(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode);
}

/* Get file extension (returns pointer into path, or NULL if no extension) */
const char *get_extension(const char *path) {
    const char *last_dot = strrchr(path, '.');
    if (last_dot && last_dot != path && last_dot[1] != '\0') {
        return last_dot + 1;
    }
    return NULL;
}

/* Add file to the list */
void add_file(const char *path) {
    if (file_count >= MAX_FILES) {
        fprintf(stderr, "Warning: Maximum file limit (%d) reached.\n", MAX_FILES);
        return;
    }
    files_to_rename[file_count] = strdup(path);
    if (!files_to_rename[file_count]) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(1);
    }
    file_count++;
}

/* Build new path with different extension */
void build_new_path(char *new_path, size_t new_path_size, const char *old_path, const char *new_ext) {
    const char *last_dot = strrchr(old_path, '.');
    if (last_dot) {
        size_t base_len = last_dot - old_path;
        snprintf(new_path, new_path_size, "%.*s.%s", (int)base_len, old_path, new_ext);
    } else {
        snprintf(new_path, new_path_size, "%s.%s", old_path, new_ext);
    }
}

/* Scan directory non-recursively */
void scan_directory_non_recursive(const char *dir_path, const char *from_ext) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (is_regular_file(full_path)) {
            const char *ext = get_extension(full_path);
            if (ext && strcasecmp_custom(ext, from_ext) == 0) {
                add_file(full_path);
            }
        }
    }

    closedir(dir);
}

/* Forward declaration for recursive function */
void scan_directory_recursive_impl(const char *dir_path, const char *from_ext);

/* Scan directory recursively */
void scan_directory_recursive(const char *dir_path, const char *from_ext) {
    scan_directory_recursive_impl(dir_path, from_ext);
}

void scan_directory_recursive_impl(const char *dir_path, const char *from_ext) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (is_regular_file(full_path)) {
            const char *ext = get_extension(full_path);
            if (ext && strcasecmp_custom(ext, from_ext) == 0) {
                add_file(full_path);
            }
        } else if (is_directory(full_path)) {
            scan_directory_recursive_impl(full_path, from_ext);
        }
    }

    closedir(dir);
}

/* Remove leading dot from extension if present */
void clean_extension(char *dest, const char *src, size_t dest_size) {
    if (src[0] == '.') {
        strncpy(dest, src + 1, dest_size - 1);
        dest[dest_size - 1] = '\0';
    } else {
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}

int main(int argc, char *argv[]) {
    char *path_str = NULL;
    char *from_ext_str = NULL;
    char *to_ext_str = NULL;

    /* Parse command line arguments */
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage_and_exit("");
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path") == 0) {
            i++;
            if (i < argc) {
                path_str = argv[i];
            } else {
                print_usage_and_exit("Error: Flag -p or --path requires an argument.");
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--from") == 0) {
            i++;
            if (i < argc) {
                from_ext_str = argv[i];
            } else {
                print_usage_and_exit("Error: Flag -f or --from requires an argument.");
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--to") == 0) {
            i++;
            if (i < argc) {
                to_ext_str = argv[i];
            } else {
                print_usage_and_exit("Error: Flag -t or --to requires an argument.");
            }
        } else if (strcmp(argv[i], "-y") == 0) {
            skip_confirmation = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            recursive = 1;
        } else {
            char msg[256];
            snprintf(msg, sizeof(msg), "Error: Unknown argument '%s'.", argv[i]);
            print_usage_and_exit(msg);
        }
        i++;
    }

    /* Validate required arguments */
    if (!path_str) {
        print_usage_and_exit("Error: Path (-p) must be provided.");
    }
    if (!from_ext_str) {
        print_usage_and_exit("Error: 'From' extension (-f) must be provided.");
    }
    if (!to_ext_str) {
        print_usage_and_exit("Error: 'To' extension (-t) must be provided.");
    }

    /* Clean extensions (remove leading dot if any) */
    char from_ext_clean[256];
    char to_ext_clean[256];
    clean_extension(from_ext_clean, from_ext_str, sizeof(from_ext_clean));
    clean_extension(to_ext_clean, to_ext_str, sizeof(to_ext_clean));

    /* Validate path is a directory */
    if (!is_directory(path_str)) {
        fprintf(stderr, "Error: The provided path is not a valid directory: %s\n", path_str);
        exit(1);
    }

    printf("Scanning directory: %s\n", path_str);

    /* Find files to rename */
    if (recursive) {
        scan_directory_recursive(path_str, from_ext_clean);
    } else {
        scan_directory_non_recursive(path_str, from_ext_clean);
    }

    if (file_count == 0) {
        printf("No files with extension '.%s' found to rename.\n", from_ext_clean);
        exit(0);
    }

    printf("file found:\n");
    for (int j = 0; j < file_count; j++) {
        printf("  %s\n", files_to_rename[j]);
    }
    printf("Will change extensions from '.%s' to '.%s'\n", from_ext_clean, to_ext_clean);

    /* Ask for confirmation unless skipped */
    if (!skip_confirmation) {
        printf("Do you want to proceed with renaming? (y/N): ");
        fflush(stdout);

        char input[64];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Operation cancelled.\n");
            exit(0);
        }

        /* Remove trailing newline */
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        /* Check if user confirmed */
        if (len == 0 || (len == 1 && tolower((unsigned char)input[0]) != 'y')) {
            printf("Operation cancelled.\n");
            exit(0);
        }
    }

    printf("---\n");

    /* Rename files */
    int file_renamed_count = 0;
    for (int j = 0; j < file_count; j++) {
        char new_path[MAX_PATH_LEN];
        build_new_path(new_path, sizeof(new_path), files_to_rename[j], to_ext_clean);

        printf("Renaming: %s -> %s\n", files_to_rename[j], new_path);

        if (rename(files_to_rename[j], new_path) != 0) {
            fprintf(stderr, "  -> Failed to rename file '%s'\n", files_to_rename[j]);
        } else {
            file_renamed_count++;
        }
    }

    printf("---\n");

    /* Print summary */
    if (file_renamed_count == 0) {
        printf("No files were renamed.\n");
    } else if (file_renamed_count == 1) {
        printf("Done. Successfully renamed 1 file.\n");
    } else {
        printf("Done. Successfully renamed %d files.\n", file_renamed_count);
    }

    /* Free allocated memory */
    for (int j = 0; j < file_count; j++) {
        free(files_to_rename[j]);
    }

    return 0;
}
