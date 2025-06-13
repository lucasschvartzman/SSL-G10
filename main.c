#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// WINUTIL

#ifdef _WIN32
    #include <windows.h>
#endif

void habilitarColoresConsola() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    dwMode = 0;
    GetConsoleMode(hErr, &dwMode);
    SetConsoleMode(hErr, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

// --- Macros ---

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#define printmsg(format,...) \
    printf(ANSI_COLOR_BLUE format ANSI_COLOR_RESET, ##__VA_ARGS__)
#define printerr(format,...) \
    fprintf(stderr, ANSI_COLOR_RED "[ERROR] " format ANSI_COLOR_RESET, ##__VA_ARGS__)

// Para mensajes internos.
#define DESARROLLO true

#define memprinterr() \
    if(DESARROLLO) printerr("La funcion malloc() fallo cuando se ejecuto en: %s()\n",__func__)

// Una gramática regular tiene como máximo 2 símbolos en su lado derecho.
#define LADO_DERECHO_MAX 2

#define PRODUCCION_MIN 4

#define STDIN_BUFFER_SIZE 1024

#define EPSILON '@'

#define SIMBOLOS_NO_TERMINALES "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SIMBOLOS_TERMINALES "abcdefghijklmnopqrstuvwxyz"

// --- Utils ---

// Operaciones de manejo de memoria

// Considerar que si utilizamos hilos esto puede tener race-conditions.

extern size_t heapUsado;

void* tmalloc(size_t size);

void tfree(void* ptr, size_t size);

size_t heapUsado = 0;

void* tmalloc(const size_t size) {
    heapUsado += size;
    return malloc(size);
}

void tfree(void* ptr, const size_t size) {
    heapUsado -= size;
    free(ptr);
}

// Operaciones de entrada (stdin)

char* obtenerCadenaEntrada();

char obtenerCaracterEntrada();

char *obtenerCadenaEntrada() {

    char bufferEntrada[STDIN_BUFFER_SIZE];
    if (fgets(bufferEntrada,sizeof(bufferEntrada),stdin) == NULL) {
        printerr("No se pudo leer la cadena desde stdin.");
    }
    const size_t longitudCadena = strcspn(bufferEntrada, "\n");
    bufferEntrada[longitudCadena] = '\0'; // Lo hacemos C String.
    char* cadenaEntrada = malloc((longitudCadena + 1) * sizeof(char));
    if (cadenaEntrada == NULL) {
        memprinterr();
        return NULL;
    }
    strcpy(cadenaEntrada, bufferEntrada);
    return cadenaEntrada;
}

static void sanitizarBufferEntrada() {
    for (int c; (c = getchar()) != '\n' && c != EOF;);
}

char obtenerCaracterEntrada() {
    const char caracter = (char)getchar();
    if (caracter != '\n') {
        sanitizarBufferEntrada();
    }
    return caracter;
}

// Operaciones de cadenas

bool contieneCaracter(char caracter, const char *cadena);

bool soloTieneSimbolosConjunto(const char *cadena, const char *conjunto);

int contarCaracter(char caracter, const char *cadena);

bool contieneCaracter(const char caracter, const char *cadena) {
    if (cadena == NULL) {
        return NULL;
    }
    return strchr(cadena, caracter) != NULL;
}

bool soloTieneSimbolosConjunto(const char *cadena, const char *conjunto) {
    if (cadena == NULL || conjunto == NULL) {
        return false;
    }
    for (int i = 0; cadena[i] != '\0'; i++) {
        if (!contieneCaracter(cadena[i],conjunto)) {
            return false;
        }
    }
    return true;
}

int contarCaracter(const char caracter, const char *cadena) {
    int ocurrenciasCaracter = 0;
    for (int i = 0; cadena[i] != '\0'; i++) {
        if (caracter == cadena[i]) ocurrenciasCaracter++;
    }
    return ocurrenciasCaracter;
}

// --- Estructuras de datos ---

typedef struct {
    char ladoIzquierdo;
    char ladoDerecho[LADO_DERECHO_MAX + 1];
} Produccion;

typedef struct {
    // Elementos de una gramática
    char *simbolosNoTerminales;
    char *simbolosTerminales;
    Produccion *producciones;
    char axioma;
    // Información administrativa
    int cantidadProducciones;
} Gramatica;

Gramatica* crearGramatica();

bool esGramaticaRegular(const Gramatica* gramatica);

void mostrarGramatica(const Gramatica* gramatica);

char* generarPalabraAleatoria(const Gramatica* gramatica);

void destruirGramatica(Gramatica* gramatica);

void destruirGramatica(Gramatica *gramatica) {
    if (gramatica != NULL) {
        if (gramatica->simbolosNoTerminales != NULL) {
            free(gramatica->simbolosNoTerminales);
        }
        if (gramatica->simbolosTerminales != NULL) {
            free(gramatica->simbolosTerminales);
        }
        if (gramatica->producciones != NULL) {
            free(gramatica->producciones);
        }
        free(gramatica);
    }
}

char *obtenerSimbolosNoTerminales() {
    printmsg("Ingrese los simbolos no terminales (sin espacios, ni comas): ");
    char *simbolosNoTerminales = obtenerCadenaEntrada();
    if (!soloTieneSimbolosConjunto(simbolosNoTerminales,SIMBOLOS_NO_TERMINALES)) {
        printerr("Ha ingresado simbolos no terminales incorrectos (por ej. ',' o una minuscula)");
        free(simbolosNoTerminales);
        return NULL;
    }
    return simbolosNoTerminales;
}

char *obtenerSimbolosTerminales() {
    printmsg("Ingrese los simbolos terminales (sin espacios, ni comas): ");
    char *simbolosTerminales = obtenerCadenaEntrada();
    if (!soloTieneSimbolosConjunto(simbolosTerminales,SIMBOLOS_TERMINALES)) {
        printerr("Ha ingresado simbolos terminales incorrectos (por ej. ',' o una mayuscula)");
        free(simbolosTerminales);
        return NULL;
    }
    return simbolosTerminales;
}

char obtenerAxioma() {
    printmsg("Ingrese el axioma (en mayuscula): ");
    const char axioma = obtenerCaracterEntrada();
    if (!contieneCaracter(axioma,SIMBOLOS_NO_TERMINALES)) {
        printerr("Ha ingresado un caracter que no puede ser axioma.");
        return '\0';
    }
    return axioma;
}

bool esFormatoProduccionValido(const char *produccion) {
    if (produccion == NULL) {
        return false;
    }
    const size_t longitudCadena = strlen(produccion);
    // Validamos que la cadena ingresada tenga como mínimo 4 ("PRODUCCION_MIN") caracteres. Por ejemplo: "S->x"
    if (longitudCadena < PRODUCCION_MIN) {
        return false;
    }
    // Validamos que la cadena ingresada tenga una flecha "->"
    const char *flecha = strstr(produccion, "->");
    if (flecha == NULL) {
        return false;
    }
    // Validamos que el lado izquierdo de la cadena, tenga un único caracter.
    const size_t longitudLadoIzquierdo = flecha - produccion;
    if (longitudLadoIzquierdo != 1) {
        return false;
    }
    // Validamos que el lado derecho no contenga más flechas.
    const char *ladoDerecho = flecha + 2; // strlen("->") = 2
    if (strstr(ladoDerecho, "->") != NULL) {
        return false;
    }
    // Validamos que el lado derecho tenga como máximo 2 ("LADO_DERECHO_MAX") símbolos. Por ejemplo: aT
    if (strlen(ladoDerecho) > LADO_DERECHO_MAX) {
        return false;
    }
    return true;
}

Produccion parsearProduccion(const char* cadenaProduccion) {
    Produccion nuevaProduccion;
    // Le asigno su lado izquierdo.
    nuevaProduccion.ladoIzquierdo = cadenaProduccion[0];
    // Le asigno su lado derecho.
    const char *ladoDerecho = cadenaProduccion + 3;
    strncpy(nuevaProduccion.ladoDerecho, ladoDerecho, LADO_DERECHO_MAX);
    nuevaProduccion.ladoDerecho[LADO_DERECHO_MAX] = '\0'; // Lo convertimos a C String.
    return nuevaProduccion;
}

Produccion *parsearProducciones(char *cadenaProducciones, int *resultadoCantidadProducciones) {
    // Se puede saber la cantidad de producciones a través de la cantidad de comas en el String más uno.
    const int cantidadProducciones = contarCaracter(',', cadenaProducciones) + 1;
    Produccion *producciones = malloc((cantidadProducciones + 1) * sizeof(Produccion));
    if (producciones == NULL) {
        memprinterr();
        return NULL;
    }
    const char *token = strtok(cadenaProducciones, ",");
    for (int i = 0; (token != NULL && i < cantidadProducciones); i++) {
        if (!esFormatoProduccionValido(token)) {
            printerr("Formato invalido para una produccion ingresada: %s", token);
            free(producciones);
            return NULL;
        }
        producciones[i] = parsearProduccion(token);
        token = strtok(NULL, ",");
    }
    *resultadoCantidadProducciones = cantidadProducciones;
    return producciones;
}

Produccion *obtenerProducciones(int *resultadoCantidadProducciones) {
    printmsg("Ingrese las producciones separadas por comas (ej: S->aT,S->a): ");
    char *cadenaProducciones = obtenerCadenaEntrada();
    if (cadenaProducciones == NULL) {
        return NULL;
    }
    Produccion *producciones = parsearProducciones(cadenaProducciones, resultadoCantidadProducciones);
    free(cadenaProducciones);
    return producciones;
}

void inicializarGramatica(Gramatica* gramatica) {
    gramatica->simbolosNoTerminales = NULL;
    gramatica->simbolosTerminales = NULL;
    gramatica->producciones = NULL;
    gramatica->axioma = '\0';
    gramatica->cantidadProducciones = 0;
}

Gramatica *crearGramatica() {
    Gramatica *nuevaGramatica = malloc(sizeof(Gramatica));
    if (nuevaGramatica == NULL) {
        memprinterr();
        return NULL;
    }
    inicializarGramatica(nuevaGramatica); // Para garantizar valores coherentes.
    nuevaGramatica->simbolosNoTerminales = obtenerSimbolosNoTerminales();
    if (nuevaGramatica->simbolosNoTerminales == NULL) {
        destruirGramatica(nuevaGramatica);
        return NULL;
    }
    nuevaGramatica->simbolosTerminales = obtenerSimbolosTerminales();
    if (nuevaGramatica->simbolosTerminales == NULL) {
        destruirGramatica(nuevaGramatica);
        return NULL;
    }
    nuevaGramatica->producciones = obtenerProducciones(&nuevaGramatica->cantidadProducciones);
    if (nuevaGramatica->producciones == NULL) {
        destruirGramatica(nuevaGramatica);
        return NULL;
    }
    nuevaGramatica->axioma = obtenerAxioma();
    if (nuevaGramatica->axioma == '\0') {
        destruirGramatica(nuevaGramatica);
        return NULL;
    }
    return nuevaGramatica;
}

void mostrarGramatica(const Gramatica *gramatica) {
    printmsg("\nGramatica regular ingresada:\n");
    printmsg("GR = ({%s},{%s},{", gramatica->simbolosNoTerminales, gramatica->simbolosTerminales);
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        printmsg("%c->%s", gramatica->producciones[i].ladoIzquierdo, gramatica->producciones[i].ladoDerecho);
        if (i + 1 < gramatica->cantidadProducciones) {
            printmsg(",");
        }
    }
    printmsg("},%c)\n", gramatica->axioma);
}

bool esAxiomaSimboloNoTerminal(const Gramatica *gramatica) {
    return contieneCaracter(gramatica->axioma,gramatica->simbolosNoTerminales);
}

bool sonLadosIzquierdosSimbolosNoTerminales(const Gramatica *gramatica) {
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char ladoIzquierdoProduccion = gramatica->producciones[i].ladoIzquierdo;
        if (!contieneCaracter(ladoIzquierdoProduccion, SIMBOLOS_NO_TERMINALES)) {
            printerr("El lado izquierdo de la produccion %d no es un simbolo no terminal: %c.", i++, ladoIzquierdoProduccion);
            return false;
        }
    }
    return true;
}

bool esLinealAIzquierda(const char* ladoDerecho, const Gramatica *gramatica) {
    return (contieneCaracter(ladoDerecho[0],gramatica->simbolosTerminales) && contieneCaracter(ladoDerecho[1],gramatica->simbolosNoTerminales));
}

bool esLinealADerecha(const char* ladoDerecho, const Gramatica *gramatica) {
    return (contieneCaracter(ladoDerecho[0],gramatica->simbolosNoTerminales) && contieneCaracter(ladoDerecho[1],gramatica->simbolosTerminales));
}

/*
        Las gramáticas regulares establecen que los lados derechos de las
        producciones solo pueden ser alguna de las siguientes combinaciones:

        - Un símbolo terminal (o EPSILON).
        - Un símbolo terminal seguido de un símbolo no terminal (Lineal a derecha)
        - Un símbolo no terminal seguido de un símbolo no terminal (Lineal a izquierda)
 */
bool cumpleRestriccionesGramaticaRegular(const char *ladoDerecho, const Gramatica *gramatica) {
    const char* simbolosTerminales = gramatica->simbolosTerminales;
    const size_t longitudLadoDerecho = strlen(ladoDerecho);
    // Primer caso: Si se trata de un solo símbolo, este debe ser terminal o ser Epsilon.
    if (longitudLadoDerecho == 1) {
        return contieneCaracter(ladoDerecho[0],simbolosTerminales) || ladoDerecho[0] == EPSILON;
    }
    // Segundo caso: Si se trata de dos símbolos, debe ser alguna de las combinaciones mencionadas anteriormente.
    if (longitudLadoDerecho == 2) {
        return esLinealADerecha(ladoDerecho,gramatica) || esLinealAIzquierda(ladoDerecho,gramatica);
    }
    // Si el lado derecho no contiene una cantidad diferente de símbolos entonces no cumple. (No debería evaluarse).
    return false;
}

bool sonLadosDerechosValidos(const Gramatica *gramatica) {
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char *ladoDerechoProduccion = gramatica->producciones[i].ladoDerecho;
        if (!cumpleRestriccionesGramaticaRegular(ladoDerechoProduccion, gramatica)) {
            printerr("El lado derecho de la produccion %d no cumple con las restricciones de las gramaticas regulares: %s.",i++, ladoDerechoProduccion);
            return false;
        }
    }
    return true;
}

