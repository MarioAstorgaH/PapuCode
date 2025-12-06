#ifndef LEXICO_H
#define LEXICO_H


#include <iostream>
#include <vector>

using namespace std;




//---------------------------------------- columnas
#define COL_ESPACIO         0
#define COL_LETRAS          1
#define COL_NUMEROS         2
#define COL_PUNTO           3
#define COL_UNITARIOS       4
#define COL_IGUAL           5
#define COL_MAYOR           6
#define COL_MENOR           7
#define COL_MAS             8
#define COL_MENOS           9
#define COL_COMILLAS        10
#define COL_OTROS           11
#define COL_EOLN            12
#define COL_EOF             13
#define COL_HASH            14
#define COL_LLAVE_ABRIR     15
#define COL_LLAVE_CERRAR    16

//------------------------------------------- lineas 
#define LIN_ESPACIO         0
#define LIN_IDENTIFICADOR   1
#define LIN_CADENA          3
#define LIN_NUMERO          5   
#define LIN_SIMBOLO         9
#define LIN_MAS             10
#define LIN_HASH            13
#define LIN_MENOS           15
#define LIN_COMENTARIO      18
#define LIN_EOLN            21
#define LIN_EOF             22
#define LIN_SIN_TIPO        23
#define LIN_PUNTO           24
#define LIN_IGUAL           25
#define LIN_MAYOR           28
#define LIN_MENOR           31

#define LIN_NUM_ENTERO      100
#define LIN_NUM_FLOTANTE    101


//---------------------- reservadas
#define RES_SI              3000
#define RES_FINSI           3001
#define RES_SINO            3002

#define RES_CICLO           3003
#define RES_FINCI           3004

#define RES_MIENTRAS        3005
#define RES_FINMI           3006

#define RES_DEF             3007
#define RES_FINDEF          3008

#define RES_SALIDA          3012
#define RES_ENTRADA         3013

#define RES_INICIO          3014
#define RES_FINAL           3015


//---------------------------- tipos
#define RES_ENTERO          4000
#define RES_FLOTANTE        4001
#define RES_CADENA          4002
#define RES_FUNCION         4003
#define RES_NO_DECL         4004




//---------------------------------------- codigos de error
#define ERR_NOERROR         0
#define ERR_CADENA          -1000
#define ERR_NUMERO          -1001
#define ERR_COMENTANTARIO   -1002
#define ERR_CAR_INVALIDO    -1003


struct tToken
{
    int tipoToken;
    string token;
};


class Lexico
{
private :
    int noLineas;
    //list<char> tiraCars;
    vector<tToken> lstTokens;

public :
    Lexico();
    ~Lexico();

    int generaLexico(vector<char> tiraCars, string &errToken, bool conComentarios);
    int tipoCaracter(char c);
    int tipoToken(int c);
    vector<tToken> get();

    string getTipoTokenStr(int t, string tt);
    int tipoIdentificador(string id);

    int getLineas();
    void imprimir();
};


string toLower(string s);
string toUpper(string s);

#endif