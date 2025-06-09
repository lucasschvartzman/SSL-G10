#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

#define malloc_printerr() \
    if(DESARROLLO) printerr("La funcion malloc() fallo cuando se ejecuto en: %s()\n",__func__)

// Una gramática regular tiene como máximo 2 símbolos en su lado derecho.
#define LADO_DERECHO_MAX 2

#define PRODUCCION_MIN 4

#define STDIN_BUFFER_SIZE 1024

#define EPSILON '@'

#define SIMBOLOS_NO_TERMINALES "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SIMBOLOS_TERMINALES "abcdefghijklmnopqrstuvwxyz"

// --- Utils ---

char *obtenerCadenaEntrada() {
    char buffer[STDIN_BUFFER_SIZE];
    fgets(buffer, sizeof(buffer),stdin);
    const size_t cantidadSimbolos = strcspn(buffer, "\n");
    buffer[cantidadSimbolos] = '\0';
    char *cadena = malloc((cantidadSimbolos + 1) * sizeof(char));
    if (cadena == NULL) {
        malloc_printerr();
        return NULL;
    }
    strcpy(cadena, buffer);
    return cadena;
}

void sanitizarBufferEntrada() {
    for (int c; (c = getchar()) != '\n' && c != EOF;);
}

char obtenerCaracterEntrada() {
    const char caracter = (char) getchar();
    if (caracter != '\n') {
        sanitizarBufferEntrada();
    }
    return caracter;
}

bool contieneCaracter(const char *cadena, const char caracter) {
    return strchr(cadena, caracter) != NULL;
}

