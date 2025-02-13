#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

/**************************** DEFINES *******************************/

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#elif __linux__
#define DEBUG_BREAK() __builtin_trap()
#endif

typedef enum
{
    NERO,    // 0: Nero
    ROSSO,   // 1: Rosso
    VERDE,   // 2: Verde
    GIALLO,  // 3: Giallo
    BLU,     // 4: Blu
    MAGENTA, // 5: Magenta
    CIANO,   // 6: Ciano
    BIANCO,  // 7: Bianco
    NUMERO_COLORI
} TextColor;

/********************** LOGGING *************************************/

void _log(char *prefix, char *msg, TextColor textcolor, ...)
{
    // Array di stringhe con i codici ANSI corrispondenti ai colori
    const char *codici_colori[] = {
        "\033[30m", // NERO
        "\033[31m", // ROSSO
        "\033[32m", // VERDE
        "\033[33m", // GIALLO
        "\033[34m", // BLU
        "\033[35m", // MAGENTA
        "\033[36m", // CIANO
        "\033[37m"  // BIANCO
    };

    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", codici_colori[textcolor], prefix, msg);
    // Buffer per il messaggio finale
    char textBuffer[8192] = {};

    // Gestione degli argomenti variabili
    va_list args;
    va_start(args, textcolor);                                     // Inizializza la lista degli argomenti variabili
    vsnprintf(textBuffer, sizeof(textBuffer), formatBuffer, args); // Formatta il messaggio
    va_end(args);                                                  // Termina l'uso della lista degli argomenti variabili

    // Stampa il messaggio finale
    puts(textBuffer);
}

#define SM_TRACE(msg, ...) _log("TRACE:", msg, VERDE, ##__VA_ARGS__)
#define SM_WARN(msg, ...) _log("WARN:", msg, GIALLO, ##__VA_ARGS__)
#define SM_ERROR(msg, ...) _log("ERROR:", msg, ROSSO, ##__VA_ARGS__)

#define SM_ASSERT(x, msg, ...)            \
    {                                     \
        if (!(x))                         \
        {                                 \
            SM_ERROR(msg, ##__VA_ARGS__); \
            DEBUG_BREAK();                \
        }                                 \
    }

/******************** BUMP ALLOCATOR *******************************/

typedef struct
{
    size_t capacity;
    size_t used;
    char *memory;
} BumpAllocator;

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator ba = {};
    ba.memory = (char *)malloc(size);
    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size);
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate memory");
    }
    return ba;
}

char *bump_alloc(BumpAllocator *bumpAllocator, size_t size)
{
    char *result = NULL;
    size_t alignedSize = (size + 7) & ~7;
    if (bumpAllocator->used + alignedSize <= bumpAllocator->capacity)
    {
        result = bumpAllocator->memory + bumpAllocator->used;
        bumpAllocator->used += alignedSize;
    }
    else
    {
        SM_ASSERT(false, "BumpAllocator full");
    }
    return result;
}

/******************** FILE IO *******************************/

long get_timestamp(char *file)
{
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

bool file_exists(char *filePath)
{
    SM_ASSERT(filePath, "No filePath supplied");

    FILE *file = fopen(filePath, "rb");
    if (!file)
    {
        return false;
    }
    fclose(file);
    return true;
}

long get_file_size(char *filePath)
{
    SM_ASSERT(filePath, "No filePath supplied");
    long fileSize = 0;
    FILE *file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed opening file %s", filePath);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return fileSize;
}

char *read_file_in_buffer(char *filePath, size_t *fileSize, char *buffer)
{
    SM_ASSERT(filePath, "No filePath supplied");
    SM_ASSERT(fileSize, "No fileSize supplied");
    SM_ASSERT(buffer, "No buffer supplied");

    *fileSize = 0;
    FILE *file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed opening file %s", filePath);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);
    fclose(file);
}

void write_file(char *filePath, char *buffer, size_t size)
{
    SM_ASSERT(filePath, "No filePath supplied");
    SM_ASSERT(buffer, "No buffer supplied");
    FILE *file = fopen(filePath, "wb");
    if (!file)
    {
        SM_ERROR("Failed opening file %s", filePath);
        return;
    }
    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

char *read_file(char *filePath, size_t *fileSize, BumpAllocator *bumpAllocator)
{
    char *file = NULL;
    long fileSize2 = get_file_size(filePath);
    if (fileSize2)
    {
        char *buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        file = read_file_in_buffer(filePath, fileSize, buffer);
    }
    return file;
}

bool copy_file_in_buffer(char *srcPath, char *destPath, char *buffer)
{
    size_t fileSize = 0;
    char *data = read_file(srcPath, &fileSize, buffer);
    FILE *output = fopen(destPath, "wb");
    if (!output)
    {
        SM_ERROR("Failed opening file %s", destPath);
        return false;
    }
    size_t result = fwrite(data, sizeof(char), fileSize, output);
    if (!result)
    {
        SM_ERROR("Failed writing file %s", destPath);
        return false;
    }
    fclose(output);
    return true;
}

bool copy_file(char *srcPath, char *destPath, BumpAllocator *bumpAllocator)
{
    char *file = NULL;
    long fileSize2 = get_file_size(srcPath);
    if (fileSize2)
    {
        char *buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        return copy_file_in_buffer(srcPath, destPath, buffer);
    }
    return false;
}