bool tieneLinealidadUnica(const Gramatica *gramatica) {
    bool tieneLinealidadADerecha = false;
    bool tieneLinealidadAIzquierda = false;
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char *ladoDerecho = gramatica->producciones[i].ladoDerecho;
        const size_t longitudLadoDerecho = strlen(ladoDerecho);
        // Solo verificamos producciones de 2 símbolos (las de 1 símbolo no afectan la linealidad)
        if (longitudLadoDerecho == 2) {
            tieneLinealidadADerecha = esLinealADerecha(ladoDerecho,gramatica);
            tieneLinealidadAIzquierda = esLinealAIzquierda(ladoDerecho,gramatica);
            // Si tiene linealidad a izquierda y a derecha simultáneamente, no es regular.
            if (tieneLinealidadADerecha && tieneLinealidadAIzquierda) {
                printerr("La gramatica contiene producciones lineales a derecha e izquierda simultaneamente.");
                return false;
            }
        }
    }
    // Si llegamos acá, la gramática tiene linealidad única (o no tiene producciones de 2 símbolos).
    return true;
}

#define CANTIDAD_LETRAS 52

bool seUsaEpsilonCorrectamente(const Gramatica *gramatica) {
    bool simbolosConEpsilon[CANTIDAD_LETRAS] = {false};
    // Marcamos los simbolos no terminales que producen epsilon.
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char* ladoDerecho = gramatica->producciones[i].ladoDerecho;
        if (strlen(ladoDerecho) == 1 && ladoDerecho[0] == EPSILON) {
            const char ladoIzquierdo = gramatica->producciones[i].ladoIzquierdo;
            simbolosConEpsilon[(int)ladoIzquierdo] = true;
        }
    }
    // Verificamos que no vuelvan a aparecer en el lado derecho de otras producciones.
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char* ladoDerecho = gramatica->producciones[i].ladoDerecho;
        const size_t longitudLadoDerecho = strlen(ladoDerecho);
        for (int j = 0; j < longitudLadoDerecho; j++) {
            const char simboloActual = ladoDerecho[j];
            if (simbolosConEpsilon[(int)simboloActual]) {
                printerr("El simbolo '%c' produce epsilon pero aparece en el lado derecho de otra produccion.\n",ladoDerecho[j]);
                return false;
            }
        }
    }
    return true;
}

