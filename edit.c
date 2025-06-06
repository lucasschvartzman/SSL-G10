#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//gcc "Generador de lenguajes.c" -o "Generador de lenguajes.exe"

typedef struct gram
{
    char tproducciones [10][10];
    char noTerminales [5];
    char terminales [5];
    char axioma;
} gramatica;

void cargarTerminales (gramatica *g)
{
    strcpy (g->terminales, "abc");
}

void cargarNoTerminales (gramatica *g)
{
    strcpy (g->noTerminales, "STQ");
}

void cargaAxioma (gramatica *g)
{
    g->axioma = 'S';
}

/*STR REPLACE:
Postcondiciones: Se debe hacer un free de result si no es nulo
*/

char *str_replace (char *orig, char*rep, char*with) {
    char *result; //string resultado
    char *ins; //puntero a proximo punto de insercion
    char *tmp; //cursor
    int len_rep; //longitud del string al ser reemplazado
    int len_with; //longitud del string reemplazo
    int len_front; //distance between rep and end of last rep
    int count; //cantidad de reemplazos

    //sanity checks and
    if (!orig || !rep)
        return NULL;
    len_rep = strlen (rep);
    if (len_rep == 0)
        return NULL;
    if (!with)
        with = "";
    len_with = strlen(with);

    //Cuenta la cantidad de reemplazos a realizar
    ins = orig;
    for (count=0; (tmp=strstr(ins, rep));++count) {
        ins = tmp + len_rep;
    }

    //Reresva espacio para realizar formar la cadena reemplazada
    tmp = result = malloc (strlen(orig) + (len_with-len_rep)*count+1);

    if (!result)
        return NULL;

    /*Copia de "a partes" la cadena original hasta la siguiente aparicióon de la cadena a reemplazar, luego copia el reemplazo y continua copiando la cadena original hasta el siguiente valor a reemplazar*/

    while (count--) {
        ins =strstr (orig,rep);
        len_front=ins-orig;
        tmp = strncpy (tmp, orig, len_front) + len_front;
        tmp = strcpy (tmp, with) + len_with;
        orig += len_front + len_rep; //move to next "end of rep"
    }

    strcpy (tmp, orig);
    return result;
}

void cargarProducciones (gramatica *g)
{
    strcpy (g->tproducciones[0], "S->aS");
    strcpy (g->tproducciones[1], "S->aT");
    strcpy (g->tproducciones[2], "S->bT");
    strcpy (g->tproducciones[3], "S->bQ");
    strcpy (g->tproducciones[4], "S->c");
    for (int i=5; i<10; i++) {
        strcpy(g->tproducciones[i], "");
    }

}

char buscarNT (char s[], gramatica g)
{
    int i = 0;
    while ((i<strlen(g.noTerminales))&&(!strchr(s,g.noTerminales[i])))
        i++;
    if (i<strlen(g.noTerminales))
    return g.noTerminales[i]; //devuelve el no terminal actual en la cadena de derivación
    return '\0';
}



// T->a, T->b, T->c
// S->aT

void derivar (gramatica g)
{
    char cadenaDerivacion [2048] = "";
    char ladoDerecho [10]="";
    int primeraNT; //La primera produccion en la tabla con ese terminal
    int cantNT; //Cantidad de producciones en la tabla con ese terminal
    int elegida; //Produccion elegida
    int i;
    srand (time(NULL)); //semilla del random
    cadenaDerivacion[0] = g.axioma;
    cadenaDerivacion[1] = '\0';
    char NTActual; //No terminal actual
    char NTActualS[3];
    while ((NTActual=buscarNT(cadenaDerivacion, g)) != '\0')
    {
        i=0;
        while (g.tproducciones[i][0] != NTActual)
            i++;
        primeraNT = i;
        while (g.tproducciones[i][0] == NTActual)
            i++;
        cantNT=i;
        //Ahora se neceesita un numero random entre primeraNT y ultimaNT

        elegida = primeraNT + (rand()%(cantNT-primeraNT)); // prod elegida
        printf ("primeraNT=%d\n", primeraNT);
        printf ("cantNT = %d\n", cantNT);
        printf("elegida=%d\n", elegida);
        strncpy (ladoDerecho, &g.tproducciones[elegida][3],strlen(g.tproducciones[elegida])-3);
        ladoDerecho[strlen(g.tproducciones[elegida])-3] = '\0';
        NTActualS[0] = NTActual;
        NTActualS[1] = '\0';
        char* cadenaReemplaza = str_replace(cadenaDerivacion, NTActualS, ladoDerecho);
        strcpy(cadenaDerivacion, cadenaReemplaza);
        free(cadenaReemplaza);
    }
    printf("cadena=%s\n", cadenaDerivacion);
    return;
}

int main ()
{
    gramatica g;
    cargaAxioma (&g);
    cargarNoTerminales(&g);
    cargarTerminales(&g);
    cargarProducciones(&g);
    derivar (g);
    return 0;
}