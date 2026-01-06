#ifndef RUNTIME_H
#define RUNTIME_H

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <stack> // <--- AGREGAR ESTO
#include "bytecode.h"

using namespace std;

// Estructura para valores (Entero o Cadena)
struct Valor {
    int tipo; // 0=Num, 1=Str
    double valNum;
    string valStr;
};

class Runtime {
public:
    Runtime(vector<tInstruccion> code);
    void run();

private:
    vector<tInstruccion> codigo;
    int ip; // Instruction Pointer
    vector<map<string, Valor>> scopes;
    stack<Valor> pila;

    // --- NUEVO: PILA PARA FUNCIONES ---
    stack<int> pilaLlamadas;
    // ----------------------------------

    map<string, int> etiquetas; // Mapa de etiquetas para saltos rápidos

    void buscarEtiquetas();
    void pushNum(double v);
    void pushStr(string s);
    Valor pop();
};

#endif