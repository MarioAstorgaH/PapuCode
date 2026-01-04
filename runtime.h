#ifndef RUNTIME_H
#define RUNTIME_H

#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <variant> // C++17 feature util, o usamos struct clásico
#include "bytecode.h" // Necesitamos conocer tInstruccion

using namespace std;

// Estructura para manejar valores híbridos (Números y Strings) en la pila
struct Valor {
    int tipo; // 0 = Numero, 1 = Cadena
    double valNum;
    string valStr;
};

class Runtime {
private:
    vector<tInstruccion> codigo;
    stack<Valor> pila;
    map<string, Valor> memoria; // Variables: nombre -> valor
    map<string, int> etiquetas; // Saltos: nombre_label -> indice_instruccion
    int ip; // Instruction Pointer (qué linea estamos ejecutando)

    // Métodos auxiliares
    void buscarEtiquetas();
    void pushNum(double v);
    void pushStr(string s);
    Valor pop();

public:
    Runtime(vector<tInstruccion> code);
    void run();
};

#endif