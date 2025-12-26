#include <iostream>
#include <list>
#include <string>
#include <cctype>
#include "lexico.h"
#include "sintaxis.h"

using namespace std;

Sintaxis::Sintaxis(Lexico lex)
{
    lexico = lex;
    lstTokens = lex.get();
    iToken = 0;
    tokActual = {0, ""};
}

Sintaxis::~Sintaxis()
{
}

void Sintaxis::sincronizar()
{
    if (tokActual.tipoToken == LIN_EOF) return;
    sigToken(); // Avanzar al menos uno
    while (tokActual.tipoToken != LIN_EOLN && tokActual.tipoToken != LIN_EOF)
    {
        sigToken();
    }
    if (tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
    }
}

void Sintaxis::registrarError(int codigo)
{
    if (colaErrores.size() > 100) return;

    InfoError errorInfo;
    errorInfo.codigo = codigo;
    errorInfo.mensaje = mensajeError(codigo);
    errorInfo.linea = tokActual.linea; // Capturamos la linea
    colaErrores.push(errorInfo);
}

int Sintaxis::generaSintaxis()
{
    procPrincipal();
    if (!colaErrores.empty()) {
        return colaErrores.front().codigo;
    }
    return ERR_NO_SINTAX_ERROR;
}


void Sintaxis::sigToken()
{
    // Protección de limites
    if (iToken >= lstTokens.size()) {
        tokActual.tipoToken = LIN_EOF;
        return;
    }

    // Avanzamos al siguiente token
    tokActual = lstTokens[iToken];
    iToken++;
}

int Sintaxis::procPrincipal()
{
    int error = ERR_NO_SINTAX_ERROR;
    int tipoIdent;
    string ident;

    sigToken();

    while (tokActual.tipoToken != RES_INICIO && tokActual.tipoToken != LIN_EOF)
    {
        error = ERR_NO_SINTAX_ERROR; // Resetear siempre al inicio

        if (tokActual.tipoToken == RES_ENTERO || tokActual.tipoToken == RES_FLOTANTE || tokActual.tipoToken == RES_CADENA)
        {
            tipoIdent = tokActual.tipoToken;
            sigToken();
            if (tokActual.tipoToken == LIN_IDENTIFICADOR)
            {
                ident = tokActual.token;
                sigToken();
                if (tokActual.tipoToken == LIN_EOLN)
                {
                    // === CORRECCIÓN AQUÍ ===
                    int errSemantico = agregarIdentificador(ident, tipoIdent, 0);
                    if (errSemantico != ERR_SEMANTICA_NO_ERROR) {
                         registrarError(errSemantico);
                         // NO asignamos 'error = errSemantico' para evitar que se dispare sincronizar()
                         // La estructura sintáctica es correcta, solo falló la lógica.
                    }
                    sigToken();
                }
                else
                    error = ERR_EOLN;
            }
            else
                error = ERR_IDENTIFICADOR;
        }
        else if (tokActual.tipoToken == RES_DEF)
        {
            tipoIdent = RES_FUNCION;
            sigToken();
            if (tokActual.tipoToken == LIN_IDENTIFICADOR)
            {
                ident = tokActual.token;
                sigToken();
                if (tokActual.tipoToken == LIN_EOLN)
                {
                    int errSemantico = agregarIdentificador(ident, tipoIdent, 0);
                    if (errSemantico == ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE) {
                        registrarError(errSemantico);
                        // Aquí tampoco sincronizamos, pero tal vez querramos evitar procesar el cuerpo
                        // Por simplicidad, dejamos que siga
                    }

                    sigToken();
                    procInstrucciones();

                    if (tokActual.tipoToken == RES_FINDEF)
                    {
                        sigToken();
                        if (tokActual.tipoToken == LIN_EOLN)
                            sigToken();
                        else
                            error = ERR_EOLN;
                    }
                    else
                        error = ERR_FINDEF;
                }
                else
                    error = ERR_EOLN;
            }
            else
                error = ERR_IDENTIFICADOR;
        }
        else if (tokActual.tipoToken == LIN_EOLN)
        {
            sigToken();
        }
        else
        {
            error = ERR_INICIO;
        }

        // Solo sincronizamos si hubo un error SINTÁCTICO real
        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        }
    }

    if (tokActual.tipoToken == RES_INICIO)
    {
        sigToken();
        if (tokActual.tipoToken == LIN_EOLN)
        {
            sigToken();
            procInstrucciones();
        }
        else
        {
            registrarError(ERR_EOLN);
            sincronizar();
            procInstrucciones();
        }

        if (tokActual.tipoToken != RES_FINAL)
        {
            registrarError(ERR_FINAL);
        }
    }

    return colaErrores.empty() ? ERR_NO_SINTAX_ERROR : colaErrores.front().codigo;
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
            case RES_CICLO : error = procDefCiclo(); break;
            case RES_MIENTRAS : error = procDefMientras(); break;
            case RES_SALIDA : error = procDefSalida(); break;
            case RES_ENTRADA : error = procDefEntrada(); break;
            case LIN_IDENTIFICADOR : error = procDefIdentificador(); break;
            case LIN_EOLN: sigToken(); continue;
            default: error = ERR_INSTRUCCION_DESCONOCIDA; break;
        }

        if (error != ERR_NO_SINTAX_ERROR)
        {
            registrarError(error);
            sincronizar();
        }
        else
        {
            if (tokActual.tipoToken == LIN_EOLN)
            {
                sigToken();
                if (tokActual.tipoToken == RES_FINMI || tokActual.tipoToken == RES_FINDEF ||
                    tokActual.tipoToken ==  RES_FINAL || tokActual.tipoToken ==  RES_FINSI)
                    break;
            }
            else
            {
                registrarError(ERR_EOLN);
                sincronizar();
            }
        }
    }
    return ERR_NO_SINTAX_ERROR;
}

