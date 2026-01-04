#include "sintaxis.h"

// Constructor que inicializa los flags de semántica y bytecode
Sintaxis::Sintaxis(Lexico lex, bool activarSemantica, bool activarBytecode)
{
    lexico = lex;
    lstTokens = lex.get();
    iToken = 0;
    tokActual = {0, ""};
    realizarAnalisisSemantico = activarSemantica;
    generarCodigo = activarBytecode;
}

Sintaxis::~Sintaxis() {}

// Avanza al siguiente token
void Sintaxis::sigToken() {
    if (iToken >= lstTokens.size()) {
        tokActual.tipoToken = LIN_EOF;
        return;
    }
    tokActual = lstTokens[iToken];
    iToken++;
}

// Recuperación de errores (Panic Mode)
void Sintaxis::sincronizar() {
    if (tokActual.tipoToken == LIN_EOF) return;
    sigToken();
    while (tokActual.tipoToken != LIN_EOLN && tokActual.tipoToken != LIN_EOF) {
        sigToken();
    }
    if (tokActual.tipoToken == LIN_EOLN) {
        sigToken();
    }
}

// Registra errores en la cola
void Sintaxis::registrarError(int codigo) {
    if (colaErrores.size() > 100) return;
    InfoError errorInfo = {codigo, mensajeError(codigo), tokActual.linea};
    colaErrores.push(errorInfo);
}

// Método principal público
int Sintaxis::generaSintaxis() {
    procPrincipal();
    return colaErrores.empty() ? ERR_NO_SINTAX_ERROR : colaErrores.front().codigo;
}

// =========================================================================
// MÉTODOS DE LA GRAMÁTICA
// =========================================================================

int Sintaxis::procPrincipal() {
    int error = ERR_NO_SINTAX_ERROR;
    int tipoIdent;
    string ident;

    sigToken(); // Leer primer token

    while (tokActual.tipoToken != RES_INICIO && tokActual.tipoToken != LIN_EOF) {
        error = ERR_NO_SINTAX_ERROR;

        // Declaración de variables (entero, flotante, cadena)
        if (tokActual.tipoToken == RES_ENTERO || tokActual.tipoToken == RES_FLOTANTE || tokActual.tipoToken == RES_CADENA) {
            tipoIdent = tokActual.tipoToken;
            sigToken();
            if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                ident = tokActual.token;
                sigToken();
                if (tokActual.tipoToken == LIN_EOLN) {
                    if (realizarAnalisisSemantico) {
                        int err = agregarIdentificador(ident, tipoIdent, 0);
                        if (err != 0) registrarError(err);
                    } else {
                        agregarIdentificador(ident, tipoIdent, 0);
                    }
                    sigToken();
                } else error = ERR_EOLN;
            } else error = ERR_IDENTIFICADOR;
        }
        // Declaración de funciones (def)
        else if (tokActual.tipoToken == RES_DEF) {
            tipoIdent = RES_FUNCION;
            sigToken();
            if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                ident = tokActual.token;
                sigToken();
                if (tokActual.tipoToken == LIN_EOLN) {
                    if (realizarAnalisisSemantico) {
                        int err = agregarIdentificador(ident, tipoIdent, 0);
                        if (err == ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE) registrarError(err);
                    } else {
                        agregarIdentificador(ident, tipoIdent, 0);
                    }

                    sigToken();
                    procInstrucciones(); // Cuerpo de la función

                    if (tokActual.tipoToken == RES_FINDEF) {
                        sigToken();
                        if (tokActual.tipoToken == LIN_EOLN) sigToken();
                        else error = ERR_EOLN;
                    } else error = ERR_FINDEF;
                } else error = ERR_EOLN;
            } else error = ERR_IDENTIFICADOR;
        }
        // Saltos de línea vacíos
        else if (tokActual.tipoToken == LIN_EOLN) {
            sigToken();
        }
        else {
            error = ERR_INICIO;
        }

        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        }
    }

    // Bloque Principal (inicio ... final)
    if (tokActual.tipoToken == RES_INICIO) {
        sigToken();
        if (tokActual.tipoToken == LIN_EOLN) {
            sigToken();
            procInstrucciones();
        } else {
            registrarError(ERR_EOLN);
            sincronizar();
            procInstrucciones();
        }

        if (tokActual.tipoToken != RES_FINAL) {
            registrarError(ERR_FINAL);
        }
    }
    return colaErrores.empty() ? 0 : colaErrores.front().codigo;
}

