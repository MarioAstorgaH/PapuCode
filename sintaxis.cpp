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

    // Preparamos la etiqueta, pero NO emitimos el JMP todavía
    string lblMain = "";
    if (generarCodigo) {
        lblMain = generador.nuevaEtiqueta();
    }

    sigToken(); // Leer primer token

    // =========================================================================
    // FASE 1: VARIABLES GLOBALES
    // (Se ejecutan secuencialmente al iniciar el programa)
    // =========================================================================
    while (tokActual.tipoToken == RES_ENTERO ||
           tokActual.tipoToken == RES_FLOTANTE ||
           tokActual.tipoToken == RES_CADENA ||
           tokActual.tipoToken == LIN_EOLN)
    {
        if (tokActual.tipoToken == LIN_EOLN) {
            sigToken();
            continue;
        }

        // Proceso de Declaración Global
        tipoIdent = tokActual.tipoToken;
        sigToken();
        if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
            ident = tokActual.token;
            sigToken();
            if (tokActual.tipoToken == LIN_EOLN) {
                // Registrar en Tabla de Símbolos
                if (realizarAnalisisSemantico) agregarIdentificador(ident, tipoIdent, 0);
                else agregarIdentificador(ident, tipoIdent, 0);

                // Generar código de inicialización (STORE)
                if (generarCodigo) {
                    // Empujamos valor default y guardamos en memoria GLOBAL
                    if (tipoIdent == RES_CADENA) generador.emitir("PUSH", "");
                    else generador.emitir("PUSH", "0");

                    // 🔥 Aquí usamos STORE_LOCAL porque estamos en scope 0 (Global)
                    // y queremos que la VM reserve el espacio ahora mismo.
                    generador.emitir("STORE_LOCAL", ident);
                }

                sigToken();
            } else { error = ERR_EOLN; break; }
        } else { error = ERR_IDENTIFICADOR; break; }
    }

    if (error != ERR_NO_SINTAX_ERROR) {
        registrarError(error);
        return error;
    }

    // =========================================================================
    // FASE 2: EL SALTO MÁGICO (JMP)
    // (Ahora que ya declaramos las variables, saltamos sobre las funciones)
    // =========================================================================
    if (generarCodigo) {
        generador.emitir("JMP", lblMain);
    }

    // =========================================================================
    // FASE 3: FUNCIONES
    // (Código muerto que solo se ejecuta al ser llamado)
    // =========================================================================
    while (tokActual.tipoToken == RES_DEF || tokActual.tipoToken == LIN_EOLN)
    {
        if (tokActual.tipoToken == LIN_EOLN) {
            sigToken();
            continue;
        }

        // ... [AQUÍ VA TU LÓGICA DE FUNCIONES EXACTAMENTE IGUAL] ...
        tipoIdent = RES_FUNCION;
        sigToken(); // Consumir 'def'

        if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
            ident = tokActual.token;

            // 1. REGISTRAR FUNCIÓN
            if (realizarAnalisisSemantico) {
                int err = agregarIdentificador(ident, tipoIdent, 0);
                if (err != 0) registrarError(err);
            } else {
                agregarIdentificador(ident, tipoIdent, 0);
            }

            sigToken(); // Consumir nombre

            // 2. LABEL
            if (generarCodigo) generador.emitir("LABEL", ident);

            // 3. PARÁMETROS
            vector<string> parametrosInvertidos;

            if (tokActual.token == "(") {
                sigToken();
                if (tokActual.token != ")") {
                    while (true) {
                        int tipoParam = 0;
                        if (tokActual.tipoToken == RES_ENTERO || tokActual.tipoToken == RES_FLOTANTE || tokActual.tipoToken == RES_CADENA) {
                            tipoParam = tokActual.tipoToken;
                            sigToken();
                            if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                                parametrosInvertidos.push_back(tokActual.token);
                                if (realizarAnalisisSemantico) semantica.agregarIdentificador(tokActual.token, tipoParam);
                                sigToken();
                            } else { error = ERR_IDENTIFICADOR; break; }
                        } else {
                           if (tokActual.token != ")" && tokActual.token != ",") { error = ERR_INSTRUCCION_DESCONOCIDA; break; }
                        }
                        if (tokActual.token == ",") sigToken();
                        else if (tokActual.token == ")") break;
                        else { error = ERR_PARENTESIS_CERRAR; break; }
                    }
                }
                if (error == ERR_NO_SINTAX_ERROR && tokActual.token == ")") sigToken();
            }

            // 4. GUARDAR PARÁMETROS
            if (generarCodigo && error == ERR_NO_SINTAX_ERROR) {
                for (int i = parametrosInvertidos.size() - 1; i >= 0; i--) {
                    generador.emitir("STORE_LOCAL", parametrosInvertidos[i]);
                }
            }

            // 5. CUERPO
            if (tokActual.tipoToken == LIN_EOLN) {
                sigToken();
                procInstrucciones();
                if (tokActual.tipoToken == RES_FINDEF) {
                    if (generarCodigo) generador.emitir("RET");
                    sigToken();
                    if (tokActual.tipoToken == LIN_EOLN) sigToken();
                    else error = ERR_EOLN;
                } else error = ERR_FINDEF;
            } else error = ERR_EOLN;

        } else error = ERR_IDENTIFICADOR;
        // ... [FIN LOGICA FUNCIONES] ...

        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar(); // Importante para recuperarse
            // break; // Opcional: Break si quieres detenerte al primer error de función
        }
    }

    // =========================================================================
    // FASE 4: BLOQUE PRINCIPAL (INICIO)
    // (Aquí aterriza el JMP de la Fase 2)
    // =========================================================================
    if (tokActual.tipoToken == RES_INICIO) {
        if (generarCodigo) generador.emitir("LABEL", lblMain);

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
    } else {
        // Si llegamos aquí y no es INICIO (y no es EOF), es un error de estructura
        if (tokActual.tipoToken != LIN_EOF) error = ERR_INICIO;
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
           tokActual.tipoToken != RES_SINO &&
           tokActual.tipoToken != RES_FINCI)
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
            case RES_ENTERO:
            case RES_FLOTANTE:
            case RES_CADENA: {
                int tipo = tokActual.tipoToken;
                sigToken(); // Consumir tipo

                if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
                    // Semántica
                    if (realizarAnalisisSemantico) semantica.agregarIdentificador(tokActual.token, tipo);
                    else agregarIdentificador(tokActual.token, tipo, 0);

                    // --- NUEVO: GENERAR CÓDIGO DE INICIALIZACIÓN LOCAL ---
                    if (generarCodigo) {
                        // Empujamos un valor por defecto (0 o "")
                        if (tipo == RES_CADENA) generador.emitir("PUSH", "");
                        else generador.emitir("PUSH", "0");

                        // Y lo forzamos a ser local
                        generador.emitir("STORE_LOCAL", tokActual.token);
                    }
                    // -----------------------------------------------------

                    sigToken(); // Consumir nombre variable
                } else {
                    error = ERR_IDENTIFICADOR;
                }
                break;
            }
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
                    tokActual.tipoToken == RES_SINO || tokActual.tipoToken == RES_FINCI)
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
int Sintaxis::procDefCiclo() {
    int error = ERR_NO_SINTAX_ERROR;
    sigToken(); // Consumir 'ciclo'

    if (tokActual.token == "(") {
        sigToken(); // Consumir '('

        // 1. INICIALIZACIÓN
        if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
            string id = tokActual.token;
            int tipoId = semantica.getTipoIdentificador(id);
            if (realizarAnalisisSemantico && tipoId == RES_NO_DECL) return ERR_SEMANTICA_IDENTIFICADOR_NO_DECL;

            sigToken();
            if (tokActual.token == "=") {
                sigToken();
                int tipoExpr = 0;
                error = procDefExpresion(tipoExpr);
                if (error != ERR_NO_SINTAX_ERROR) return error;
                if (generarCodigo) generador.emitir("STORE_LOCAL", id);
            } else return ERR_INSTRUCCION_DESCONOCIDA;
        } else return ERR_IDENTIFICADOR;

        // 🔥🔥🔥 CORRECCIÓN CRÍTICA AQUÍ 🔥🔥🔥
        // Verificamos que sea punto y coma, PERO NO LO CONSUMIMOS (sigToken).
        // ¿Por qué? Porque 'procDefCondicion' hace un sigToken() al inicio
        // y necesitamos que consuma este ';' en lugar de comerse la variable 'i'.
        if (tokActual.token == ";") {
             // NO HACER sigToken(); <--- EL SECRETO ESTÁ EN BORRAR ESTO
        }
        else return ERR_INSTRUCCION_DESCONOCIDA;
        // -------------------------------------------------------------------

        // 2. ETIQUETAS
        string lblCond = "", lblInc = "", lblCuerpo = "", lblFin = "";
        if (generarCodigo) {
            lblCond = generador.nuevaEtiqueta();
            lblInc = generador.nuevaEtiqueta();
            lblCuerpo = generador.nuevaEtiqueta();
            lblFin = generador.nuevaEtiqueta();
            generador.emitir("LABEL", lblCond);
        }

        // 3. CONDICIÓN
        error = procDefCondicion(); // Este consumirá el primer ';'
        if (error != ERR_NO_SINTAX_ERROR) return error;

        if (generarCodigo) {
            generador.emitir("JMPF", lblFin);
            generador.emitir("JMP", lblCuerpo);
            generador.emitir("LABEL", lblInc);
        }

        // El segundo punto y coma SÍ lo consumimos nosotros
        if (tokActual.token == ";") sigToken();
        else return ERR_INSTRUCCION_DESCONOCIDA;

        // 4. INCREMENTO
        if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
            string id = tokActual.token;
            sigToken();

            if (tokActual.token == "++") {
                if (generarCodigo) {
                    generador.emitir("LOAD", id);
                    generador.emitir("PUSH", "1");
                    generador.emitir("ADD");
                    generador.emitir("STORE_LOCAL", id);
                }
                sigToken();
            }
            else if (tokActual.token == "--") {
                if (generarCodigo) {
                    generador.emitir("LOAD", id);
                    generador.emitir("PUSH", "1");
                    generador.emitir("SUB");
                    generador.emitir("STORE_LOCAL", id);
                }
                sigToken();
            }
        } else return ERR_IDENTIFICADOR;

        if (tokActual.token == ")") sigToken();
        else return ERR_PARENTESIS_CERRAR;

        if (generarCodigo) {
            generador.emitir("JMP", lblCond);
            generador.emitir("LABEL", lblCuerpo);
        }

        if (tokActual.tipoToken == LIN_EOLN) sigToken();
        else return ERR_EOLN;

        // 5. CUERPO
        procInstrucciones();

        if (generarCodigo) {
            generador.emitir("JMP", lblInc);
            generador.emitir("LABEL", lblFin);
        }

        if (tokActual.tipoToken == RES_FINCI) sigToken();
        else error = ERR_INSTRUCCION_DESCONOCIDA;

    } else error = ERR_PARENTESIS_ABRIR;

    return error;
}

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
// =========================================================================
// JERARQUÍA DE EXPRESIONES (Expresion -> Termino -> Factor)
// =========================================================================