bool tieneSoloSimbolosConjunto(const char *cadena, const char *conjunto) {
    if (cadena == NULL || conjunto == NULL) {
        return false;
    }
    for (int i = 0; cadena[i] != '\0'; i++) {
        if (!contieneCaracter(conjunto, cadena[i])) {
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

// --- Estructuras ---

typedef struct {
    char ladoIzquierdo;
    char ladoDerecho[LADO_DERECHO_MAX + 1];
} Produccion;

typedef struct {
    char *simbolosNoTerminales;
    char *simbolosTerminales;
    Produccion *producciones;
    int cantidadProducciones;
    char axioma;
} Gramatica;

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
    if (!tieneSoloSimbolosConjunto(simbolosNoTerminales,SIMBOLOS_NO_TERMINALES)) {
        printerr("Ha ingresado simbolos no terminales incorrectos (por ej. ',' o una minuscula)");
        free(simbolosNoTerminales);
        return NULL;
    }
    return simbolosNoTerminales;
}

char *obtenerSimbolosTerminales() {
    printmsg("Ingrese los simbolos terminales (sin espacios, ni comas): ");
    char *simbolosTerminales = obtenerCadenaEntrada();
    if (!tieneSoloSimbolosConjunto(simbolosTerminales,SIMBOLOS_TERMINALES)) {
        printerr("Ha ingresado simbolos terminales incorrectos (por ej. ',' o una mayuscula)");
        free(simbolosTerminales);
        return NULL;
    }
    return simbolosTerminales;
}

char obtenerAxioma() {
    printmsg("Ingrese el axioma (en mayuscula): ");
    const char axioma = obtenerCaracterEntrada();
    if (!contieneCaracter(SIMBOLOS_NO_TERMINALES, axioma)) {
        printerr("Ha ingresado un caracter que no puede ser axioma.");
        return -1;
    }
    return axioma;
}

bool esFormatoProduccionValido(const char *produccion) {
    if (produccion == NULL) {
        return false;
    }

    const size_t longitud = strlen(produccion);

    // Validamos que la cadena ingresada tenga como mínimo 4 ("PRODUCCION_MIN") caracteres. Por ej: "S->x"
    if (longitud < PRODUCCION_MIN) {
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

    // Validamos que el lado derecho tenga como máximo 2 ("LADO_DERECHO_MAX") símbolos. Ej: aT
    if (strlen(ladoDerecho) > LADO_DERECHO_MAX) {
        return false;
    }

    return true;
}

bool parseProduccion(const char *cadenaProduccion, Produccion *produccion) {
    if (!esFormatoProduccionValido(cadenaProduccion)) {
        return false;
    }

    produccion->ladoIzquierdo = cadenaProduccion[0];

    const char *ladoDerecho = cadenaProduccion + 3;
    strncpy(produccion->ladoDerecho, ladoDerecho, LADO_DERECHO_MAX);
    produccion->ladoDerecho[LADO_DERECHO_MAX] = '\0';

    return true;
}

Produccion *parseProducciones(char *cadenaProducciones, int *resultadoCantidadProducciones) {
    const int cantidadProducciones = contarCaracter(',', cadenaProducciones) + 1;
    Produccion *producciones = malloc((cantidadProducciones + 1) * sizeof(Produccion));
    if (producciones == NULL) {
        malloc_printerr();
        return NULL;
    }
    const char *token = strtok(cadenaProducciones, ",");
    for (int i = 0; (token != NULL && i < cantidadProducciones); i++) {
        if (!parseProduccion(token, &producciones[i])) {
            printerr("Formato invalido para una produccion ingresada: %s", token);
            free(producciones);
            return NULL;
        }
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
    Produccion *producciones = parseProducciones(cadenaProducciones, resultadoCantidadProducciones);
    free(cadenaProducciones);
    return producciones;
}

Gramatica *crearGramatica() {
    Gramatica *nuevaGramatica = malloc(sizeof(Gramatica));
    if (nuevaGramatica == NULL) {
        malloc_printerr();
        return NULL;
    }

    nuevaGramatica->simbolosNoTerminales = NULL;
    nuevaGramatica->simbolosTerminales = NULL;
    nuevaGramatica->producciones = NULL;

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
    if (nuevaGramatica->axioma == -1) {
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
        if (i+1 < gramatica->cantidadProducciones) {
            printmsg(",");
        }
    }
    printmsg("},%c)\n", gramatica->axioma);
}

bool validarLadosIzquierdos(const Gramatica *gramatica) {
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char ladoIzquierdo = gramatica->producciones->ladoIzquierdo;
        if (!contieneCaracter(SIMBOLOS_NO_TERMINALES, ladoIzquierdo)) {
            printerr(
                "El lado izquierdo de la produccion %d no cumple con las restricciones de las gramaticas regulares: %c.",
                i+1, ladoIzquierdo);
            return false;
        }
    }
    return true;
}

bool esProduccionRegular(const char *ladoDerecho, const char *terminales, const char *noTerminales) {
    const size_t longitud = strlen(ladoDerecho);

    // En caso de que tenga un solo simbolo, que sea un terminal o EPSILON.
    if (longitud == 1) {
        return contieneCaracter(terminales, ladoDerecho[0])
               || ladoDerecho[0] == EPSILON;
    }

    // En caso de que tenga dos simbolos, que sea un terminal-noTerminal o noTerminal-terminal.
    if (longitud == 2) {
        return
                (contieneCaracter(terminales, ladoDerecho[0]) && contieneCaracter(noTerminales, ladoDerecho[1]))
                || (contieneCaracter(noTerminales, ladoDerecho[0]) && contieneCaracter(terminales, ladoDerecho[1]));
    }

    return false;
}

bool validarLadosDerechos(const Gramatica *gramatica) {
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char *ladoDerecho = gramatica->producciones[i].ladoDerecho;

        if (!esProduccionRegular(ladoDerecho, gramatica->simbolosTerminales, gramatica->simbolosNoTerminales)) {
            printerr(
                "El lado derecho de la produccion %d no cumple con las restricciones de las gramaticas regulares: '%s'.",
                i+1, ladoDerecho);
            return false;
        }
    }
    return true;
}

bool validarEpsilonRestriccion(const Gramatica *gramatica) {
    bool simbolosConEpsilon[1024] = {false}; // Array para marcar símbolos

    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        if (strlen(gramatica->producciones[i].ladoDerecho) == 1 && gramatica->producciones[i].ladoDerecho[0] ==
            EPSILON) {
            simbolosConEpsilon[(int) gramatica->producciones[i].ladoIzquierdo] = true;
        }
    }

    // Verificamos que los símbolos terminales que produjeron EPSILON no aparezcan en el lado derecho de otras producciones.
    for (int i = 0; i < gramatica->cantidadProducciones; i++) {
        const char *ladoDerecho = gramatica->producciones[i].ladoDerecho;
        const size_t longitud = strlen(ladoDerecho);

        for (int j = 0; j < longitud; j++) {
            if (simbolosConEpsilon[(int) ladoDerecho[j]]) {
                printerr("El simbolo '%c' produce epsilon pero aparece en el lado derecho de otra produccion.\n",
                         ladoDerecho[j]);
                return false;
            }
        }
    }
    return true;
}

bool validarProducciones(const Gramatica *gramatica) {
    return validarLadosIzquierdos(gramatica) && validarLadosDerechos(gramatica) && validarEpsilonRestriccion(gramatica);
}

bool esGramaticaRegular(const Gramatica *gramatica) {
    if (!contieneCaracter(gramatica->simbolosNoTerminales, gramatica->axioma)) {
        printerr("El axioma ingresado no pertenece al conjunto de simbolos no terminales");
        return false;
    }

    if (!validarProducciones(gramatica)) {
        return false;
    }

    return true;
}

char buscarNoTerminal(const char* cadenaDerivacion, const char* simbolosNoTerminales) {
    for (int i = 0; cadenaDerivacion[i] != '\0'; i++) {
        if (contieneCaracter(simbolosNoTerminales,cadenaDerivacion[i])) {
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
        malloc_printerr();
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

void generarPalabraAleatoria(const Gramatica* gramatica) {

    srand(time(NULL));

    char* cadenaDerivacion = malloc(2*sizeof(char));
    if (cadenaDerivacion == NULL) {
        malloc_printerr();
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
        Produccion produccionElegida = produccionesDeNoTerminal[indiceAleatorio];


        char* nuevaCadena = aplicarDerivacion(cadenaDerivacion, noTerminalActual, produccionElegida.ladoDerecho);
        free(cadenaDerivacion);
        cadenaDerivacion = nuevaCadena;


        printf(" -> %s", cadenaDerivacion);

        free(produccionesDeNoTerminal);
    }

    printf("\n\n");
    free(cadenaDerivacion);
}

int main(void) {
    printmsg("Generador de palabras aleatorias - Grupo 10\n\n");

    Gramatica *gramatica = crearGramatica();
    if (gramatica == NULL) {
        exit(EXIT_FAILURE);
    }

    if (esGramaticaRegular(gramatica)) {
        mostrarGramatica(gramatica);
        generarPalabraAleatoria(gramatica);
    } else {
        printerr("La gramatica ingresada no es regular\n");
    }

    destruirGramatica(gramatica);
    exit(EXIT_SUCCESS);
}
