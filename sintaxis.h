#ifndef SINTAXIS_H
#define SINTAXIS_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "lexico.h"     // <--- Aquí ya vienen todos los #define
#include "semantica.h"

using namespace std;

struct InfoError {
    int codigo;
    string mensaje;
    int linea;
};

class Sintaxis {
private:
    Lexico lexico;
    Semantica semantica;
    vector<tToken> lstTokens;
    int iToken;
    tToken tokActual;
    std::queue<InfoError> colaErrores;

    // Métodos de utilidad interna
    void sigToken();
    void registrarError(int codigo);
    void sincronizar();

    // Métodos de la gramática
    int procPrincipal();
    int procInstrucciones();
    int procDefCiclo();
    int procDefSi();
    int procDefMientras();
    int procDefSalida();
    int procDefEntrada();
    int procDefCondicion();
    int procDefExpresion(int &tipoResultado);
    int procDefIdentificador();

    // Métodos auxiliares
    int agregarIdentificador(string iden, int tipo, int linea);
    vector<tToken> getListaIdentificadores();
    int getTipoIdentificador(string iden);
    string getStrTipoIdentificador(int tipo);

public:
    Sintaxis(Lexico lex);
    ~Sintaxis();

    int generaSintaxis();
    string mensajeError(int err);
    void imprimirErrores();
};

#endif