// 1. MANEJA SUMAS Y RESTAS (+, -)
int Sintaxis::procDefExpresion(int &tipoResultado)
{
    int error = ERR_NO_SINTAX_ERROR;

    // Llamamos a Termino (que maneja multiplicaciones)
    error = procDefTermino(tipoResultado);
    if (error != ERR_NO_SINTAX_ERROR) return error;

    while (tokActual.token == "+" || tokActual.token == "-") {
        string op = tokActual.token;
        int tipoIzq = tipoResultado;

        sigToken(); // Consumir operador

        int tipoDer = 0;
        error = procDefTermino(tipoDer); // Lado derecho
        if (error != ERR_NO_SINTAX_ERROR) return error;

        // Semántica
        if (realizarAnalisisSemantico) {
            if (!semantica.sonTiposCompatibles(tipoIzq, tipoDer))
                registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);
        }

        // Generación
        if (generarCodigo) {
            generador.emitir(op == "+" ? "ADD" : "SUB");
        }

        tipoResultado = (tipoIzq == RES_FLOTANTE || tipoDer == RES_FLOTANTE) ? RES_FLOTANTE : RES_ENTERO;
    }

    return error;
}

// 2. MANEJA MULTIPLICACIÓN Y DIVISIÓN (*, /)
int Sintaxis::procDefTermino(int &tipoResultado)
{
    int error = ERR_NO_SINTAX_ERROR;

    // Llamamos a Factor (Números, paréntesis, llamadas)
    error = procDefFactor(tipoResultado);
    if (error != ERR_NO_SINTAX_ERROR) return error;

    while (tokActual.token == "*" || tokActual.token == "/") {
        string op = tokActual.token;
        int tipoIzq = tipoResultado;

        sigToken(); // Consumir operador

        int tipoDer = 0;
        error = procDefFactor(tipoDer); // Lado derecho
        if (error != ERR_NO_SINTAX_ERROR) return error;

        if (realizarAnalisisSemantico) {
            if (!semantica.sonTiposCompatibles(tipoIzq, tipoDer))
                registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);
        }

        if (generarCodigo) {
            generador.emitir(op == "*" ? "MUL" : "DIV");
        }

        tipoResultado = (tipoIzq == RES_FLOTANTE || tipoDer == RES_FLOTANTE) ? RES_FLOTANTE : RES_ENTERO;
    }

    return error;
}