bool cumpleValidaciones(const Gramatica *gramatica) {
    if (gramatica == NULL || gramatica->producciones == NULL) {
        return false;
    }
    // 1) Verificar que el axioma pertenezca al conjunto de símbolos no terminales.
    if (!esAxiomaSimboloNoTerminal(gramatica)) {
        printerr("El axioma ingresado no es valido. Motivo: No pertenece al conjunto de simbolos no terminales de la gramatica.");
        return false;
    }
    // 2) Verificar que los lados izquierdos de las producciones sean únicamente símbolos no terminales.
    if (!sonLadosIzquierdosSimbolosNoTerminales(gramatica)) {
        return false;
    }
    // 3) Verificar que los lados derechos de las producciones cumplan con las restricciones de las gramáticas regulares.
    if (!sonLadosDerechosValidos(gramatica)) {
        return false;
    }
    // 4) Verificar que la gramática sea únicamente lineal a izquierda o a derecha, no ambas.
    if (!tieneLinealidadUnica(gramatica)) {
        return false;
    }
    // 5) Verificar que se si se usa Epsilon, sea de manera correcta.
    if (!seUsaEpsilonCorrectamente(gramatica)) {
        return false;
    }
    // Si todas las validaciones anteriores se cumplieron, la gramática ingresada es regular.
    return true;
}

