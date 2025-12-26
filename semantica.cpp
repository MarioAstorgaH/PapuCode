#include "semantica.h"

Semantica::Semantica() {
    // Constructor vacio por ahora
}

int Semantica::agregarIdentificador(string nombre, int tipo) {
    if (existeIdentificador(nombre)) {
        return ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE;
    }
    tToken nuevo;
    nuevo.token = nombre;
    nuevo.tipoToken = tipo;
    tablaSimbolos.push_back(nuevo);
    return ERR_NOERROR; // 0
}

int Semantica::getTipoIdentificador(string nombre) {
    for (const auto& token : tablaSimbolos) {
        if (token.token == nombre) return token.tipoToken;
    }
    return RES_NO_DECL; // Retorna codigo indicando que no existe
}

bool Semantica::existeIdentificador(string nombre) {
    return getTipoIdentificador(nombre) != RES_NO_DECL;
}

bool Semantica::sonTiposCompatibles(int tipoReceptor, int tipoValor) {
    if (tipoReceptor == tipoValor) return true;
    // Entero acepta Entero
    // Flotante acepta Flotante y Entero (casting implicito)
    if (tipoReceptor == RES_FLOTANTE && tipoValor == RES_ENTERO) return true;

    return false;
}

int Semantica::obtenerTipoResultante(int tipo1, int tipo2, string operador) {
    if (tipo1 == RES_CADENA || tipo2 == RES_CADENA) {
        if (operador == "+") return RES_CADENA; // Concatenacion
        return ERR_SEMANTICA_TIPOS_INCOMPATIBLES; // No se puede restar/mult cadenas
    }

    if (operador == "/" || tipo1 == RES_FLOTANTE || tipo2 == RES_FLOTANTE) {
        return RES_FLOTANTE;
    }

    return RES_ENTERO;
}