#include "lexico.h"
#include <cctype>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

// =========================================================================
// FUNCIONES AUXILIARES
// =========================================================================

// Elimina caracteres no imprimibles (como \r) de un string
string cleanString(string s) {
    string cleaned = "";
    for (char c : s) {
        if (c >= 32 && c <= 126) { // Solo caracteres ASCII imprimibles
            cleaned += c;
        }
    }
    return cleaned;
}

string toLower(string s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

string toUpper(string s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return res;
}

// =========================================================================
// IMPLEMENTACIÓN DE LA CLASE LEXICO
// =========================================================================

Lexico::Lexico() {
    noLineas = 0;
}

Lexico::~Lexico() {
}

vector<tToken> Lexico::get() {
    return lstTokens;
}

int Lexico::getLineas() {
    return noLineas;
}

int Lexico::generaLexico(vector<char> entrada, string &errorToken, bool imprimir)
{
    int estado = 0;
    int columna = 0;
    int typToken = LIN_SIN_TIPO;
    string strToken = "";
    char car;
    int tipoError = ERR_NOERROR;

    noLineas = 1;
    lstTokens.clear();

    // Matriz de Transicion
    static int matran[15][17] = {
      // L   N   .   U   =   >   <   +   -   "  EOL  #   {   }  EOF OTRO ESP
        { 1,  2, 11, 10,  6,  8,  9, 11, 11,  3, -1, 13, 11, 11, 14, 12, 0}, // 0
        { 1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 1
        { -1, 2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 2
        { 3,  3,  3,  3,  3,  3,  3,  3,  3,  5, -1000, 3, 3, 3, -1000, 3, 3}, // 3
        { -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 4
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 5
        { -1, -1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 6
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 7
        { -1, -1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 8
        { -1, -1, -1, -1,  7,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 9
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 10
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 11
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // 12
        { 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,  0, 13, 13, 13, -1, 13, 13}, // 13
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}  // 14
    };

    for (int i = 0; i < entrada.size(); )
    {
        car = entrada[i];
        columna = tipoCaracter(car);
        int transicion = matran[estado][columna];

        if (transicion != -1)
        {
            if (transicion == -1000) { tipoError = ERR_CADENA; break; }
            else if (transicion == -1001) { tipoError = ERR_NUMERO; break; }

            estado = transicion;
            if (estado != 0 && estado != 13) strToken += car;

            i++;
        }
        else
        {
            // EL AUTOMATA SE DETUVO
            if (estado != 0)
            {
                if (estado == 1) typToken = LIN_IDENTIFICADOR;
                else if (estado == 2) typToken = LIN_NUM_ENTERO;
                else if (estado == 4) typToken = LIN_NUM_FLOTANTE;
                else if (estado == 5) typToken = LIN_CADENA;
                else if (estado == 6) typToken = LIN_IGUAL;
                else if (estado == 7 || estado == 10 || estado == 11) typToken = LIN_SIMBOLO;
                else if (estado == 8) typToken = LIN_MAYOR;
                else if (estado == 9) typToken = LIN_MENOR;
                else if (estado == 12) { tipoError = ERR_CAR_INVALIDO; break; }
                else if (estado == 14) typToken = LIN_EOF;

                if (typToken == LIN_IDENTIFICADOR) {
                    // LIMPIEZA PROFUNDA ANTES DE COMPARAR
                    string idLimpio = cleanString(strToken);
                    int bR = tipoIdentificador(toLower(idLimpio));
                    if (bR != -1) typToken = bR;
                }
                else if (typToken == LIN_SIMBOLO) {
                    string sLimpio = cleanString(strToken);
                    if (sLimpio == "+") typToken = LIN_MAS;
                    else if (sLimpio == "-") typToken = LIN_MENOS;
                }

                if (strToken != "" && tipoError == ERR_NOERROR) {
                    tToken t;
                    t.token = cleanString(strToken); // Guardamos el token limpio
                    t.tipoToken = typToken;
                    t.linea = noLineas;
                    lstTokens.push_back(t);
                }
            }

            if (columna == COL_EOLN) {
                tToken t; t.token = "EOLN"; t.tipoToken = LIN_EOLN; t.linea = noLineas;
                lstTokens.push_back(t);
                noLineas++;
                i++;
            }

            estado = 0;
            strToken = "";
        }
    }

    if (tipoError != ERR_NOERROR) {
        errorToken = strToken;
        return tipoError;
    }
    return ERR_NOERROR;
}

int Lexico::tipoCaracter(char c)
{
    // IMPORTANTE: Tratar \r como espacio para ignorarlo
    if (c == 13) return COL_ESPACIO;

    if (isalpha(c)) return COL_LETRAS;
    if (isdigit(c)) return COL_NUMEROS;
    if (c == '.') return COL_PUNTO;
    if (c == '(' || c == ')' || c == ',' || c == '*' || c == '/') return COL_UNITARIOS;
    if (c == '=') return COL_IGUAL;
    if (c == '>') return COL_MAYOR;
    if (c == '<') return COL_MENOR;
    if (c == '+') return COL_MAS;
    if (c == '-') return COL_MENOS;
    if (c == '"') return COL_COMILLAS;
    if (c == '\n') return COL_EOLN;
    if (c == '#') return COL_HASH;
    if (c == '{') return COL_LLAVE_ABRIR;
    if (c == '}') return COL_LLAVE_CERRAR;
    if (c == 0 || c == EOF) return COL_EOF;
    if (isspace(c)) return COL_ESPACIO;
    return COL_OTROS;
}

int Lexico::tipoToken(int c)
{
    return LIN_SIN_TIPO;
}

int Lexico::tipoIdentificador(string id)
{
    // Comparación directa de strings limpios
    if (id == "inicio") return RES_INICIO;
    if (id == "final") return RES_FINAL;
    if (id == "entero") return RES_ENTERO;
    if (id == "flotante") return RES_FLOTANTE;
    if (id == "cadena") return RES_CADENA;
    if (id == "si") return RES_SI;
    if (id == "sino") return RES_SINO;
    if (id == "finsi") return RES_FINSI;
    if (id == "mientras") return RES_MIENTRAS;
    if (id == "finmi") return RES_FINMI;
    if (id == "ciclo") return RES_CICLO;
    if (id == "finci") return RES_FINCI;
    if (id == "salida") return RES_SALIDA;
    if (id == "entrada") return RES_ENTRADA;
    if (id == "def") return RES_DEF;
    if (id == "findef") return RES_FINDEF;
    if (id=="retornar") return RES_RETORNAR;

    return -1;
}

string Lexico::getTipoTokenStr(int t, string tt)
{
    if (t == LIN_IDENTIFICADOR) return "IDENTIFICADOR [" + toUpper(tt) + "]";
    if (t == LIN_NUM_ENTERO) return "ENTERO";
    if (t == LIN_NUM_FLOTANTE) return "FLOTANTE";
    if (t == LIN_CADENA) return "CADENA";
    if (t == LIN_EOLN) return "EOLN";

    if (t == RES_INICIO) return "RES_INICIO";
    if (t == RES_FINAL) return "RES_FINAL";

    return "SIMBOLO/OTRO";
}

void Lexico::imprimir()
{
    cout << "---------------------------------------" << endl;
    cout << "LISTA DE TOKENS GENERADOS:" << endl;
    cout << "---------------------------------------" << endl;
    for (int i = 0; i < lstTokens.size(); i++)
    {
        cout << "Linea " << lstTokens[i].linea << ": \t"
             << lstTokens[i].token << "\t\t (" << lstTokens[i].tipoToken << ")" << endl;
    }
    cout << "---------------------------------------" << endl;
}