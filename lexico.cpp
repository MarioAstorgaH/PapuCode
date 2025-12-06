#include <iostream>
#include <list>
#include <string>
#include <cctype>
#include "lexico.h"


using namespace std;


int matrizLexico[34][17] = {
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {-1, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, -1, 4, -1000, -1000, 4, 4, 4},
    {0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {-1, -1, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1001, -1001, 8, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001, -1001},
    {-1, -1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0},
    {-1, -1, -1, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0},
    {14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, -1, -1, 14, 14, 14},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0},
    {19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, -1002, 19, 19, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1003, 0, 0, 0, 0, 0},
    {0, 0, 0, -1003, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {-1, -1, -1, -1, -1, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0, 0, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {-1, -1, -1, -1, -1, 30, -1, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, 32, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, 33, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};



vector<pair<string, int>> lstReservadas = {
    {"si", RES_SI},
    {"finsi", RES_FINSI},
    {"sino", RES_SINO},
    {"ciclo", RES_CICLO},
    {"finci", RES_FINCI},
    {"mientras", RES_MIENTRAS},
    {"finmi", RES_FINMI},
    {"def", RES_DEF},
    {"findef", RES_FINDEF},
    {"entero", RES_ENTERO},
    {"flotante", RES_FLOTANTE},
    {"cadena", RES_CADENA},
    {"salida", RES_SALIDA},
    {"entrada", RES_ENTRADA},
    {"inicio", RES_INICIO},
    {"final", RES_FINAL}
};




Lexico::Lexico()
{
    //    
}
 
Lexico::~Lexico()
{
    //
}


int Lexico::generaLexico(vector<char> tiraCars, string &errToken, bool conComentarios)
{
    int tipoError = ERR_NOERROR;
    string strToken = "";
    int typToken = LIN_SIN_TIPO;
    int iToken = 0;
    noLineas = 0;
    char car;
    int linea;

    int columna;

    tToken tokenData;
    
    while (iToken < size(tiraCars))
    {
        car = tiraCars[iToken];

        columna = tipoCaracter(car);

        if (typToken == LIN_SIN_TIPO)
        {
            typToken = tipoToken(columna);
            linea = typToken;
        }

        if (matrizLexico[linea][columna] == -1000)
        {
            tipoError = ERR_CADENA;
            break;
        }
        else if (matrizLexico[linea][columna] == -1001)
        {
            tipoError = ERR_NUMERO;
            break;
        }
        else if (matrizLexico[linea][columna] == -1002)
        {
            tipoError = ERR_COMENTANTARIO;
            break;
        }
        else if (matrizLexico[linea][columna] == -1003)
        {
            strToken = car;
            tipoError = ERR_CAR_INVALIDO;
            break;
        }
        else if (matrizLexico[linea][columna] > 0)
        {
            strToken = strToken + car;
            linea = matrizLexico[linea][columna];
            iToken = iToken + 1;
        }
        else if (matrizLexico[linea][columna] == -1)
        {
            if (typToken == LIN_IDENTIFICADOR)
            {
                int bR = tipoIdentificador(toLower(strToken));
                tokenData.tipoToken = bR;
                tokenData.token = toLower(strToken);
                lstTokens.push_back(tokenData);
            }
            else if (typToken == LIN_NUMERO || typToken == LIN_MAS ||
                typToken == LIN_MENOS || typToken == LIN_IGUAL || typToken == LIN_MAYOR || typToken == LIN_MENOR)
            {
                tokenData.tipoToken = typToken;
                tokenData.token = strToken;
                lstTokens.push_back(tokenData);
            }
            else if (typToken == LIN_ESPACIO)
                iToken = iToken + 1;
            else if (typToken == LIN_EOLN)
            {
                tokenData.tipoToken = typToken;
                tokenData.token = "[EOLN]";
                lstTokens.push_back(tokenData);
                
                noLineas = noLineas + 1; 
                iToken = iToken + 1;
            }
            else if (typToken == LIN_COMENTARIO || typToken == LIN_HASH)
            {
                strToken = strToken + car;
                iToken = iToken + 1;

                if (conComentarios)
                {
                    tokenData.tipoToken = typToken;
                    tokenData.token = strToken;
                    lstTokens.push_back(tokenData);                    
                }
            }    
            else
            {
                strToken = strToken + car;
                iToken = iToken + 1;
                tokenData.tipoToken = typToken;
                tokenData.token = strToken;
                lstTokens.push_back(tokenData);
            }
            strToken = "";
            typToken = LIN_SIN_TIPO;
        }    
    }

    errToken = strToken;
    return tipoError;
}

void Lexico::imprimir()
{
    cout << "\n   LISTA DE TOKENS\n";
    cout << "---------------------------\n";
    for (int i = 0; i < lstTokens.size(); i++)    
        cout << lstTokens[i].token << "\t\t\t\t :: " << lstTokens[i].tipoToken << "-" << getTipoTokenStr(lstTokens[i].tipoToken, lstTokens[i].token) << "\n";
}

int Lexico::tipoCaracter(char c)
{
    int car = 0;

    if (c == ' ')
        car = COL_ESPACIO; 
    else if ((c == '_') or (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') or (c == '_'))
        car = COL_LETRAS;
    else if (c >= '0' and c <= '9') 
        car = COL_NUMEROS; 
    else if (c == '.') 
        car = COL_PUNTO;
    else if (string("()[]*/,").find(c) != string::npos)    
        car = COL_UNITARIOS; 
    else if (c == '=')
        car = COL_IGUAL; 
    else if (c == '>')
        car = COL_MAYOR;
    else if (c == '<')
        car = COL_MENOR; 
    else if (c == '+')
        car = COL_MAS;
    else if (c == '-')
        car = COL_MENOS;
    else if (c == '"')
        car = COL_COMILLAS;
    else if (c == '\n')
        car = COL_EOLN;
    else if (c == '#') 
        car = COL_HASH;
    else if (c == '{')
        car = COL_LLAVE_ABRIR;
    else if (c == '}')
        car = COL_LLAVE_CERRAR;
    else if (c == '\0')
        car = COL_EOF;
    else 
        car = COL_OTROS;

    return car;

}


int Lexico::tipoToken(int c)
{
    int tipoTok = -1;

    if (c == COL_ESPACIO)
        tipoTok = LIN_ESPACIO;
    else if (c == COL_EOLN)
        tipoTok = LIN_EOLN;
    else if (c == COL_EOF)
        tipoTok = LIN_EOF;
    else if (c == COL_LETRAS)
        tipoTok = LIN_IDENTIFICADOR;
    else if (c == COL_COMILLAS)
        tipoTok = LIN_CADENA;
    else if (c == COL_NUMEROS)
        tipoTok = LIN_NUMERO;
    else if (c == COL_UNITARIOS)
        tipoTok = LIN_SIMBOLO;
    else if (c == COL_MAS)
        tipoTok = LIN_MAS;
    else if (c == COL_HASH)
        tipoTok = LIN_HASH; 
    else if (c == COL_MENOS)
        tipoTok = LIN_MENOS;
    else if (c == COL_LLAVE_ABRIR)
        tipoTok = LIN_COMENTARIO;
    else if (c == COL_PUNTO)
        tipoTok = LIN_PUNTO;
    else if (c == COL_IGUAL)
        tipoTok = LIN_IGUAL;
    else if (c == COL_MAYOR)
        tipoTok = LIN_MAYOR;
    else if (c == COL_MENOR)
        tipoTok = LIN_MENOR;
    else 
        tipoTok = LIN_SIN_TIPO;

    return tipoTok;
}

string Lexico::getTipoTokenStr(int t, string tt)
{
    string sT = "No definido";
    
    if (t == LIN_ESPACIO)
        sT = "ESPACIO";
    else if (t == LIN_IDENTIFICADOR)   
        sT = "IDENTIFICADOR [" + toUpper(tt) + "]";    // Identificador ó palabra reservada ??
    else if (t == LIN_CADENA)
        sT = "CADENA";
    else if (t == LIN_NUMERO)
        sT = "NUMERO";   //  Número Entero ó Flotante ??
    else if (t == LIN_SIMBOLO)
        sT = "SIMBOLO";   // (  )  [  ]  *  /    ,
    else if (t == LIN_MAS)
        sT = "OPERADOR SUMA";   // +   ++
    else if (t == LIN_HASH)
        sT = "COMENTARIO CORTO";
    else if (t == LIN_MENOS)
        sT = "OPERADOR RESTA";
    else if (t == LIN_COMENTARIO)
        sT = "COMENTARIO LARGO";
    else if (t == LIN_EOLN)
        sT = "FIN DE LINEA";
    else if (t == LIN_EOF)
        sT = "FIN DE ARCHIVO";
    else if (t == LIN_SIN_TIPO)
        sT = "SIN TIPO DEFINIDO";
    else if (t == LIN_PUNTO)
        sT = "PUNTO";
    else if (t == LIN_IGUAL)
        sT = "OPERADOR IGUAL";   //  =   ==
    else if (t == LIN_MAYOR)
        sT = "OPERADOR MAYOR";  //  >   >=  
    else if (t == LIN_MENOR)
        sT = "OPERADOR MENOR";  //  <   <=   <>
    else if (t >= RES_SI && t <= RES_FINAL)
        sT = "PALABRA RESERVADA [" + tt + "]";    
    else if (t >= RES_ENTERO && t <= RES_CADENA)
        sT = "TIPO DE DATO [" + tt + "]";

    return sT;
}

int Lexico::tipoIdentificador(string id)
{
    int pRes = LIN_IDENTIFICADOR;

    for (const auto& p : lstReservadas)
        if (id == p.first)
        {
            pRes = p.second;
            break;
        }
    return pRes;
}

vector<tToken> Lexico::get()
{
   return lstTokens;
}

int Lexico::getLineas()
{
    return noLineas;
}



//--------------------------------- funciones que NO existen en Python
string toLower(string s)
{
    for (char& c : s)    
        c = tolower(static_cast<unsigned char>(c));
    return s;
}

string toUpper(string s)
{
    for (char& c : s)    
        c = toupper(static_cast<unsigned char>(c));
    return s;
}
    