bool esGramaticaRegular(const Gramatica *gramatica) {
    return cumpleValidaciones(gramatica);
}

// -----------

char buscarNoTerminal(const char* cadenaDerivacion, const char* simbolosNoTerminales) {
    if (cadenaDerivacion == NULL) {
        return '\0';
    }
    for (int i = 0; cadenaDerivacion[i] != '\0'; i++) {
        if (contieneCaracter(cadenaDerivacion[i],simbolosNoTerminales)) {
            return cadenaDerivacion[i];
        }
    }
    return '\0';
}

Produccion* obtenerProduccionesDeNoTerminal(const Gramatica* gramatica, const char noTerminalActual, int* cantidadProduccionesEncontradas) {

    int cantidadEncontradas = 0;

    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        if (gramatica->producciones[i].ladoIzquierdo == noTerminalActual) {
            cantidadEncontradas++;
        }
    }

    Produccion* produccionesNoTerminal = malloc(cantidadEncontradas * sizeof(Produccion));
    if (produccionesNoTerminal == NULL) {
        memprinterr();
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        if (gramatica->producciones[i].ladoIzquierdo == noTerminalActual) {
            produccionesNoTerminal[j++] = gramatica->producciones[i];
        }
    }

    *cantidadProduccionesEncontradas = cantidadEncontradas;
    return produccionesNoTerminal;
}

