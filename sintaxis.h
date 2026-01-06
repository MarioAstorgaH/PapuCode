#ifndef SINTAXIS_H
#define SINTAXIS_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "lexico.h"
#include "semantica.h"
#include "bytecode.h" // <--- IMPORTAR

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
    GenBytecode generador; // <--- OBJETO GENERADOR

    vector<tToken> lstTokens;
    int iToken;
    tToken tokActual;
    std::queue<InfoError> colaErrores;

    bool realizarAnalisisSemantico;
    bool generarCodigo; // <--- FLAG NUEVO

    // Métodos internos
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
    int procDefRetornar();

    // Auxiliares
    int agregarIdentificador(string iden, int tipo, int linea);
    int getTipoIdentificador(string iden);

public:
    // Constructor actualizado
    Sintaxis(Lexico lex, bool activarSemantica, bool activarBytecode);
    ~Sintaxis();

    int generaSintaxis();
    string mensajeError(int err);
    void imprimirErrores();
    void imprimirBytecode(); // Método puente para llamar a generador.imprimir()
    // NUEVO MÉTODO
    vector<tInstruccion> getBytecodeGenerado() {
        return generador.getCodigo();
    }
};

#endif