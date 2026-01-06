#include "runtime.h"
#include <cstdlib> // Para atof/stod

Runtime::Runtime(vector<tInstruccion> code) {
    codigo = code;
    ip = 0;
    scopes.push_back(map<string, Valor>()); // Scope Global (Indice 0)
    buscarEtiquetas();
}

// Mapea las etiquetas (L1, L2...) a sus posiciones en el vector de código
// para que los saltos (JMPF) sean rápidos.
void Runtime::buscarEtiquetas() {
    for (int i = 0; i < codigo.size(); i++) {
        if (codigo[i].operacion == "LABEL") {
            etiquetas[codigo[i].parametro] = i;
        }
    }
}

void Runtime::pushNum(double v) {
    Valor val; val.tipo = 0; val.valNum = v;
    pila.push(val);
}

void Runtime::pushStr(string s) {
    Valor val; val.tipo = 1; val.valStr = s; val.valNum = 0;
    pila.push(val);
}

Valor Runtime::pop() {
    if (pila.empty()) {
        cerr << "Error Runtime: Pila vacia (Stack Underflow)" << endl;
        exit(1);
    }
    Valor v = pila.top();
    pila.pop();
    return v;
}

void Runtime::run() {
    cout << "=======================================" << endl;
    cout << "           EJECUTANDO PROGRAMA         " << endl;
    cout << "=======================================" << endl;

    while (ip < codigo.size()) {
        string op = codigo[ip].operacion;
        string param = codigo[ip].parametro;

        cout << "[IP:" << ip << "] " << op << " " << param << endl;
        // --- MOVIMIENTO DE DATOS ---
        if (op == "PUSH") {
            if (param.size() > 0 && param[0] == '"') {
                string limpio = param.substr(1, param.size() - 2);
                pushStr(limpio);
            } else {
                pushNum(stod(param));
            }
        }
        else if (op == "LOAD") {
            // 1. Buscar en Scope Local (tope de la pila)
            if (scopes.back().count(param)) {
                pila.push(scopes.back()[param]);
            }
            // 2. Si no, buscar en Global (base de la pila)
            else if (scopes[0].count(param)) {
                pila.push(scopes[0][param]);
            }
            else {
                cerr << "Error Runtime: Variable '" << param << "' no existe." << endl;
                exit(1);
            }
        }
        else if (op == "STORE") {
            Valor v = pop();

            // Prioridad: ¿Ya existe en local? -> Actualizar local
            if (scopes.back().count(param)) {
                scopes.back()[param] = v;
            }
            // ¿Existe en global? -> Actualizar global
            else if (scopes[0].count(param)) {
                scopes[0][param] = v;
            }
            // ¿No existe? -> Crear en Local (o Global si estamos en el main)
            else {
                scopes.back()[param] = v;
            }
        }
        // STORE_STR y STORE_NUM siguen la misma lógica o usan STORE genérico
        // --- ENTRADA / SALIDA ---
        else if (op == "OUT") {
            Valor v = pop();
            if (v.tipo == 1) cout << v.valStr << endl;
            else cout << v.valNum << endl;
        }
        else if (op == "IN_STR") {
            string input;
            cout << "Ingrese texto para '" << param << "': ";
            if (cin.peek() == '\n') cin.ignore();
            getline(cin, input);

            Valor v; v.tipo = 1; v.valStr = input; v.valNum = 0;

            // --- CORRECCIÓN: USAR SCOPES EN LUGAR DE MEMORIA ---
            if (scopes.back().count(param)) {         // Existe en local?
                scopes.back()[param] = v;
            } else if (scopes[0].count(param)) {      // Existe en global?
                scopes[0][param] = v;
            } else {                                  // Nuevo -> Local
                scopes.back()[param] = v;
            }
            // ---------------------------------------------------
        }

        // LECTURA DE NUMEROS (Estricta: Falla si no es número)
        else if (op == "IN_NUM") {
            string input;
            cout << "Ingrese numero para '" << param << "': ";

            if (cin.peek() == '\n') cin.ignore();
            cin >> input;

            try {
                size_t idx;
                double d = stod(input, &idx);

                // Si sobran caracteres (ej: "12abc"), es un error fatal
                if (idx != input.size()) {
                    cerr << "Error Runtime: La entrada '" << input << "' no es un numero valido." << endl;
                    exit(1); // <--- MATA EL PROGRAMA
                }

                // Si todo bien, guardamos

                Valor v; v.tipo = 0; v.valNum = d;

                // --- CORRECCIÓN: USAR SCOPES EN LUGAR DE MEMORIA ---
                if (scopes.back().count(param)) {         // Existe en local?
                    scopes.back()[param] = v;
                } else if (scopes[0].count(param)) {      // Existe en global?
                    scopes[0][param] = v;
                } else {                                  // Nuevo -> Local
                    scopes.back()[param] = v;
                }

            } catch (...) {
                // Si la conversión falla totalmente (ej: "hola"), error fatal
                cerr << "Error Runtime: Tipo incorrecto. Se esperaba un numero para variable '" << param << "'." << endl;
                exit(1); // <--- MATA EL PROGRAMA
            }
        }
        // --- ARITMETICA ---
        else if (op == "ADD") {
            Valor b = pop(); Valor a = pop();
            if (a.tipo == 0 && b.tipo == 0) pushNum(a.valNum + b.valNum);
            else {
                string sa = (a.tipo==1) ? a.valStr : to_string(a.valNum);
                string sb = (b.tipo==1) ? b.valStr : to_string(b.valNum);
                pushStr(sa + sb);
            }
        }
        else if (op == "SUB") {
            Valor b = pop(); Valor a = pop();
            // Validar que ambos sean números
            if (a.tipo == 1 || b.tipo == 1) {
                cerr << "Error Runtime: No se pueden restar cadenas de texto." << endl;
                exit(1);
            }
            pushNum(a.valNum - b.valNum);
        }
        else if (op == "MUL") {
            Valor b = pop(); Valor a = pop();
            pushNum(a.valNum * b.valNum);
            if (a.tipo == 1 || b.tipo == 1) {
                cerr << "Error Runtime: No se pueden restar cadenas de texto." << endl;
                exit(1);
            }
        }
        else if (op == "DIV") {
            Valor b = pop(); Valor a = pop();
            if (a.tipo == 1 || b.tipo == 1) {
                cerr << "Error Runtime: No se pueden restar cadenas de texto." << endl;
                exit(1);
            }
            if (b.valNum == 0) { cerr << "Error: Division por cero" << endl; exit(1); }
            pushNum(a.valNum / b.valNum);
        }
        // --- LOGICA Y SALTOS (AQUI ESTABA EL FALTANTE) ---
        else if (op == "GT") { // >
            Valor b = pop(); Valor a = pop();
            pushNum(a.valNum > b.valNum ? 1 : 0);
        }
        else if (op == "LT") { // <
            Valor b = pop(); Valor a = pop();
            pushNum(a.valNum < b.valNum ? 1 : 0);
        }
        else if (op == "GTE") { // >= (ESTE FALTABA)
            Valor b = pop(); Valor a = pop();
            pushNum(a.valNum >= b.valNum ? 1 : 0);
        }
        else if (op == "LTE") { // <= (ESTE FALTABA)
            Valor b = pop(); Valor a = pop();
            pushNum(a.valNum <= b.valNum ? 1 : 0);
        }
        else if (op == "EQ") { // ==
            Valor b = pop(); Valor a = pop();
            if (a.tipo != b.tipo) pushNum(0);
            else if (a.tipo == 0) pushNum(a.valNum == b.valNum ? 1 : 0);
            else pushNum(a.valStr == b.valStr ? 1 : 0);
        }
        else if (op == "NE") { // <> (ESTE FALTABA)
            Valor b = pop(); Valor a = pop();
            if (a.tipo != b.tipo) pushNum(1);
            else if (a.tipo == 0) pushNum(a.valNum != b.valNum ? 1 : 0);
            else pushNum(a.valStr != b.valStr ? 1 : 0);
        }
        else if (op == "JMPF") {
            Valor cond = pop();
            if (cond.valNum == 0) { // Si es Falso (0)
                if (etiquetas.find(param) != etiquetas.end()) {
                    ip = etiquetas[param];
                    continue; // Salto directo
                } else {
                    cerr << "Error: Etiqueta no encontrada " << param << endl;
                    exit(1);
                }
            }
        }
        // ... (Junto a JMPF) ...
        else if (op == "JMP") {
            if (etiquetas.find(param) != etiquetas.end()) {
                ip = etiquetas[param];
                continue;
            } else {
                cerr << "Error: Etiqueta " << param << " no encontrada" << endl;
                exit(1);
            }
        }
        else if (op == "LABEL") {
            // Nada
        }
        // ===============================================
        // FUNCIONES (CALL y RET)
        // ===============================================
        else if (op == "CALL") {
            if (etiquetas.count(param)) {
                pilaLlamadas.push(ip + 1);
                ip = etiquetas[param];

                // --- NUEVO: CREAR SCOPE VACÍO PARA LA FUNCIÓN ---
                scopes.push_back(map<string, Valor>());

                continue;
            } // ... error else ...
        }
        else if (op == "RET") {
            if (pilaLlamadas.empty()) { /* Error */ }

            int retorno = pilaLlamadas.top();
            pilaLlamadas.pop();
            ip = retorno;

            // --- NUEVO: BORRAR SCOPE DE LA FUNCIÓN ---
            // (Solo si no estamos en el scope global, por seguridad)
            if (scopes.size() > 1) {
                scopes.pop_back();
            }

            continue;
        }
        ip++;
    }
    
    cout << "=======================================" << endl;
    cout << "           FIN DEL PROGRAMA            " << endl;
    cout << "=======================================" << endl;
}