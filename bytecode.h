#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

// Estructura de una instrucción de la Máquina de Pila
struct tInstruccion {
    string operacion; // Ej: PUSH, ADD, STORE, JMPF
    string parametro; // Ej: 10, edad, L1 (puede estar vacío)
};

class GenBytecode {
private:
    vector<tInstruccion> codigoGenerado;
    int contadorEtiquetas;

public:
    GenBytecode();
    void cerrar();
    // Agrega una instrucción a la lista
    void emitir(string op, string param = "");

    // Genera una etiqueta única (L1, L2, ...) para saltos
    string nuevaEtiqueta();

    // Muestra el código generado en consola
    void imprimir();

    // Retorna el vector (útil si luego quieres ejecutarlo)
    vector<tInstruccion> getCodigo();
private:
    ofstream archivo;
};

#endif