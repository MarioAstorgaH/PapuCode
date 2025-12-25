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
    // Destructor
}

// Función auxiliar para recuperar el parser tras un error
// Salta todos los tokens hasta encontrar un EOLN o EOF
void Sintaxis::sincronizar()
{
    // Importante: Si ya estamos en EOF, no hacemos nada para evitar loops
    if (tokActual.tipoToken == LIN_EOF) return;

    // Avanzamos al menos una vez para no quedarnos pegados en el error actual
    sigToken();

    // Consumir tokens hasta hallar EOLN o EOF
    while (tokActual.tipoToken != LIN_EOLN && tokActual.tipoToken != LIN_EOF)
    {
        sigToken();
    }

    // Si paramos por un EOLN, lo consumimos para empezar limpio en la sig linea
    if (tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
    }
}

void Sintaxis::registrarError(int codigo)
{
    // Límite de seguridad para evitar std::bad_alloc por bucles infinitos
    if (colaErrores.size() > 100) {
        return;
    }

    InfoError errorInfo;
    errorInfo.codigo = codigo;
    errorInfo.mensaje = mensajeError(codigo);
    colaErrores.push(errorInfo);
}

int Sintaxis::generaSintaxis()
{
    procPrincipal();

    if (!colaErrores.empty()) {
        // CORRECCIÓN: Acceder al atributo .codigo
        return colaErrores.front().codigo;
    }
    return ERR_NO_SINTAX_ERROR;
}
// Reemplaza tu método sigToken por este más seguro
void Sintaxis::sigToken()
{
    // 1. Protección de límites: Si ya nos pasamos del vector, devolver EOF
    if (iToken >= lstTokens.size()) {
        tokActual.tipoToken = LIN_EOF;
        return;
    }

    tokActual = lstTokens[iToken];

    if (tokActual.tipoToken == LIN_EOLN)
    {
        int iAux = iToken + 1;
        // 2. Protección en el bucle: Verificar que iAux no exceda el tamaño
        while (iAux < lstTokens.size() && lstTokens[iAux].tipoToken == LIN_EOLN)
            iAux += 1;
        iToken = iAux;
    }
    else if (tokActual.tipoToken != LIN_EOF)
    {
        iToken++;
    }
}

int Sintaxis::procPrincipal()
{
    int error = ERR_NO_SINTAX_ERROR;
    int tipoIdent;
    string ident;

    sigToken();

    // MODIFICADO: El while continúa hasta que se acabe el archivo o empiece el bloque principal
    // Ya no se detiene si 'error' es diferente de 0.
    while (tokActual.tipoToken != RES_INICIO && tokActual.tipoToken != LIN_EOF)
    {
        error = ERR_NO_SINTAX_ERROR; // Resetear error local

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
                    // agregar identificador valido
                    error = agregarIdentificador(ident, tipoIdent, 0);
                    if (error != ERR_SEMANTICA_NO_ERROR) {
                         registrarError(error);
                         // No sincronizamos aquí porque ya estamos en el EOLN
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
                    error = agregarIdentificador(ident, tipoIdent, 0);
                    if (error == ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE) {
                        registrarError(error);
                        // Si la funcion ya existe, intentamos parsearla de todas formas para hallar errores dentro
                    }

                    sigToken();
                    procInstrucciones(); // Llamamos instrucciones sin asignar error global

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
            error = ERR_INICIO; // Esperaba declaraciones o INICIO
        }

        // Si hubo error de estructura en este ciclo, registramos y sincronizamos
        if (error != ERR_NO_SINTAX_ERROR) {
            registrarError(error);
            sincronizar();
        }
    }

    // Procesamiento del bloque principal (INICIO ... FINAL)
    if (tokActual.tipoToken == RES_INICIO)
    {
        sigToken();
        if (tokActual.tipoToken == LIN_EOLN)
        {
            sigToken();
            procInstrucciones(); // Analiza el cuerpo
        }
        else
        {
            registrarError(ERR_EOLN);
            sincronizar();
            procInstrucciones(); // Intentamos seguir
        }

        if (tokActual.tipoToken != RES_FINAL)
        {
            registrarError(ERR_FINAL);
            // No sincronizamos porque ya estamos al final o cerca
        }
    }
    // Si salió del while por EOF y nunca encontró INICIO
    else if (tokActual.tipoToken == LIN_EOF)
    {
        // Solo es error si no encontramos ni INICIO ni nada válido antes
        // Depende de tu lógica, a veces un archivo vacio es válido.
        // registrarError(ERR_INICIO);
    }

    if (!colaErrores.empty()) {
        return colaErrores.front().codigo; // <--- CORRECCIÓN AQUÍ
    }
    return ERR_NO_SINTAX_ERROR;
}

