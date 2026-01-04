#include "semantica.h"

Semantica::Semantica() {
    // Constructor vacio
}


int Semantica::agregarIdentificador(string nombre, int tipo) {
    if (tablaSimbolos.find(nombre) != tablaSimbolos.end()) {
        return ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE;
    }
    tablaSimbolos[nombre] = tipo;
    return ERR_SEMANTICA_NO_ERROR;
}

int Semantica::getTipoIdentificador(string nombre) {
    // ¡OJO! Si usamos tablaSimbolos[nombre] aqui, C++ crea la variable
    // automaticamente si no existe. ESO ES LO QUE CAUSA EL BUG.
    // Usamos .find() para solo leer.

    auto it = tablaSimbolos.find(nombre);
    if (it != tablaSimbolos.end()) {
        return it->second; // Retornamos el tipo encontrado
    }

    return RES_NO_DECL; // Retornamos -1 indicando que no existe
}


bool Semantica::existeIdentificador(string nombre) {
    bool existe = tablaSimbolos.find(nombre) != tablaSimbolos.end();
    return existe;
}
bool Semantica::sonTiposCompatibles(int tipo1, int tipo2) {
    if (tipo1 == tipo2) return true;

    // Enteros y flotantes se pueden mezclar
    if ((tipo1 == RES_ENTERO || tipo1 == RES_FLOTANTE) &&
        (tipo2 == RES_ENTERO || tipo2 == RES_FLOTANTE)) {
        return true;
        }

    return false;
}