int Sintaxis::procInstrucciones()
{
    int error = ERR_NO_SINTAX_ERROR;

    while (tokActual.tipoToken != LIN_EOF &&
           tokActual.tipoToken != RES_FINAL &&
           tokActual.tipoToken != RES_FINDEF &&
           tokActual.tipoToken != RES_FINMI &&
           tokActual.tipoToken != RES_FINSI)
    {
        error = ERR_NO_SINTAX_ERROR;
        switch (tokActual.tipoToken)
        {
            case RES_SI : error = procDefSi(); break;
            case RES_MIENTRAS : error = procDefMientras(); break;
            case RES_CICLO : error = procDefCiclo(); break;
            case RES_SALIDA : error = procDefSalida(); break;
            case RES_ENTRADA : error = procDefEntrada(); break;
            case LIN_IDENTIFICADOR : error = procDefIdentificador(); break;
            case LIN_EOLN: sigToken(); continue;
            default: error = ERR_INSTRUCCION_DESCONOCIDA; break;
        }

        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        } else {
            if (tokActual.tipoToken == LIN_EOLN) {
                sigToken();
                // Verificamos si el bloque terminó después del EOLN
                if (tokActual.tipoToken == RES_FINMI || tokActual.tipoToken == RES_FINDEF ||
                    tokActual.tipoToken ==  RES_FINAL || tokActual.tipoToken ==  RES_FINSI)
                    break;
            } else {
                registrarError(ERR_EOLN);
                sincronizar();
            }
        }
    }
    return ERR_NO_SINTAX_ERROR;
}

int Sintaxis::procDefSalida()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    if (tokActual.token == "(") {
        while (true) {
            sigToken();
            int tipo = 0;
            error = procDefExpresion(tipo); // La expresión deja el valor en la pila
            if (error != ERR_NO_SINTAX_ERROR) break;

            // GENERACION: Imprimir tope de pila
            if (generarCodigo) generador.emitir("OUT");

            if (tokActual.token == ",") ; // Siguiente argumento
            else if (tokActual.token == ")") { sigToken(); break; }
            else { error = ERR_PARENTESIS_CERRAR; break; }
        }
    } else error = ERR_PARENTESIS_ABRIR;
    return error;
}

int Sintaxis::procDefEntrada()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    if (tokActual.token == "(") {
        sigToken();
        if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
            string nombre = tokActual.token;
            if (realizarAnalisisSemantico && !semantica.existeIdentificador(nombre))
                registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
            int tipo = semantica.getTipoIdentificador(nombre);

            if (generarCodigo) {
                if (tipo == RES_CADENA) {
                    generador.emitir("IN_STR", nombre); // Instruccion para cadenas
                } else {
                    generador.emitir("IN_NUM", nombre); // Instruccion para numeros (Entero/Flotante)
                }
            }
            sigToken();
            if (tokActual.token == ")") sigToken();
            else error = ERR_PARENTESIS_CERRAR;
        } else error = ERR_IDENTIFICADOR;
    } else error = ERR_PARENTESIS_ABRIR;
    return error;
}

int Sintaxis::procDefIdentificador()
{
    int error = ERR_NO_SINTAX_ERROR;
    string nombreVar = tokActual.token;
    int tipoVar = semantica.getTipoIdentificador(nombreVar);

    // Validación Semántica
    if (realizarAnalisisSemantico && tipoVar == RES_NO_DECL) {
        registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
        tipoVar = RES_ENTERO; // Asumimos entero para continuar
    }

    sigToken();

    // Asignación simple (=)
    if (tokActual.token == "=") {
        if (realizarAnalisisSemantico && tipoVar == RES_FUNCION)
            registrarError(ERR_SEMANTICA_IDENT_FUNCION_MAL_USO);

        sigToken();
        int tipoExpr = 0;

        // Procesamos expresión (Genera código PUSH/ADD/ETC)
        error = procDefExpresion(tipoExpr);

        if (error == ERR_NO_SINTAX_ERROR) {
             if (realizarAnalisisSemantico && !semantica.sonTiposCompatibles(tipoVar, tipoExpr))
                 registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);

             // GENERACION: Guardar tope de pila
             if (generarCodigo) generador.emitir("STORE", nombreVar);
        }
    }
    // Incremento/Decremento (++, --)
    else if (tokActual.token == "++" || tokActual.token == "--") {
        string op = tokActual.token;
        if (realizarAnalisisSemantico && tipoVar != RES_ENTERO && tipoVar != RES_FLOTANTE)
             registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO);

        // i++ -> LOAD i, PUSH 1, ADD, STORE i
        if (generarCodigo) {
            generador.emitir("LOAD", nombreVar);
            generador.emitir("PUSH", "1");
            generador.emitir(op == "++" ? "ADD" : "SUB");
            generador.emitir("STORE", nombreVar);
        }
        sigToken();
    }
    // Llamada a función (si fuera el caso, aqui no implementado completo)
    else if (tipoVar != RES_FUNCION) {
         if (realizarAnalisisSemantico) error = ERR_SEMANTICA_FUNCION_NO_DECL;
    }

    return error;
}

