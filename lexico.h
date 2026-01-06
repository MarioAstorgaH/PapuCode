#ifndef LEXICO_H
#define LEXICO_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

// --- CÓDIGOS DE ERROR GENERALES ---
#define ERR_NOERROR 0
#define ERR_NO_SINTAX_ERROR 0
#define ERR_SEMANTICA_NO_ERROR 0

// --- ERRORES SINTÁCTICOS Y LÉXICOS ---
#define ERR_INICIO 3
#define ERR_FINAL 4
#define ERR_FINDEF 5
#define ERR_PARENTESIS_ABRIR 6
#define ERR_PARENTESIS_CERRAR 7
#define ERR_OP_LOGICO 8
#define ERR_FINMI 9
#define ERR_CONDICION 10
#define ERR_FINSI 11
#define ERR_EOLN 12
#define ERR_IDENTIFICADOR 13
#define ERR_INSTRUCCION_DESCONOCIDA 14
// Errores léxicos específicos
#define ERR_CAR_INVALIDO 15
#define ERR_CADENA 16
#define ERR_NUMERO 17
#define ERR_COMENTANTARIO 18

// --- ERRORES SEMÁNTICOS ---
#define ERR_SEMANTICA_IDENTIFICADOR_YA_EXISTE 100
#define ERR_SEMANTICA_IDENTIFICADOR_NO_DECL 101
#define ERR_SEMANTICA_IDENTIFICADOR_NO_ENTERO 102
#define ERR_SEMANTICA_FUNCION_NO_DECL 103
#define ERR_SEMANTICA_IDENT_FUNCION_MAL_USO 104
#define ERR_SEMANTICA_TIPOS_INCOMPATIBLES 105

// --- VALORES DE RETORNO DE TIPOS (IDENTIFICADORES/VARIABLES) ---
#define RES_ENTERO 20
#define RES_FLOTANTE 21
#define RES_CADENA 22
#define RES_FUNCION 23
#define RES_NO_DECL -1

// --- PALABRAS RESERVADAS (TOKENS ESTRUCTURALES) ---
#define RES_INICIO 200
#define RES_FINAL 201
#define RES_DEF 202
#define RES_FINDEF 203
#define RES_SI 204
#define RES_FINSI 205
#define RES_SINO 206
#define RES_MIENTRAS 207
#define RES_FINMI 208
#define RES_CICLO 209
#define RES_SALIDA 210
#define RES_ENTRADA 211
#define RES_FINCI 212
#define RES_RETORNAR 213

// --- COLUMNAS DEL AUTÓMATA (COL) ---
// Estos valores deben coincidir con las columnas de tu matriz en lexico.cpp
#define COL_LETRAS 0
#define COL_NUMEROS 1
#define COL_PUNTO 2
#define COL_UNITARIOS 3
#define COL_IGUAL 4
#define COL_MAYOR 5
#define COL_MENOR 6
#define COL_MAS 7
#define COL_MENOS 8
#define COL_COMILLAS 9
#define COL_EOLN 10
#define COL_HASH 11
#define COL_LLAVE_ABRIR 12
#define COL_LLAVE_CERRAR 13
#define COL_EOF 14
#define COL_OTROS 15
#define COL_ESPACIO 16  // Si usas lógica especial para espacios

// --- TIPOS DE TOKENS LÉXICOS (LIN) ---
#define LIN_SIN_TIPO 0
#define LIN_IDENTIFICADOR 50
#define LIN_NUMERO 51
#define LIN_NUM_ENTERO 51
#define LIN_NUM_FLOTANTE 52
#define LIN_CADENA 53
#define LIN_EOLN 60
#define LIN_EOF 61
// Simbolos y Operadores
#define LIN_MAS 70
#define LIN_MENOS 71
#define LIN_IGUAL 72
#define LIN_MAYOR 73
#define LIN_MENOR 74
#define LIN_SIMBOLO 75
#define LIN_COMENTARIO 76
#define LIN_HASH 77
#define LIN_ESPACIO 78
#define LIN_PUNTO 79

struct tToken {
    int tipoToken;
    string token;
    int linea;
};

class Lexico {
private:
    vector<tToken> lstTokens;
    int noLineas; // Restaurado a noLineas para coincidir con tu cpp

    // Métodos privados internos del léxico
    int tipoCaracter(char c);
    int tipoToken(int c);
    int tipoIdentificador(string id);
    string getTipoTokenStr(int t, string tt);

public:
    Lexico();
    ~Lexico();

    vector<tToken> get();
    int generaLexico(vector<char> entrada, string &errorToken, bool imprimir);
    int getLineas(); // Asumiendo que en cpp se llama getLineas()
    void imprimir();
    string toLowe();
};

#endif