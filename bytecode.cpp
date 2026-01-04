#include "bytecode.h"

GenBytecode::GenBytecode() {
    contadorEtiquetas = 0;
    codigoGenerado.clear();
}

void GenBytecode::emitir(string op, string param) {
    tInstruccion inst;
    inst.operacion = op;
    inst.parametro = param;
    codigoGenerado.push_back(inst);
}

string GenBytecode::nuevaEtiqueta() {
    contadorEtiquetas++;
    return "L" + to_string(contadorEtiquetas);
}

void GenBytecode::imprimir() {
    cout << "=======================================" << endl;
    cout << "       BYTECODE GENERADO (STACK)       " << endl;
    cout << "=======================================" << endl;
    
    for (size_t i = 0; i < codigoGenerado.size(); i++) {
        cout << i << ":\t" << codigoGenerado[i].operacion;
        
        if (!codigoGenerado[i].parametro.empty()) {
            cout << "\t" << codigoGenerado[i].parametro;
        }
        
        // Si es una etiqueta (LABEL), la marcamos visualmente
        if (codigoGenerado[i].operacion == "LABEL") {
            cout << ":";
        }
        
        cout << endl;
    }
    cout << "=======================================" << endl;
}

vector<tInstruccion> GenBytecode::getCodigo() {
    return codigoGenerado;
}