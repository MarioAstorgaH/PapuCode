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
    //    
}


int Sintaxis::generaSintaxis()
{   // SI!!! esta función de una sola linea es inutil, pero necesaria para compatibilidad con la version Python!
    return procPrincipal();
}

void Sintaxis::sigToken()
{
    int iAux;
    tokActual = lstTokens[iToken];
    if (tokActual.tipoToken == LIN_EOLN)
    {
        iAux = iToken + 1;
        while (lstTokens[iAux].tipoToken == LIN_EOLN)
            iAux += 1;
        iToken = iAux;
    }
    else if (tokActual.tipoToken != LIN_EOF)
        iToken++;

}

int Sintaxis::procPrincipal()
{
    int error = ERR_NO_SINTAX_ERROR;
    int tipoIdent;
    string ident;

    sigToken();

    while (error == ERR_NO_SINTAX_ERROR && !(tokActual.tipoToken == RES_INICIO || tokActual.tipoToken == LIN_EOF)) 
    {
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
                    if (error == ERR_SEMANTICA_NO_ERROR)
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
                    if (error == ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE)
                        break;
                    sigToken();
                    error = procInstrucciones();

                    if (error == ERR_NO_SINTAX_ERROR)
                    {
                        if (tokActual.tipoToken == RES_FINDEF)
                        {
                            sigToken();
                            if (tokActual.tipoToken == LIN_EOLN)
                                sigToken();
                            else 
                                error = error = ERR_EOLN;
                        }
                        else 
                            error = ERR_FINDEF;
                    }
                }
                else 
                    error = ERR_EOLN;
            }
            else 
                error = ERR_IDENTIFICADOR;
        }
        else if (tokActual.tipoToken == LIN_EOLN)
            sigToken();
        else
            error = ERR_INICIO;
    }

    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == RES_INICIO)
    {
        sigToken();
        if (tokActual.tipoToken == LIN_EOLN)
        {
            sigToken();
            error = procInstrucciones();
        }
        else 
            error = ERR_EOLN;

        if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken != RES_FINAL)
            error = ERR_FINAL;
    }        
    return error;
}

int Sintaxis::procInstrucciones()
{
    int error = ERR_NO_SINTAX_ERROR;

    while (error == ERR_NO_SINTAX_ERROR && !(tokActual.tipoToken == RES_FINAL || tokActual.tipoToken == RES_FINDEF))
    {
        switch (tokActual.tipoToken)
        {
            case RES_SI :
                error = procDefSi();
                break;
            case RES_CICLO :
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
        }

        if (error == ERR_NO_SINTAX_ERROR)
        {
            if (tokActual.tipoToken == LIN_EOLN)
            {
                sigToken();
                if (tokActual.tipoToken == RES_FINMI || tokActual.tipoToken == RES_FINDEF || 
                    tokActual.tipoToken ==  RES_FINAL || tokActual.tipoToken ==  RES_FINSI) 
                    break;
            }
            else 
                error = ERR_EOLN;
        }
    }        

    return error;
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
        error = procInstrucciones();
        if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == RES_FINSI)
            sigToken();
        else
            error = ERR_FINSI;
    }        
    else 
        error = ERR_CONDICION;
        
    return error;
}

int Sintaxis::procDefMientras()
{
    int error = ERR_NO_SINTAX_ERROR;

    error = procDefCondicion();
    if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == LIN_EOLN)
    {
        sigToken();
        error = procInstrucciones();
        if (error == ERR_NO_SINTAX_ERROR && tokActual.tipoToken == RES_FINMI)
            sigToken();
        else
            error = ERR_FINMI;
    }
    else 
        error = ERR_CONDICION;
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
                ;
            else if (tokActual.token == ")")
            {
                sigToken();
                break;
            }
            else 
                error = ERR_PARENTESIS_CERRAR; 
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



/*

std::vector<MiStruct> lista = {
    {1, "hola"},
    {2, "mundo"},
    {3, "C++"}
};

for (const MiStruct& a : lista) {
    std::cout << a.numero << " - " << a.texto << std::endl;
}



for (size_t i = 0; i < lista.size(); ++i) {
    std::cout << lista[i].numero << " - " << lista[i].texto << std::endl;
}
*/