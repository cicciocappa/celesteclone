#pragma once
#include <stdio.h>
#include <stdarg.h>

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

#define SM_ASSERT(x, msg, ...)       \
{                                    \
    if (!(x))                        \
    {                                \
       SM_ERROR(msg, ##__VA_ARGS__); \
       DEBUG_BREAK();                \
    }                                \
}                                     