int Sintaxis::procInstrucciones()
{
    int error = ERR_NO_SINTAX_ERROR;

    // MODIFICADO: El while continúa hasta encontrar delimitadores de fin de bloque
    while (tokActual.tipoToken != LIN_EOF &&
           tokActual.tipoToken != RES_FINAL &&
           tokActual.tipoToken != RES_FINDEF &&
           tokActual.tipoToken != RES_FINMI &&
           tokActual.tipoToken != RES_FINSI)
    {
        error = ERR_NO_SINTAX_ERROR; // Resetear error para esta instrucción

        switch (tokActual.tipoToken)
        {
            case RES_SI :
                error = procDefSi();
                break;
            case RES_CICLO :
                error = procDefCiclo();
                break;
            case RES_MIENTRAS :
                error = procDefMientras();
                break;
            case RES_SALIDA :
                error = procDefSalida();
                break;
            case RES_ENTRADA :
                error = procDefEntrada();
                break;
            case LIN_IDENTIFICADOR :
                error = procDefIdentificador();
                break;
            case LIN_EOLN:
                // Linea vacia, la consumimos y continuamos
                sigToken();
                continue;
            default:
                // Token desconocido dentro de un bloque de instrucciones
                error = ERR_INICIO; // O un error genérico "Instrucción no válida"
                break;
        }

        // Si la instrucción individual devolvió error
        if (error != ERR_NO_SINTAX_ERROR)
        {
            registrarError(error);
            sincronizar(); // Saltamos al siguiente EOLN para intentar la siguiente instrucción
        }
        else
        {
            // Si la instrucción fue exitosa, debe haber un EOLN
            if (tokActual.tipoToken == LIN_EOLN)
            {
                sigToken();
                // Verificamos si terminamos bloque
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

    return ERR_NO_SINTAX_ERROR; // Siempre retorna éxito al padre, los errores ya están en la cola
}

int Sintaxis::procDefCiclo()
{
    int error = ERR_NO_SINTAX_ERROR;
    sigToken();
    // Lógica del ciclo (aquí estaba vacía en tu código original)
    return error;
}

int Sintaxis::procDefSi()
{
    int error = ERR_NO_SINTAX_ERROR;

    error = procDefCondicion();
    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
        procInstrucciones(); // Recursividad resiliente

        if (tokActual.tipoToken == RES_FINSI)
            sigToken();
        else
            error = ERR_FINSI;
    }
    else if (error == ERR_NO_SINTAX_ERROR)
    {
        error = ERR_EOLN;
    }

    return error;
}

int Sintaxis::procDefMientras()
{
    int error = ERR_NO_SINTAX_ERROR;

    error = procDefCondicion();
    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
        procInstrucciones(); // Recursividad resiliente

        if (tokActual.tipoToken == RES_FINMI)
            sigToken();
        else
            error = ERR_FINMI;
    }
    else if (error == ERR_NO_SINTAX_ERROR)
    {
        error = ERR_EOLN;
    }

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
            error = procDefExpresion();

            if (error != ERR_NO_SINTAX_ERROR)
                break;

            if (tokActual.token == ",")
                ; // Continuar
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
    sigToken();
    error = procDefExpresion();
    if (error == ERR_NO_SINTAX_ERROR)
    {
        if (tokActual.token == "==" || tokActual.token == ">=" || tokActual.token == "<=" ||
            tokActual.token == "<>" || tokActual.token == ">" || tokActual.token == "<")
        {
            sigToken();
            error = procDefExpresion();
        }
        else
            error = ERR_OP_LOGICO;
    }
    return error;
}

int Sintaxis::procDefExpresion()
{
    int error = ERR_NO_SINTAX_ERROR;

    while (true)
    {
        if (tokActual.tipoToken == LIN_CADENA || tokActual.tipoToken == LIN_NUMERO || tokActual.tipoToken == LIN_NUM_ENTERO ||
            tokActual.tipoToken == LIN_NUM_FLOTANTE || tokActual.tipoToken == LIN_IDENTIFICADOR)
        {
            sigToken();
            if (tokActual.token == "+" || tokActual.token == "-" || tokActual.token == "/" || tokActual.token == "*")
                sigToken();
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
            error = procDefExpresion();

            if (error != ERR_NO_SINTAX_ERROR)
                break;

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
    int tipoId = getTipoIdentificador(tokActual.token);
    if (tipoId == RES_NO_DECL)
        return ERR_SEMANTICA_IDENTIFICADOR_NO_DECL ;

    sigToken();
    if (tokActual.token == "=" && (tipoId == RES_ENTERO || tipoId == RES_FLOTANTE ||
        tipoId == RES_CADENA))
    {
        sigToken();
        error = procDefExpresion();
    }
    else if (tokActual.token == "++" || tokActual.token == "--")
    {
        if (tipoId != RES_ENTERO)
            error = ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO;
        else
            sigToken();
    }
    else if (tipoId != RES_FUNCION)
        error = ERR_SEMANTICA_FUNCION_NO_DECL;

    return error;
}


string Sintaxis::mensajeError(int err)
{
    string s = "";

    switch (err)
    {
        case ERR_NO_SINTAX_ERROR :
            s = "No se encontraron errores de SINTAXIS";
            break;
        case ERR_IDENTIFICADOR :
            s = "ERROR DE SINTAXIS: se esperaba un Identificador ";
            break;
        case ERR_EOLN :
            s = "ERROR DE SINTAXIS: se esperaba un EOLN";
            break;
        case ERR_INICIO :
            s = "ERROR DE SINTAXIS: se esperaba [INICIO]";
            break;
        case ERR_FINAL :
            s = "ERROR DE SINTAXIS: se esperaba [FINAL]";
            break;
        case ERR_FINDEF :
            s = "ERROR DE SINTAXIS: se esperaba [FINDEF]";
            break;
        case ERR_PARENTESIS_ABRIR :
            s = "ERROR DE SINTAXIS: se esperaba (";
            break;
        case ERR_PARENTESIS_CERRAR :
            s = "ERROR DE SINTAXIS: se esperaba )";
            break;
        case ERR_OP_LOGICO :
            s = "ERROR DE SINTAXIS: se esperaba operador logico";
            break;
        case ERR_FINMI :
            s = "ERROR DE SINTAXIS: se esperaba [FINMI]";
            break;
        case ERR_CONDICION :
            s = "ERROR DE SINTAXIS: error en la condición";
            break;
        case ERR_FINSI :
            s = "ERROR DE SINTAXIS: se esperaba [FINSI]";
            break;

        case ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE :
            s = "ERROR DE SEMÁNTICA: identificador YA declarado";
            break;
        case ERR_SEMANTICA_IDENTIFICADOR_NO_DECL :
            s = "ERROR DE SEMÁNTICA: identificador NO declarado";
            break;
        case ERR_SEMANTICA_FUNCION_NO_DECL :
            s = "ERROR DE SEMANTICA: función NO declarada ó mal uso de idententificador";
            break;
        case ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO :
            s = "ERROR DE SEMÁNTICA: identificador debe ser Entero";
            break;
        case ERR_SEMANTICA_IDENT_FUNCION_MAL_USO :
            s = "ERROR DE SEMANTICA: identificador no declarado ó mal uso de funcion";
            break;
    }

    return s;
}


int Sintaxis::agregarIdentificador(string iden, int tipo, int linea)
{
    tToken aux;
    for (const tToken& elemento : tablaSimbolos)
        if (elemento.token == iden)
            return ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE;

    aux.tipoToken = tipo;
    aux.token = iden;
    tablaSimbolos.push_back(aux);
    return ERR_SEMANTICA_NO_ERROR;
}

vector<tToken> Sintaxis::getListaIdentificadores()
{
    return tablaSimbolos;
}

int Sintaxis::getTipoIdentificador(string iden)
{
    for (const tToken& elemento : tablaSimbolos)
        if (elemento.token == iden)
            return elemento.tipoToken;
    return RES_NO_DECL;
}

string Sintaxis::getStrTipoIdentificador(int tipo)
{
    string s = "";

    switch (tipo)
    {
        case RES_ENTERO :
            s = "ENTERO";
            break;

        case RES_FLOTANTE :
            s = "FLOTANTE";
            break;

        case RES_CADENA :
            s = "CADENA";
            break;

        case RES_FUNCION :
            s = "FUNCION";
            break;
    }

    return s;
}
void Sintaxis::imprimirErrores()
{
    if (colaErrores.empty()) {
        cout << "---------------------------------------" << endl;
        cout << "ANALISIS SINTACTICO FINALIZADO CON EXITO" << endl;
        cout << "No se encontraron errores." << endl;
        cout << "---------------------------------------" << endl;
        return;
    }

    cout << "---------------------------------------" << endl;
    cout << "ERRORES DE SINTAXIS ENCONTRADOS (" << colaErrores.size() << "):" << endl;
    cout << "---------------------------------------" << endl;

    int contador = 1;
    // Hacemos una copia para no vaciar la cola original si se necesitara despues
    // O simplemente la vaciamos como en este bucle:
    while (!colaErrores.empty()) {
        InfoError info = colaErrores.front(); // <--- CORRECCIÓN: Guardar en variable tipo InfoError

        cout << contador << ". [Codigo " << info.codigo << "] " << info.mensaje << endl;

        colaErrores.pop();
        contador++;
    }
    cout << "---------------------------------------" << endl;
}