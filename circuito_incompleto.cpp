#include <fstream>
#include <utility> // para std::swap
#include "circuito.h"

///
/// As strings que definem os tipos de porta
///

// Funcao auxiliar que testa se uma string com nome de porta eh valida
// Caso necessario, converte os caracteres da string para maiusculas
bool validType(std::string& Tipo)
{
  if (Tipo.size()!=2) return false;
  Tipo.at(0) = toupper(Tipo.at(0));
  Tipo.at(1) = toupper(Tipo.at(1));
  if (Tipo=="NT" ||
      Tipo=="AN" || Tipo=="NA" ||
      Tipo=="OR" || Tipo=="NO" ||
      Tipo=="XO" || Tipo=="NX") return true;
  return false;
}

// Funcao auxiliar que retorna um ponteiro que aponta para uma porta alocada dinamicamente
// O tipo da porta alocada depende do parametro string de entrada (AN, OR, etc.)
// Caso o tipo nao seja nenhum dos validos, retorna nullptr
// Pode ser utilizada nas funcoes: Circuito::setPort, Circuito::digitar e Circuito::ler
ptr_Port allocPort(std::string& Tipo)
{
  if (!validType(Tipo)) return nullptr;

  if (Tipo=="NT") return new Port_NOT;
  if (Tipo=="AN") return new Port_AND;
  if (Tipo=="NA") return new Port_NAND;
  if (Tipo=="OR") return new Port_OR;
  if (Tipo=="NO") return new Port_NOR;
  if (Tipo=="XO") return new Port_XOR;
  if (Tipo=="NX") return new Port_NXOR;

  // Nunca deve chegar aqui...
  return nullptr;
}

///
/// CLASSE CIRCUITO
///

/// ***********************
/// Inicializacao e finalizacao @ian909041 dev
/// ***********************

Circuito::Circuito():
    Nin(), id_out(), out_circ(), ports()
{

}

Circuito::Circuito(const Circuito& C):
    Nin(), id_out(), out_circ(), ports()
{
    Nin = C.getNumInputs();
    id_out = C.id_out;
    out_circ = C.out_circ;
    ports = C.ports;
}

Circuito::~Circuito(){
    Nin = 0;
    id_out.clear();
    //out_circ().clear();
    //ports().clear();
}
/// ***********************
/// Funcoes de testagem
/// ***********************

// Retorna true se IdInput eh uma id de entrada do circuito valida (entre -1 e -NInput)
bool Circuito::validIdInput(int IdInput) const
{
  return (IdInput<=-1 && IdInput>=-getNumInputs());
}

// Retorna true se IdOutput eh uma id de saida do circuito valida (entre 1 e NOutput)
bool Circuito::validIdOutput(int IdOutput) const
{
  return (IdOutput>=1 && IdOutput<=getNumOutputs());
}

// Retorna true se IdPort eh uma id de porta do circuito valida (entre 1 e NPort)
bool Circuito::validIdPort(int IdPort) const
{
  return (IdPort>=1 && IdPort<=getNumPorts());
}

// Retorna true se IdOrig eh uma id valida para a origem do sinal de uma entrada de porta ou
// para a origem de uma saida do circuito (podem vir de uma entrada do circuito ou de uma porta)
// validIdOrig == validIdInput OR validIdPort
bool Circuito::validIdOrig(int IdOrig) const
{
  return validIdInput(IdOrig) || validIdPort(IdOrig);
}

// Retorna true se IdPort eh uma id de porta valida (validIdPort) e
// a porta estah definida (estah alocada, ou seja, != nullptr)
bool Circuito::definedPort(int IdPort) const
{
  if (!validIdPort(IdPort)) return false;
  if (ports.at(IdPort-1)==nullptr) return false;
  return true;
}

// Retorna true se IdPort eh uma porta existente (definedPort) e
// todas as entradas da porta com Id de origem valida (usa getId_inPort e validIdOrig)
bool Circuito::validPort(int IdPort) const
{
  if (!definedPort(IdPort)) return false;
  for (int j=0; j<getNumInputsPort(IdPort); j++)
  {
    if (!validIdOrig(getId_inPort(IdPort,j))) return false;
  }
  return true;
}