char* aplicarDerivacion(const char* cadena, char noTerminal, const char* reemplazo) {
    size_t lenCadena = strlen(cadena);
    size_t lenReemplazo = strlen(reemplazo);

    char* nuevaCadena = malloc(lenCadena + lenReemplazo + 1);

    bool reemplazoHecho = false;
    int pos = 0;

    for (int i = 0; cadena[i] != '\0'; i++) {
        if (!reemplazoHecho && cadena[i] == noTerminal) {
            strcpy(&nuevaCadena[pos], reemplazo);
            pos += lenReemplazo;
            reemplazoHecho = true;
        } else {
            nuevaCadena[pos++] = cadena[i];
        }
    }

    nuevaCadena[pos] = '\0';
    return nuevaCadena;
}

char* generarPalabraAleatoria(const Gramatica* gramatica) {

    srand(time(NULL));

    char* cadenaDerivacion = malloc(2*sizeof(char));
    if (cadenaDerivacion == NULL) {
        memprinterr();
        return;
    }

    cadenaDerivacion[0] = gramatica->axioma;
    cadenaDerivacion[1] = '\0';

    printf("\nDerivacion: %s", cadenaDerivacion);

    char noTerminalActual;

    while ((noTerminalActual = buscarNoTerminal(cadenaDerivacion,gramatica->simbolosNoTerminales))) {
        int cantidadProducciones;
        Produccion* produccionesDeNoTerminal = obtenerProduccionesDeNoTerminal(gramatica,noTerminalActual,&cantidadProducciones);

        const int indiceAleatorio = rand() % cantidadProducciones;
        const Produccion produccionElegida = produccionesDeNoTerminal[indiceAleatorio];

        char* nuevaCadena = aplicarDerivacion(cadenaDerivacion, noTerminalActual, produccionElegida.ladoDerecho);
        free(cadenaDerivacion);
        cadenaDerivacion = nuevaCadena;

        printf(" -> %s", cadenaDerivacion);

        free(produccionesDeNoTerminal);
    }

    printf("\n\n");
    free(cadenaDerivacion);

    return NULL;
}

int main(void) {

    printmsg("Generador de palabras aleatorias - Grupo 10\n\n");

    Gramatica *gramatica = crearGramatica();
    if (gramatica == NULL) {
        return -1;
    }

    if (esGramaticaRegular(gramatica)) {
        mostrarGramatica(gramatica);
        generarPalabraAleatoria(gramatica);
    } else {
        printerr("La gramatica ingresada no es regular\n");
    }

    destruirGramatica(gramatica);
    return 0;
}
