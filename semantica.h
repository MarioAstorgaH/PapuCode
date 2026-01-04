#ifndef SEMANTICA_H
#define SEMANTICA_H

#include <string>
#include <map>
#include "lexico.h" // <--- Aquí ya están definidos los códigos de error

using namespace std;

class Semantica {
private:
    map<string, int> tablaSimbolos;

public:
    Semantica();

    // Métodos principales
    int agregarIdentificador(string nombre, int tipo);
    int getTipoIdentificador(string nombre);
    bool existeIdentificador(string nombre);
    bool sonTiposCompatibles(int tipo1, int tipo2);
};

#endif