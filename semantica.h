#ifndef SEMANTICA_H
#define SEMANTICA_H

#include <string>
#include <vector>
#include "lexico.h" // Necesitamos la definicion de tToken y codigos de error

using namespace std;

class Semantica {
private:
    vector<tToken> tablaSimbolos; // La tabla vive aqui ahora

public:
    Semantica();
    
    // Métodos para gestionar variables
    int agregarIdentificador(string nombre, int tipo);
    int getTipoIdentificador(string nombre);
    bool existeIdentificador(string nombre);
    
    // Métodos de validación de tipos
    bool sonTiposCompatibles(int tipoVariable, int tipoExpresion);
    int obtenerTipoResultante(int tipo1, int tipo2, string operador);
};

#endif