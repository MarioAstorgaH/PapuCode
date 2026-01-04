#include "runtime.h"
#include <cstdlib> // Para atof/stod

Runtime::Runtime(vector<tInstruccion> code) {
    codigo = code;
    ip = 0;
    buscarEtiquetas(); // Pre-calculamos a dónde saltar
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
            if (memoria.find(param) != memoria.end()) {
                pila.push(memoria[param]);
            } else {
                cerr << "Error Runtime: Variable no inicializada '" << param << "'" << endl;
                exit(1);
            }
        }
        else if (op == "STORE") {
            Valor v = pop();
            memoria[param] = v;
        }
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
            memoria[param] = v;
        }

        // LECTURA DE NUMEROS (Validación estricta)
        else if (op == "IN_NUM") {
            string input;
            double d;
            bool esNumero = false;

            // Bucle hasta que el usuario escriba un numero valido
            while (!esNumero) {
                cout << "Ingrese numero para '" << param << "': ";
                if (cin.peek() == '\n') cin.ignore();
                cin >> input; // Usamos cin normal para numeros para evitar problemas con espacios

                try {
                    size_t idx;
                    d = stod(input, &idx);
                    // Verificamos que no haya letras despues del numero
                    if (idx == input.size()) {
                        esNumero = true;
                    } else {
                        cout << "Error: '" << input << "' no es un numero valido. Intente de nuevo." << endl;
                    }
                } catch (...) {
                    cout << "Error: Entrada invalida. Se esperaba un numero." << endl;
                }
            }

            Valor v; v.tipo = 0; v.valNum = d;
            memoria[param] = v;
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
        else if (op == "LABEL") {
            // Nada
        }

        ip++;
    }
    
    cout << "=======================================" << endl;
    cout << "           FIN DEL PROGRAMA            " << endl;
    cout << "=======================================" << endl;
}