// 3. MANEJA EL DATO REAL (Números, Variables, FUNCIONES)
int Sintaxis::procDefFactor(int &tipoResultado)
{
    int error = ERR_NO_SINTAX_ERROR;

    // A. NÚMEROS Y CADENAS
    if (tokActual.tipoToken == LIN_NUM_ENTERO || tokActual.tipoToken == LIN_NUM_FLOTANTE || tokActual.tipoToken == LIN_CADENA) {
        if (generarCodigo) {
            if (tokActual.tipoToken == LIN_CADENA) generador.emitir("PUSH", tokActual.token);
            else generador.emitir("PUSH", tokActual.token);
        }

        if (tokActual.tipoToken == LIN_CADENA) tipoResultado = RES_CADENA;
        else if (tokActual.tipoToken == LIN_NUM_FLOTANTE) tipoResultado = RES_FLOTANTE;
        else tipoResultado = RES_ENTERO;

        sigToken();
    }
    // B. IDENTIFICADORES (Variables O Funciones)
    else if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
        string nombre = tokActual.token;
        int tipoIdent = semantica.getTipoIdentificador(nombre);

        // CASO B.1: Es una FUNCIÓN (ej: factorial(n-1))
        if (tipoIdent == RES_FUNCION) {
            sigToken(); // Consumir nombre

            if (tokActual.token == "(") {
                sigToken(); // Consumir '('

                // Argumentos
                if (tokActual.token != ")") {
                    while (true) {
                        int tipoArg = 0;
                        error = procDefExpresion(tipoArg); // Recursividad aquí
                        if (error != ERR_NO_SINTAX_ERROR) return error;

                        if (tokActual.token == ",") sigToken();
                        else if (tokActual.token == ")") break;
                        else return ERR_PARENTESIS_CERRAR;
                    }
                }

                if (tokActual.token == ")") {
                    sigToken(); // Consumir ')'
                    if (generarCodigo) generador.emitir("CALL", nombre);
                    tipoResultado = RES_ENTERO; // Asumimos retorno entero
                } else return ERR_PARENTESIS_CERRAR;
            } else return ERR_PARENTESIS_ABRIR; // <--- CORREGIDO (Antes decía ERR_CONSTANTE)
        }
        // CASO B.2: Es una VARIABLE (ej: n)
        else {
            if (realizarAnalisisSemantico && tipoIdent == RES_NO_DECL) {
                registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
            }
            if (generarCodigo) {
                generador.emitir("LOAD", nombre);
            }
            tipoResultado = tipoIdent;
            sigToken();
        }
    }
    // C. EXPRESIONES ENTRE PARÉNTESIS ( n - 1 )
    else if (tokActual.token == "(") {
        sigToken();
        error = procDefExpresion(tipoResultado); // Recursividad
        if (error != ERR_NO_SINTAX_ERROR) return error;

        if (tokActual.token == ")") sigToken();
        else return ERR_PARENTESIS_CERRAR;
    }
    else {
        return ERR_INSTRUCCION_DESCONOCIDA; // <--- CORREGIDO (Usamos este como genérico)
    }

    return error;
}