int Sintaxis::procDefCiclo()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    return error;
}

int Sintaxis::procDefSi()
{
    int error = ERR_NO_SINTAX_ERROR;

    error = procDefCondicion();
    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
        procInstrucciones();
        if (tokActual.tipoToken == RES_FINSI)
            sigToken();
        else
            error = ERR_FINSI;
    }
    else if (error == ERR_NO_SINTAX_ERROR)
        error = ERR_EOLN;

    return error;
}

int Sintaxis::procDefMientras()
{
    int error = ERR_NO_SINTAX_ERROR;
    error = procDefCondicion();
    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
        procInstrucciones();
        if (tokActual.tipoToken == RES_FINMI)
            sigToken();
        else
            error = ERR_FINMI;
    }
    else if (error == ERR_NO_SINTAX_ERROR)
        error = ERR_EOLN;

    return error;
}

int Sintaxis::procDefSalida()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    if (tokActual.token == "(")
    {
        while (true)
        {
            sigToken();

            // CORRECCION: Pasar variable para el tipo
            int tipoResultado = 0;
            error = procDefExpresion(tipoResultado);

            if (error != ERR_NO_SINTAX_ERROR)
                break;

            if (tokActual.token == ",")
                ;
            else if (tokActual.token == ")")
            {
                sigToken();
                break;
            }
            else
            {
                error = ERR_PARENTESIS_CERRAR;
                break;
            }
        }
    }
    else
        error = ERR_PARENTESIS_ABRIR;

    return error;
}

int Sintaxis::procDefEntrada()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    if (tokActual.token == "(")
    {
        sigToken();
        if (tokActual.tipoToken == LIN_IDENTIFICADOR)
        {
            string nombre = tokActual.token;
            // Validar que la variable exista antes de leer en ella
            if (!semantica.existeIdentificador(nombre)) {
                registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
            }

            sigToken();
            if (tokActual.token == ")")
                sigToken();
            else
                error = ERR_PARENTESIS_CERRAR;
        }
        else
            error = ERR_IDENTIFICADOR;
    }
    else
        error = ERR_PARENTESIS_ABRIR;
    return error;
}

int Sintaxis::procDefCondicion()
{
    int error = ERR_NO_SINTAX_ERROR;
    int tipoIzq = 0;
    int tipoDer = 0;

    sigToken();
    // CORRECCION: Pasar variable tipo
    error = procDefExpresion(tipoIzq);

    if (error == ERR_NO_SINTAX_ERROR)
    {
        if (tokActual.token == "==" || tokActual.token == ">=" || tokActual.token == "<=" ||
            tokActual.token == "<>" || tokActual.token == ">" || tokActual.token == "<")
        {
            sigToken();
            // CORRECCION: Pasar variable tipo
            error = procDefExpresion(tipoDer);

            if (error == ERR_NO_SINTAX_ERROR) {
                if(!semantica.sonTiposCompatibles(tipoIzq, tipoDer)) {
                    registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);
                }
            }
        }
        else
            error = ERR_OP_LOGICO;
    }
    return error;
}

int Sintaxis::procDefExpresion(int &tipoResultado)
{
    int error = ERR_NO_SINTAX_ERROR;

    // Determinacion inicial del tipo
    if (tokActual.tipoToken == LIN_NUM_ENTERO) tipoResultado = RES_ENTERO;
    else if (tokActual.tipoToken == LIN_NUM_FLOTANTE) tipoResultado = RES_FLOTANTE;
    else if (tokActual.tipoToken == LIN_CADENA) tipoResultado = RES_CADENA;
    else if (tokActual.tipoToken == LIN_IDENTIFICADOR) {
        tipoResultado = semantica.getTipoIdentificador(tokActual.token);
        if (tipoResultado == RES_NO_DECL) {
            registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
            tipoResultado = RES_ENTERO; // Asumimos entero para continuar
        }
    }

    while (true)
    {
        if (tokActual.tipoToken == LIN_CADENA || tokActual.tipoToken == LIN_NUMERO || tokActual.tipoToken == LIN_NUM_ENTERO ||
            tokActual.tipoToken == LIN_NUM_FLOTANTE || tokActual.tipoToken == LIN_IDENTIFICADOR)
        {
            sigToken();
            if (tokActual.token == "+" || tokActual.token == "-" || tokActual.token == "/" || tokActual.token == "*")
            {
                string op = tokActual.token;
                sigToken();
                // Validacion simplificada del segundo operando...
                // (Para hacerlo robusto se requiere recursividad compleja, aqui mantenemos estructura lineal)
            }
            else
            {
                if (!(tokActual.token == ")" || tokActual.token == "," || tokActual.token == "==" || tokActual.token == ">=" ||
                    tokActual.token == "<=" || tokActual.token == "<>" || tokActual.token == ">" || tokActual.token == "<") &&
                    tokActual.tipoToken != LIN_EOLN)
                    error = ERR_PARENTESIS_CERRAR;
                break;
            }
        }
        else if (tokActual.token == "(")
        {
            sigToken();
            error = procDefExpresion(tipoResultado);
            if (error != ERR_NO_SINTAX_ERROR) break;

            if (tokActual.token == ")")
            {
                sigToken();
                if (tokActual.token == "+" || tokActual.token == "-" || tokActual.token == "/" || tokActual.token == "*")
                    sigToken();
                else
                    break;
            }
        }
        else
        {
            error = ERR_IDENTIFICADOR;
            break;
        }
    }

    return error;
}

