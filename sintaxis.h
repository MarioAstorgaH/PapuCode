#ifndef SINTAXIS_H
#define SINTAXIS_H



#include <iostream>
#include <vector>
#include "lexico.h"

using namespace std;



//-----------------------------constantes de error (SINTAXIS) (SEMANTICA)
#define ERR_NO_SINTAX_ERROR                         0
#define ERR_IDENTIFICADOR                           1
#define ERR_EOLN                                    2
#define ERR_INICIO                                  3
#define ERR_FINAL                                   4
#define ERR_FINDEF                                  5
#define ERR_PARENTESIS_ABRIR                        6
#define ERR_PARENTESIS_CERRAR                       7
#define ERR_OP_LOGICO                               8
#define ERR_FINMI                                   9
#define ERR_CONDICION                               10
#define ERR_FINSI                                   11

#define ERR_SEMANTICA_NO_ERROR                      0
#define ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE       101
#define ERR_SEMANTICA_IDENTIFICADOR_NO_DECL         102
#define ERR_SEMANTICA_FUNCION_NO_DECL               103
#define ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO       104
#define ERR_SEMANTICA_IDENT_FUNCION_MAL_USO         105



class Sintaxis 
{
private :
    Lexico lexico;
    vector<tToken> lstTokens;
    int iToken;
    tToken tokActual;
    vector<tToken> tablaSimbolos;
public:
    Sintaxis(Lexico lex);
    ~Sintaxis();

    int generaSintaxis();
    void sigToken();

    int procPrincipal();
    int procInstrucciones();
    int procDefCiclo();
    int procDefSi();
    int procDefMientras();
    int procDefSalida();
    int procDefEntrada();
    int procDefCondicion();
    int procDefExpresion();
    int procDefIdentificador();
    
    string mensajeError(int err);
    
    int agregarIdentificador(string iden, int tipo, int linea);
    vector<tToken> getListaIdentificadores();
    int getTipoIdentificador(string iden);
    string getStrTipoIdentificador(int tipo);
};


#endif