int Sintaxis::procDefExpresion(int &tipoResultado)
{
    int error = ERR_NO_SINTAX_ERROR;

    // 1. PRIMER OPERANDO (Generar PUSH o LOAD)
    if (tokActual.tipoToken == LIN_NUM_ENTERO || tokActual.tipoToken == LIN_NUM_FLOTANTE || tokActual.tipoToken == LIN_CADENA) {
        if (generarCodigo) generador.emitir("PUSH", tokActual.token);
        tipoResultado = (tokActual.tipoToken == LIN_CADENA) ? RES_CADENA :
                        (tokActual.tipoToken == LIN_NUM_FLOTANTE ? RES_FLOTANTE : RES_ENTERO);
    }
    else if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
        if (generarCodigo) generador.emitir("LOAD", tokActual.token);
        tipoResultado = semantica.getTipoIdentificador(tokActual.token);
        if (realizarAnalisisSemantico && tipoResultado == RES_NO_DECL) {
             registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
             tipoResultado = RES_ENTERO;
        }
    }

    // Consumir el operando
    if (tokActual.token != "(") sigToken();
    else { // Es paréntesis
        sigToken();
        error = procDefExpresion(tipoResultado);
        if (tokActual.token == ")") sigToken();
        else return ERR_PARENTESIS_CERRAR;
    }

    // 2. BUSCAR OPERADORES (+, -, *, /)
    while (true)
    {
        if (tokActual.token == "+" || tokActual.token == "-" || tokActual.token == "/" || tokActual.token == "*")
        {
            string op = tokActual.token;
            sigToken();
            int tipoOp2 = 0;

            // Recursividad para el segundo operando
            error = procDefExpresion(tipoOp2);

            // GENERACION
            if (generarCodigo && error == ERR_NO_SINTAX_ERROR) {
                if (op == "+") generador.emitir("ADD");
                else if (op == "-") generador.emitir("SUB");
                else if (op == "*") generador.emitir("MUL");
                else if (op == "/") generador.emitir("DIV");
            }
        }
        else
        {
            // CRITICO: Si encontramos un operador lógico (>, <, ==), paréntesis de cierre, coma o fin de linea
            // debemos ROMPER el bucle y regresar, porque esos tokens pertenecen a procDefCondicion o procDefSalida.
            if (tokActual.tipoToken == LIN_MAYOR || tokActual.tipoToken == LIN_MENOR ||
                tokActual.token == ">" || tokActual.token == "<" ||
                tokActual.token == "==" || tokActual.token == ">=" || tokActual.token == "<=" || tokActual.token == "<>" ||
                tokActual.token == ")" || tokActual.token == "," || tokActual.tipoToken == LIN_EOLN || tokActual.tipoToken == LIN_EOF)
            {
                break; // Salida correcta
            }
            else
            {
                // Token inesperado en una expresión aritmética
                error = ERR_PARENTESIS_CERRAR;
                break;
            }
        }
    }
    return error;
}

int Sintaxis::procDefCondicion() {
    int error = ERR_NO_SINTAX_ERROR;
    int t1=0, t2=0;

    sigToken(); // Avanzar al primer token de la condicion

    // 1. Expresión Izquierda
    error = procDefExpresion(t1);

    if (error == ERR_NO_SINTAX_ERROR) {
        string op = tokActual.token;
        int tipo = tokActual.tipoToken;

        // Verificación robusta del operador lógico
        bool esOperador = false;
        if (tipo == LIN_MAYOR || tipo == LIN_MENOR) esOperador = true;
        else if (op == "==" || op == ">=" || op == "<=" || op == "<>" || op == ">" || op == "<") esOperador = true;

        if (esOperador) {
            sigToken();
            // 2. Expresión Derecha
            error = procDefExpresion(t2);

            if (error == ERR_NO_SINTAX_ERROR) {
                // Semántica
                if (realizarAnalisisSemantico && !semantica.sonTiposCompatibles(t1, t2)) {
                    registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);
                }

                // Generación Código
                if (generarCodigo) {
                    if (op == "==") generador.emitir("EQ");
                    else if (tipo == LIN_MAYOR || op == ">") generador.emitir("GT");
                    else if (tipo == LIN_MENOR || op == "<") generador.emitir("LT");
                    else if (op == ">=") generador.emitir("GTE");
                    else if (op == "<=") generador.emitir("LTE");
                    else if (op == "<>") generador.emitir("NE");
                }
            }
        } else {
            error = ERR_OP_LOGICO;
        }
    }
    return error;
}

