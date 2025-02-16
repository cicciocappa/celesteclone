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

// Struttura per rappresentare il bump allocator
typedef struct
{
    char *start; // Puntatore all'inizio dell'area pre-allocata
    char *ptr;   // Puntatore corrente (usato per allocare)
    size_t size; // Dimensione totale dell'area pre-allocata
} BumpAllocator;

// Inizializza il bump allocator con una certa dimensione
BumpAllocator *bump_allocator_create(size_t size)
{
    
    BumpAllocator *allocator = malloc(sizeof(BumpAllocator));
    if (!allocator)
    {
        return NULL;
    }

    // Allocare l'area di memoria
    allocator->start = malloc(size);
    if (!allocator->start)
    {
        free(allocator);
        return NULL;
    }

    allocator->ptr = allocator->start; // Inizializza il puntatore all'inizio
    allocator->size = size;

    return allocator;
}

// Alloca una quantità di memoria specificata
void *bump_allocator_alloc(BumpAllocator *allocator, size_t size)
{
    if (!allocator || !allocator->start || size == 0)
    {
        return NULL;
    }

    // Allinea la dimensione richiesta al multiplo più vicino di sizeof(void*)
    size_t aligned_size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

    // Controlla se c'è abbastanza spazio disponibile
    if ((allocator->ptr + aligned_size) > (allocator->start + allocator->size))
    {
        return NULL; // Non c'è abbastanza spazio
    }

    // Salva l'indirizzo corrente e sposta il puntatore
    void *result = allocator->ptr;
    allocator->ptr += aligned_size;

    return result;
}
 
// Resetta il bump allocator, deallocando tutta la memoria
void bump_allocator_reset(BumpAllocator *allocator)
{
    if (allocator)
    {
        allocator->ptr = allocator->start; // Ripristina il puntatore all'inizio
    }
}

// Distrugge completamente il bump allocator, liberando l'area pre-allocata
void bump_allocator_destroy(BumpAllocator *allocator)
{
    if (allocator)
    {
        if (allocator->start)
        {
            free(allocator->start); // Libera l'area pre-allocata
        }
        free(allocator); // Libera la struttura del bump allocator
    }
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

char *read_file(const char* filename, BumpAllocator* allocator, size_t* fileSize)
{
    if (!filename || !allocator || !fileSize) {
        return NULL;  // Parametri non validi
    }

    // Apri il file in modalità binaria
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Impossibile aprire il file: %s\n", filename);
        return NULL;
    }

    // Ottieni la dimensione del file
    fseek(file, 0, SEEK_END);  // Vai alla fine del file
    *fileSize = ftell(file);   // Ottieni la posizione (dimensione del file)
    rewind(file);              // Torna all'inizio del file

    if (*fileSize == -1) {
        fclose(file);
        return NULL;  // Errore nel determinare la dimensione del file
    }

    // Alloca spazio sufficiente nel bump allocator
    void* buffer = bump_allocator_alloc(allocator, *fileSize);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Impossibile allocare memoria per il file\n");
        return NULL;
    }

    // Leggi il contenuto del file nel buffer
    size_t bytesRead = fread(buffer, 1, *fileSize, file);
    if (bytesRead != *fileSize) {
        fprintf(stderr, "Errore durante la lettura del file\n");
        fclose(file);
        return NULL;
    }

    // Chiudi il file e restituisci il buffer
    fclose(file);
    return buffer;
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

int copy_file(const char* source_filename, const char* dest_filename, BumpAllocator* allocator) {
    if (!source_filename || !dest_filename || !allocator) {
        fprintf(stderr, "Parametri non validi\n");
        return -1;  // Errore: parametri non validi
    }

    size_t fileSize;
    void* buffer = read_file(source_filename, allocator, &fileSize);

    if (!buffer) {
        fprintf(stderr, "Impossibile leggere il file sorgente: %s\n", source_filename);
        return -1;
    }

    // Apri il file di destinazione in modalità scrittura binaria
    FILE* dest_file = fopen(dest_filename, "wb");
    if (!dest_file) {
        fprintf(stderr, "Impossibile aprire il file di destinazione: %s\n", dest_filename);
        return -1;
    }

    // Scrivi il contenuto del buffer nel file di destinazione
    size_t bytesWritten = fwrite(buffer, 1, fileSize, dest_file);
    if (bytesWritten != fileSize) {
        fprintf(stderr, "Errore durante la scrittura del file di destinazione\n");
        fclose(dest_file);
        return -1;
    }

    // Chiudi il file di destinazione
    fclose(dest_file);

    printf("File '%s' copiato con successo in '%s'. Dimensione: %zu byte\n", 
           source_filename, dest_filename, fileSize);

    return 0;  // Successo
}
