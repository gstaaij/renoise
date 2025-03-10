#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "src/nob.h"

#define CMD_CC(cmd) cmd_append(cmd, "gcc")
#define CMD_CFLAGS(cmd) cmd_append(cmd, "-Wall", "-Wextra", "-Wswitch-enum", "-ggdb")
#define CMD_LFLAGS(cmd) cmd_append(cmd, "-lm")

static const char* renoise_cfiles[] = {
    "renoise",
};

bool build_renoise() {
    bool result = true;
    Cmd cmd = {0};
    File_Paths object_files = {0};
    Procs procs = {0};

    if (!mkdir_if_not_exists("./build/renoise")) return_defer(false);
    if (!mkdir_if_not_exists("./build/lib")) return_defer(false);
    if (!mkdir_if_not_exists("./build/include")) return_defer(false);

    for (size_t i = 0; i < ARRAY_LEN(renoise_cfiles); ++i) {
        const char* input_path = temp_sprintf("./src/%s.c", renoise_cfiles[i]);
        const char* output_path = temp_sprintf("./build/renoise/%s.o", renoise_cfiles[i]);

        da_append(&object_files, output_path);

        const char* depends[] = {
            input_path,
            "./src/renoise.h",
        };

        if (needs_rebuild(output_path, depends, ARRAY_LEN(depends))) {
            CMD_CC(&cmd);
            CMD_CFLAGS(&cmd);
            cmd_append(&cmd, "-c", input_path);
            cmd_append(&cmd, "-o", output_path);
            CMD_LFLAGS(&cmd);
            Proc proc = cmd_run_async_and_reset(&cmd);
            da_append(&procs, proc);
        }
    }
    if (!procs_wait_and_reset(&procs)) return_defer(false);

    const char* lib_path = "./build/lib/renoise.a";
    if (needs_rebuild(lib_path, object_files.items, object_files.count)) {
        cmd_append(&cmd, "ar", "-crs", lib_path);
        cmd_append(&cmd, "./build/renoise/renoise.o");
        if (!cmd_run_sync_and_reset(&cmd)) return_defer(false);
    }

    const char* header_src = "./src/renoise.h";
    const char* header_dest = "./build/include/renoise.h";
    if (needs_rebuild1(header_dest, header_src))
        copy_file(header_src, header_dest);

defer:
    cmd_free(cmd);
    da_free(object_files);
    da_free(procs);
    return result;
}

void log_usage(Log_Level level, const char* program) {
    nob_log(level, "Usage: %s COMMAND", program);
}

void log_commands(Log_Level level) {
    nob_log(level, "Available commands:");
    nob_log(level, "  build     Build only the library");
    nob_log(level, "  example   Build the example program");
}

static const char* examples[] = {
    "simple_demo",
};

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    const char* program = shift(argv, argc);

    if (argc < 1) {
        log_usage(ERROR, program);
        nob_log(ERROR, "No command provided.");
        log_commands(ERROR);
        return 1;
    }

    bool compile_example = true;
    const char* command = shift(argv, argc);

    if (strcmp(command, "example") == 0) {
        compile_example = true;
    } else if (strcmp(command, "build") == 0) {
        compile_example = false;
    } else {
        log_usage(ERROR, program);
        nob_log(ERROR, "Unknown command %s", command);
        log_commands(ERROR);
        return 1;
    }

    if (!mkdir_if_not_exists("./build")) return 1;

    if (!build_renoise()) return 1;

    if (!compile_example) return 0;

    Cmd cmd = {0};
    Procs procs = {0};
    for (size_t i = 0; i < ARRAY_LEN(examples); ++i) {
        CMD_CC(&cmd);
        CMD_CFLAGS(&cmd);
        cmd_append(&cmd, "-I./raylib/raylib-5.5_linux_amd64/include");
        cmd_append(&cmd, "-I./build/include");
        cmd_append(&cmd, "-o", temp_sprintf("./build/%s", examples[i]));
        cmd_append(&cmd, temp_sprintf("./example/%s.c", examples[i]));
        cmd_append(&cmd, "-L./build/lib", "-l:renoise.a");
        cmd_append(&cmd, "-L./raylib/raylib-5.5_linux_amd64/lib", "-l:libraylib.a", "-lm");
        da_append(&procs, cmd_run_async_and_reset(&cmd));
    }
    if (!procs_wait_and_reset(&procs)) return 1;

    return 0;
} 