int Sintaxis::procDefSi() {
    int error = ERR_NO_SINTAX_ERROR;

    // 1. Condición
    error = procDefCondicion();

    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN) {
        sigToken();

        // Generar etiqueta y salto
        string lblFin = "";
        if (generarCodigo) {
            lblFin = generador.nuevaEtiqueta();
            generador.emitir("JMPF", lblFin); // Saltar a Fin si Falso
        }

        procInstrucciones();

        // Poner etiqueta destino
        if (generarCodigo) generador.emitir("LABEL", lblFin);

        if (tokActual.tipoToken == RES_FINSI) sigToken();
        else error = ERR_FINSI;
    } else if (error == ERR_NO_SINTAX_ERROR) error = ERR_EOLN;

    return error;
}

int Sintaxis::procDefMientras() { return ERR_NO_SINTAX_ERROR; } // Pendiente
int Sintaxis::procDefCiclo() { return ERR_NO_SINTAX_ERROR; } // Pendiente

// =======================
// UTILIDADES
// =======================

void Sintaxis::imprimirBytecode() { generador.imprimir(); }
int Sintaxis::agregarIdentificador(string id, int t, int l) { return semantica.agregarIdentificador(id, t); }
int Sintaxis::getTipoIdentificador(string id) { return semantica.getTipoIdentificador(id); }

string Sintaxis::mensajeError(int err) {
    if (err == ERR_NO_SINTAX_ERROR) return "No error";
    if (err == ERR_INICIO) return "Se esperaba [INICIO]";
    if (err == ERR_FINAL) return "Se esperaba [FINAL]";
    if (err == ERR_FINDEF) return "Se esperaba [FINDEF]";
    if (err == ERR_PARENTESIS_ABRIR) return "Se esperaba (";
    if (err == ERR_PARENTESIS_CERRAR) return "Se esperaba )";
    if (err == ERR_OP_LOGICO) return "Se esperaba operador logico";
    if (err == ERR_FINMI) return "Se esperaba [FINMI]";
    if (err == ERR_CONDICION) return "Error en la condicion";
    if (err == ERR_FINSI) return "Se esperaba [FINSI]";
    if (err == ERR_EOLN) return "Se esperaba Fin de Linea";
    if (err == ERR_IDENTIFICADOR) return "Se esperaba Identificador";

    if (err == ERR_SEMANTICA_IDENTIFICADOR_NO_DECL) return "Identificador no declarado";
    if (err == ERR_SEMANTICA_TIPOS_INCOMPATIBLES) return "Tipos incompatibles";
    if (err == ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE) return "Identificador ya declarado";
    if (err == ERR_SEMANTICA_IDENT_FUNCION_MAL_USO) return "Mal uso de funcion";
    if (err == ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO) return "Se esperaba entero";

    return "Error sintactico codigo " + to_string(err);
}

// IMPRIMIR ERRORES (Agregado para evitar error de enlace)
void Sintaxis::imprimirErrores()
{
    if (colaErrores.empty()) {
        cout << "---------------------------------------" << endl;
        cout << "ANALISIS FINALIZADO CON EXITO" << endl;
        cout << "No se encontraron errores." << endl;
        cout << "---------------------------------------" << endl;
        return;
    }

    cout << "---------------------------------------" << endl;
    cout << "ERRORES ENCONTRADOS (" << colaErrores.size() << "):" << endl;
    cout << "---------------------------------------" << endl;

    int contador = 1;
    while (!colaErrores.empty()) {
        InfoError info = colaErrores.front();
        cout << contador << ". [Linea " << info.linea << "] Codigo " << info.codigo << ": " << info.mensaje << endl;
        colaErrores.pop();
        contador++;
    }
    cout << "---------------------------------------" << endl;
}