//Problema escolhido:
//6.2 The Unisex Bathroom Problem

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#define max(X, Y) (((X) > (Y)) ? (X) : (Y)) //Define uma macro que retornar o máximo entre 2 valores

//Define uma struct banheiro, que diz qual sexo está dentro do banheiro, e o número de pessoas lá dentro.
//Se TemHomem = true, necessariamente tem mulher é falso para este problema. Ou vice-versa.
typedef struct
{
  bool TemHomem;
  bool TemMulher;
  int PessoasDentro;
} Banheiro;

//Define uma struct sexo, que contém os sexos masculino e feminino.
//Caso Masculino seja true, o sexo será masculino.
//Caso Feminino seja true, o sexo será feminino.
//Outros casos não são tratados
typedef struct
{
  bool Masculino;
  bool Feminino;

} Sexo;

//Define uma struct pessoa, que contém o tipo Sexo (definido acima) e se ela já foi ao banheiro ou não.
typedef struct
{
  Sexo Sexo;
  bool NaoFoiAoBanheiro;
} Pessoa;

#define CAPACITY 3 //Capacidade máxima de pessoas dentro do banheiro.

Banheiro banheiro;
int numeroDePessoas; //Variável contendo o número de pessoas que irão ao banheiro, para finalizar a thread.

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t semHomemNoBanheiro = PTHREAD_COND_INITIALIZER;
pthread_cond_t semMulherNoBanheiro = PTHREAD_COND_INITIALIZER;

void esperando_sair_do_banheiro(Pessoa pessoa)
{
  if (pessoa.Sexo.Masculino)
  {
    printf("Esperando as mulheres sairem do banheiro.\n");
    return;
  }
  printf("Esperando os homens sairem do banheiro.\n");
}

void entrando_no_banheiro(Pessoa *pessoa)
{
  //Se a pessoa entrou no Banheiro.
  //O número de pessoas no banheiro aumentam.
  banheiro.PessoasDentro++;
  if (pessoa->Sexo.Masculino)
  {
    printf("Homem Entrando no banheiro.\n");
  }
  else if (pessoa->Sexo.Feminino)
  {
    printf("Mulher Entrando no banheiro.\n");
  }
  //Após todo o processo, dizemos que a pessoa foi ao banheiro.
  pessoa->NaoFoiAoBanheiro = false;
}

void saindo_do_banheiro(Pessoa *pessoa)
{
  //Se a pessoa saiu do banheiro.
  //O número de pessoas no banheiro diminuem.
  banheiro.PessoasDentro--;

  if (pessoa->Sexo.Masculino)
  {
    printf("Homem saindo do banheiro.\n");
  }
  else if (pessoa->Sexo.Feminino)
  {
    printf("Mulher saindo do banheiro.\n");
  }
  //Após todo o processo, o número global de pessoas que querem ir ao banheiro diminui.
  numeroDePessoas--;
}

bool nao_tem_ninguem_no_banheiro()
{
  //Se não tiver ninguém dentro do banheiro.
  //Não tem homem e nem mulher.
  if (banheiro.PessoasDentro == 0)
  {
    banheiro.TemHomem = false;
    banheiro.TemMulher = false;
    return true;
  }
  return false;
}

bool tem_homem_no_banheiro()
{
  if (banheiro.TemHomem && !banheiro.TemMulher)
  {
    return true;
  }
  return false;
}

bool tem_mulher_no_banheiro()
{
  if (!banheiro.TemHomem && banheiro.TemMulher)
  {
    return true;
  }
  return false;
}

void *fila_do_banheiro(void *arg)
{
  //Enquanto tiver pessoas querendo ir no banheiro, esta thread ficará rodando.
  while (numeroDePessoas > 0)
  {
  }

  printf("\nTodas as pessoas foram ao banheiro.\n");
  return NULL;
}

