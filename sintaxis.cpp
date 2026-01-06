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

    // =============================================================
    // 1. SALTO AL MAIN (Para saltar las funciones)
    // =============================================================
    string lblMain = "";
    if (generarCodigo) {
        lblMain = generador.nuevaEtiqueta();
        generador.emitir("JMP", lblMain);
    }

    sigToken(); // Leer primer token

    while (tokActual.tipoToken != RES_INICIO && tokActual.tipoToken != LIN_EOF) {
        error = ERR_NO_SINTAX_ERROR;

        // Declaración de variables globales
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
        // =========================================================
        // DEFINICIÓN DE FUNCIONES (CORREGIDO)
        // =========================================================
        else if (tokActual.tipoToken == RES_DEF) {
            tipoIdent = RES_FUNCION;
            sigToken(); // <--- FALTABA ESTO: Consumir 'def'

            if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                ident = tokActual.token;
                sigToken(); // Consumir nombre función

                // 2. LABEL (Marcamos inicio función)
                if (generarCodigo) generador.emitir("LABEL", ident);

                // --- PARÁMETROS ---
                vector<string> parametrosInvertidos;

                if (tokActual.token == "(") {
                    sigToken(); // Consumir '('

                    // Si NO es ')' inmediatamente, hay parámetros
                    if (tokActual.token != ")") {
                        while (true) {
                            // 1. Tipo de dato
                            if (tokActual.tipoToken == RES_ENTERO || tokActual.tipoToken == RES_FLOTANTE || tokActual.tipoToken == RES_CADENA) {
                                sigToken();

                                // 2. Identificador
                                if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                                    parametrosInvertidos.push_back(tokActual.token);
                                    sigToken();
                                } else {
                                    error = ERR_IDENTIFICADOR;
                                    break;
                                }
                            } else {
                                if (tokActual.token != ")" && tokActual.token != ",") {
                                     error = ERR_INSTRUCCION_DESCONOCIDA;
                                     break;
                                }
                            }

                            // 3. Coma o Cierre
                            if (tokActual.token == ",") {
                                sigToken();
                            }
                            else if (tokActual.token == ")") {
                                break;
                            }
                            else {
                                error = ERR_PARENTESIS_CERRAR;
                                break;
                            }
                        }
                    }

                    // Consumir ')'
                    if (error == ERR_NO_SINTAX_ERROR && tokActual.token == ")") {
                        sigToken();
                    }
                }

                // --- GENERAR STORE PARA PARÁMETROS ---
                if (generarCodigo && error == ERR_NO_SINTAX_ERROR) {
                    for (int i = parametrosInvertidos.size() - 1; i >= 0; i--) {
                        generador.emitir("STORE", parametrosInvertidos[i]);
                    }
                }

                // --- ¡AQUI FALTABA TODO ESTO! ---
                // Verificar EOLN después del def foo(a)
                if (tokActual.tipoToken == LIN_EOLN) {
                    sigToken();

                    // Procesar el CUERPO de la función
                    procInstrucciones();

                    // Verificar FINDEF
                    if (tokActual.tipoToken == RES_FINDEF) {
                        if (generarCodigo) generador.emitir("RET"); // Retorno implícito al final
                        sigToken();

                        if (tokActual.tipoToken == LIN_EOLN) sigToken();
                        else error = ERR_EOLN;
                    } else {
                        error = ERR_FINDEF;
                    }
                } else {
                    error = ERR_EOLN;
                }

            } else {
                error = ERR_IDENTIFICADOR;
            }
        }
        // Saltos de línea vacíos
        else if (tokActual.tipoToken == LIN_EOLN) {
            sigToken();
        }
        else {
            error = ERR_INICIO; // Esperaba def, entero o inicio
        }

        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        }
    }

    // =============================================================
    // 2. BLOQUE PRINCIPAL (INICIO ... FINAL)
    // =============================================================
    if (tokActual.tipoToken == RES_INICIO) {

        // Aterrizaje del JMP
        if (generarCodigo) {
            generador.emitir("LABEL", lblMain);
        }

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
           tokActual.tipoToken != RES_FINSI &&
           tokActual.tipoToken != RES_SINO) // <--- AGREGAR ESTO
    {
        error = ERR_NO_SINTAX_ERROR;
        switch (tokActual.tipoToken)
        {
            case RES_SI : error = procDefSi(); break;
            case RES_MIENTRAS : error = procDefMientras(); break;
            case RES_CICLO : error = procDefCiclo(); break;
            case RES_SALIDA : error = procDefSalida(); break;
            case RES_RETORNAR: error = procDefRetornar(); break;
            case RES_ENTRADA : error = procDefEntrada(); break;
            case LIN_IDENTIFICADOR : error = procDefIdentificador(); break;
            case LIN_EOLN: sigToken(); continue;
            default: error = ERR_INSTRUCCION_DESCONOCIDA; break;
        }

        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        } else {
            if (tokActual.token == ";") sigToken();
            if (tokActual.tipoToken == LIN_EOLN) {
                sigToken();
                // Verificamos si el bloque terminó después del EOLN
                if (tokActual.tipoToken == RES_FINMI || tokActual.tipoToken == RES_FINDEF ||
                    tokActual.tipoToken ==  RES_FINAL || tokActual.tipoToken ==  RES_FINSI ||
                    tokActual.tipoToken == RES_SINO) // <--- Y AGREGAR ESTO
                        break;
            } else {
                // AQUI ESTA LA MAGIA: IMPRIMIR EL CULPABLE
                cerr << "[DEBUG ERROR 12] En linea " << tokActual.linea
                     << " esperaba ENTER, pero encontre token: [" << tokActual.token
                     << "] con ID: " << tokActual.tipoToken << endl;
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

            // --- DEBUG START ---
            if (realizarAnalisisSemantico) {
                if (!semantica.existeIdentificador(nombre)) {
                    registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
                }
            }
            // --- DEBUG END ---

            int tipo = semantica.getTipoIdentificador(nombre);

            if (generarCodigo) {
                if (tipo == RES_CADENA) generador.emitir("IN_STR", nombre);
                else generador.emitir("IN_NUM", nombre);
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

    // Validación Semántica inicial
    if (realizarAnalisisSemantico && tipoVar == RES_NO_DECL) {
        registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
        tipoVar = RES_ENTERO;
    }

    sigToken(); // Consumir el ID (ej: 'contador')

    // =========================================================
    // CASO 1: ASIGNACIÓN (Variable = Valor)
    // =========================================================
    if (tokActual.token == "=") {
        if (realizarAnalisisSemantico && tipoVar == RES_FUNCION)
            registrarError(ERR_SEMANTICA_IDENT_FUNCION_MAL_USO);

        sigToken(); // Consumir '='
        int tipoExpr = 0;

        // Procesamos expresión (Lee lo que está a la derecha del igual)
        error = procDefExpresion(tipoExpr);

        if (error == ERR_NO_SINTAX_ERROR) {
             if (realizarAnalisisSemantico && !semantica.sonTiposCompatibles(tipoVar, tipoExpr))
                 registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);

             // GENERACION: Guardar resultado en la variable
             if (generarCodigo) {
                 if (tipoVar == RES_CADENA) generador.emitir("STORE_STR", nombreVar);
                 else generador.emitir("STORE", nombreVar);
             }
        }
    }
    // =========================================================
    // CASO 2: INCREMENTO/DECREMENTO (i++, i--)
    // =========================================================
    else if (tokActual.token == "++" || tokActual.token == "--") {
        string op = tokActual.token;
        if (realizarAnalisisSemantico && tipoVar != RES_ENTERO && tipoVar != RES_FLOTANTE)
             registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO);

        // Generar código: Cargar variable, sumar 1, guardar variable
        if (generarCodigo) {
            generador.emitir("LOAD", nombreVar);
            generador.emitir("PUSH_NUM", "1");
            generador.emitir(op == "++" ? "ADD" : "SUB");
            generador.emitir("STORE", nombreVar);
        }
        sigToken();
    }
    // =========================================================
    // CASO 3: LLAMADA A FUNCIÓN (miFuncion())
    // =========================================================
    else if (tokActual.token == "(") {

        // Validar que sea función
        if (realizarAnalisisSemantico && tipoVar != RES_FUNCION)
             registrarError(ERR_SEMANTICA_IDENT_FUNCION_MAL_USO);

        sigToken(); // Consumir '('

        // Por ahora manejamos llamadas sin argumentos: funcion()
        if (tokActual.token == ")") {
            sigToken(); // Consumir ')'

            // GENERAR CÓDIGO: CALL nombreFuncion
            if (generarCodigo) generador.emitir("CALL", nombreVar);
        }
        else {
            return ERR_PARENTESIS_CERRAR; // Se esperaba ')'
        }
    }
    // =========================================================
    // CASO 4: ERROR (No es nada conocido)
    // =========================================================
    else {
         // Si llegamos aqui, es que escribiste "variable" sola sin hacerle nada
         // O faltó algo.
         return ERR_INSTRUCCION_DESCONOCIDA;
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

        string lblElse = "";
        string lblFin = "";

        if (generarCodigo) {
            lblElse = generador.nuevaEtiqueta(); // Etiqueta para el SINO
            generador.emitir("JMPF", lblElse);   // Si falso, ir a SINO
        }

        // Bloque VERDADERO
        procInstrucciones();

        // Si hay un SINO, necesitamos saltar al final después del bloque verdadero
        if (tokActual.tipoToken == RES_SINO) {
            if (generarCodigo) {
                lblFin = generador.nuevaEtiqueta();
                generador.emitir("JMP", lblFin);    // Saltar al final absoluto (evitar el else)
                generador.emitir("LABEL", lblElse); // Aquí empieza el SINO
            }

            sigToken(); // Consumir 'sino'
            if (tokActual.tipoToken == LIN_EOLN) sigToken();
            else error = ERR_EOLN;

            // Bloque FALSO (SINO)
            if (error == ERR_NO_SINTAX_ERROR) {
                procInstrucciones();
                if (generarCodigo) generador.emitir("LABEL", lblFin); // Fin absoluto
            }
        }
        else {
            // Si NO hubo sino, la etiqueta de falso es el fin
            if (generarCodigo) generador.emitir("LABEL", lblElse);
        }

        if (tokActual.tipoToken == RES_FINSI) sigToken();
        else error = ERR_FINSI;
    } else if (error == ERR_NO_SINTAX_ERROR) error = ERR_EOLN;

    return error;
}
int Sintaxis::procDefMientras() {
    int error = ERR_NO_SINTAX_ERROR;

    string lblInicio = "";
    string lblFin = "";

    if (generarCodigo) {
        lblInicio = generador.nuevaEtiqueta();
        lblFin = generador.nuevaEtiqueta();
        generador.emitir("LABEL", lblInicio); // Marcamos el inicio para volver
    }

    // Condición
    error = procDefCondicion();

    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN) {
        sigToken();

        if (generarCodigo) {
            generador.emitir("JMPF", lblFin); // Si la condición es falsa, salir
        }

        // Cuerpo del ciclo
        procInstrucciones();

        if (generarCodigo) {
            generador.emitir("JMP", lblInicio); // Volver a evaluar condición
            generador.emitir("LABEL", lblFin);  // Salida del ciclo
        }

        if (tokActual.tipoToken == RES_FINMI) sigToken();
        else error = ERR_FINMI;
    }
    else if (error == ERR_NO_SINTAX_ERROR) error = ERR_EOLN;

    return error;
}
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
int Sintaxis::procDefRetornar() {
    sigToken(); // Consumir 'retornar'

    // Calcular la expresión a retornar (La deja en el tope de la pila)
    int tipoDummy = 0;
    int error = procDefExpresion(tipoDummy);

    // Generar RET
    // Nota: El valor ya quedó en la pila gracias a procDefExpresion
    if (generarCodigo && error == 0) {
        generador.emitir("RET");
    }

    if (tokActual.token == ";") sigToken(); // Consumir ; si hay
    return error;
}