// Retorna true se o circuito eh valido (estah com todos os dados corretos):
// - numero de entradas, saidas e portas valido (> 0)
// - todas as portas validas (usa validPort)
// - todas as saidas com Id de origem validas (usa getIdOutput e validIdOrig)
// Essa funcao deve ser usada antes de salvar ou simular um circuito
bool Circuito::valid() const
{
  if (getNumInputs()<=0) return false;
  if (getNumOutputs()<=0) return false;
  if (getNumPorts()<=0) return false;
  for (int i=0; i<getNumPorts(); i++)
  {
    if (!validPort(i+1)) return false;
  }
  for (int i=0; i<getNumOutputs(); i++)
  {
    if (!validIdOrig(getIdOutput(i+1))) return false;
  }
  return true;
}

/// ***********************
/// Funcoes de consulta
/// ***********************

int Circuito::getNumInputs() const
{
  return Nin;
}

int Circuito::getNumOutputs() const
{
  return this->id_out.size();
}

int Circuito::getNumPorts() const
{
  return this->ports.size();
}

  // Retorna a origem (a id) do sinal de saida cuja id eh IdOutput
  // Depois de testar o parametro (validIdOutput), retorna id_out[IdOutput-1]
  // ou 0 se parametro invalido
int Circuito::getIdOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput -1)) return id_out[IdOutput -1];
  return false;
}

bool3S Circuito::getOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput-1)) return out_circ[IdOutput-1];
  return bool3S::UNDEF;
}

std::string Circuito::getNamePort(int IdPort) const
{
  if (definedPort(IdPort)) return ports[IdPort-1]->getName();
  return "??";
}

int Circuito::getNumInputsPort(int IdPort) const
{
  if (definedPort(IdPort)) return ports[IdPort-1]->getNumInputs();
  return false;
}

  // Retorna a origem (a id) da I-esima entrada da porta cuja id eh IdPort
  // Depois de testar se a porta existe (definedPort) e o indice da entrada I,
  // retorna ports[IdPort-1]->getId_in(I)
  // ou 0 se parametro invalido
int Circuito::getId_inPort(int IdPort, int I) const
{
  if (definedPort(IdPort) && validIdInput(I)) return ports[IdPort-1]->getId_in(I);
  return false;
}

/// ***********************
/// Funcoes de modificacao
/// ***********************

//falta_fazer();

/// ***********************
/// E/S de dados
/// ***********************

bool Circuito::ler(const std::string& arq)
{
  try
  {
    std::ifstream ArqCircuito;
    std::string titulo, stPorta;
    int Nin, Nout, Np;
    ArqCircuito >> titulo;
    ArqCircuito >> Nin >> Nout >> Np;
    ArqCircuito >> stPorta;

    if (!ArqCircuito.good() || titulo != "CIRCUITO" || stPorta != "PORTA") throw 1;

    for (int i = 0; i < Np; i++)
    {
        int index;
        std::string prnts, portTipo;
        ArqCircuito >> index >> prnts >> portTipo;

        if (index != i + 1 || prnts != ")" || validType(portTipo))
        {
            ArqCircuito.close();
            throw 2;
        }

        Port *P = allocPort(portTipo);
        if ((*P).ler(ArqCircuito) && validPort((*P).getId_in(index)));
    }

    std::string saidas;
    ArqCircuito >> saidas;

    if (!ArqCircuito.good() || saidas != "SAIDAS") throw 3;

    for (int i = 0; i < Nout; i++)
    {
        int index;
        std::string prnts;
        ArqCircuito >> index >> prnts;

        if (index != i + 1 || prnts != ")" || validIdOrig(index))
        {
            ArqCircuito.close();
            throw 4;
        }
    }
  }
  
  catch (int erro)
  {
    return false;
  }
  return true;
}



//falta_fazer();

/// ***********************
/// SIMULACAO (funcao principal do circuito)
/// ***********************

//falta_fazer();