void *masculino()
{
  //Toda vez que criarmos a thread masculino, ela instanciará um homem que ainda não foi ao banheiro
  Pessoa homem;
  homem.NaoFoiAoBanheiro = true;
  homem.Sexo.Masculino = true;

  while (homem.NaoFoiAoBanheiro)
  {
    pthread_mutex_lock(&mutex);

    while (tem_mulher_no_banheiro())
    {
      //Se tiver alguém do sexo oposto no banheiro, todas as pessoas que estão dentro do banheiro devem sair para então essa pessoa entrar
      esperando_sair_do_banheiro(homem);
      pthread_cond_wait(&semMulherNoBanheiro, &mutex);
    }

    //Quando conseguir entrar no banheiro, dizemos que este sexo está no banheiro
    banheiro.TemHomem = true;
    banheiro.TemMulher = false;

    //Se tiver menos pessoas que a capacidade do banheiro, podemos inserir esta pessoa lá dentro.
    if (banheiro.PessoasDentro < CAPACITY)
    {
      entrando_no_banheiro(&homem);
    }
    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_lock(&mutex);

  //Após a pessoa ter ido ao banheiro, ela pode sair.
  saindo_do_banheiro(&homem);

  //Se não tiver mais ninguém, envia um sinal que não tem mais homem no banheiro
  if (nao_tem_ninguem_no_banheiro())
  {
    pthread_cond_broadcast(&semHomemNoBanheiro);
  }

  pthread_mutex_unlock(&mutex);
  return NULL;
}

void *feminino()
{
  //Toda vez que criarmos a thread feminino, ela instanciará uma mulher que ainda não foi ao banheiro
  Pessoa mulher;
  mulher.NaoFoiAoBanheiro = true;
  mulher.Sexo.Feminino = true;

  while (mulher.NaoFoiAoBanheiro)
  {
    pthread_mutex_lock(&mutex);

    while (tem_homem_no_banheiro())
    {
      //Se tiver alguém do sexo oposto no banheiro, todas as pessoas que estão dentro do banheiro devem sair para essa pessoa entrar
      esperando_sair_do_banheiro(mulher);
      pthread_cond_wait(&semHomemNoBanheiro, &mutex);
    }

    //Quando conseguir entrar no banheiro, dizemos que este sexo está no banheiro
    banheiro.TemHomem = false;
    banheiro.TemMulher = true;

    //Se tiver menos pessoas que a capacidade do banheiro, podemos inserir esta pessoa lá dentro.
    if (banheiro.PessoasDentro < CAPACITY)
    {
      entrando_no_banheiro(&mulher);
    }

    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_lock(&mutex);

  //Após a pessoa ter ido ao banheiro, ela pode sair.
  saindo_do_banheiro(&mulher);

  //Se não tiver mais ninguém, envia um sinal que não tem mais mulher no banheiro
  if (nao_tem_ninguem_no_banheiro())
  {
    pthread_cond_broadcast(&semMulherNoBanheiro);
  }

  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(void)
{
  //Instancia um banheiro que não tem ninguém dentro.
  banheiro.PessoasDentro = 0;
  banheiro.TemHomem = false;
  banheiro.TemMulher = false;

  //Define o número de homens e mulheres que irão ao banheiro.
  int numeroDeHomens = 20;
  int numeroDeMulheres = 15;

  numeroDePessoas = numeroDeHomens + numeroDeMulheres;

  //Cria a thread da fila do banheiro.
  pthread_t filaDoBanheiro;
  pthread_create(&filaDoBanheiro, NULL, fila_do_banheiro, NULL);

  //Cria a thread de homem e mulher (com seu número correspondente)
  pthread_t homem[numeroDeHomens];
  pthread_t mulher[numeroDeMulheres];
  //For de 0 até o número máximo de homens e mulheres, neste caso, 20.
  for (int i = 0; i < max(numeroDeHomens, numeroDeMulheres); i++)
  {
    //Esses ifs irão garantir que o array de threads terá somente o número máximo de threads definido na main
    //Número de Homens e Número de mulheres.
    if (i < numeroDeHomens)
    {
      pthread_create(&homem[i], NULL, masculino, NULL);
    }
    if (i < numeroDeMulheres)
    {
      pthread_create(&mulher[i], NULL, feminino, NULL);
    }
  }

  pthread_join(filaDoBanheiro, NULL);

  return 0;
}