int Sintaxis::procDefIdentificador()
{
    int error = ERR_NO_SINTAX_ERROR;
    string nombreVar = tokActual.token;
    int tipoVar = semantica.getTipoIdentificador(nombreVar);

    if (tipoVar == RES_NO_DECL) {
        // No retornamos error fatal, solo registramos y seguimos
        registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_DECL);
        tipoVar = RES_ENTERO; // Dummy type
    }

    sigToken();
    if (tokActual.token == "=")
    {
        if (tipoVar == RES_FUNCION) registrarError(ERR_SEMANTICA_IDENT_FUNCION_MAL_USO);

        sigToken();
        int tipoExpresion = 0;
        // CORRECCION: Pasar variable
        error = procDefExpresion(tipoExpresion);

        if (error == ERR_NO_SINTAX_ERROR) {
             if (!semantica.sonTiposCompatibles(tipoVar, tipoExpresion)) {
                 registrarError(ERR_SEMANTICA_TIPOS_INCOMPATIBLES);
             }
        }
    }
    else if (tokActual.token == "++" || tokActual.token == "--")
    {
        if (tipoVar != RES_ENTERO && tipoVar != RES_FLOTANTE)
            registrarError(ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO);
        sigToken();
    }
    else if (tipoVar != RES_FUNCION)
        error = ERR_SEMANTICA_FUNCION_NO_DECL;

    return error;
}


string Sintaxis::mensajeError(int err)
{
    string s = "";
    switch (err)
    {
        case ERR_NO_SINTAX_ERROR : s = "No se encontraron errores"; break;
        case ERR_INICIO : s = "se esperaba [INICIO]"; break;
        case ERR_FINAL : s = "se esperaba [FINAL]"; break;
        case ERR_FINDEF : s = "se esperaba [FINDEF]"; break;
        case ERR_PARENTESIS_ABRIR : s = "se esperaba ("; break;
        case ERR_PARENTESIS_CERRAR : s = "se esperaba )"; break;
        case ERR_OP_LOGICO : s = "se esperaba operador logico"; break;
        case ERR_FINMI : s = "se esperaba [FINMI]"; break;
        case ERR_CONDICION : s = "error en la condicion"; break;
        case ERR_FINSI : s = "se esperaba [FINSI]"; break;
        case ERR_EOLN : s = "se esperaba Fin de Linea (EOLN)"; break;
        case ERR_IDENTIFICADOR : s = "se esperaba un Identificador"; break;
        case ERR_INSTRUCCION_DESCONOCIDA : s = "Instruccion desconocida o inesperada"; break;

        case ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE : s = "ERROR SEMANTICO: Identificador ya declarado"; break;
        case ERR_SEMANTICA_IDENTIFICADOR_NO_DECL : s = "ERROR SEMANTICO: Identificador NO declarado"; break;
        case ERR_SEMANTICA_TIPOS_INCOMPATIBLES : s = "ERROR SEMANTICO: Tipos incompatibles"; break;
        case ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO : s = "ERROR SEMANTICO: Se esperaba variable numerica"; break;
        case ERR_SEMANTICA_IDENT_FUNCION_MAL_USO : s = "ERROR SEMANTICO: Mal uso de funcion/variable"; break;

        default: s = "Error desconocido"; break;
    }
    return s;
}

// Estos Wrappers ahora usan Semantica
int Sintaxis::agregarIdentificador(string iden, int tipo, int linea)
{
    return semantica.agregarIdentificador(iden, tipo);
}

// Este se puede quedar vacío o eliminarse si no lo usas en main
vector<tToken> Sintaxis::getListaIdentificadores()
{
    vector<tToken> vacio;
    return vacio;
}

int Sintaxis::getTipoIdentificador(string iden)
{
    return semantica.getTipoIdentificador(iden);
}

string Sintaxis::getStrTipoIdentificador(int tipo)
{
    if (tipo == RES_ENTERO) return "ENTERO";
    if (tipo == RES_FLOTANTE) return "FLOTANTE";
    if (tipo == RES_CADENA) return "CADENA";
    return "OTRO";
}

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