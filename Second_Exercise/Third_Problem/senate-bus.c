//Problema escolhido:
//7.4 The Senate Bus Problem

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#define LOTACAO 50 //Capacidade máxima do ônibus

#define min(X, Y) (((X) < (Y)) ? (X) : (Y)) //Define uma macro que retornar o mínimo entre 2 valores

bool onibusNoPonto = false;
int numeroDePessoasNoOnibus = 0;
int numeroDePessoasNoPonto = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t onibusAcabouDeChegar = PTHREAD_COND_INITIALIZER;
pthread_cond_t onibusProntoParaPartir = PTHREAD_COND_INITIALIZER;

void onibus_saindo()
{
  printf("Ônibus saindo com %d pessoas\n", numeroDePessoasNoOnibus);

  numeroDePessoasNoPonto -= numeroDePessoasNoOnibus; //O número de pessoas no ponto diminui do número de pessoas no ônibus.
  onibusNoPonto = false;
  numeroDePessoasNoOnibus = 0; //Como não tem ônibus, não tem mais ninguém dentro do ônibus
}

void onibus_chegando()
{
  printf("Ônibus Chegou\n");
  onibusNoPonto = true;
}

void passageiro_esperando_o_onibus()
{
  printf("Passageiro esperando ônibus chegar\n");
  numeroDePessoasNoPonto++;
}

void passageiro_vai_viajar(bool passageiroEntrouNoOnibus)
{
  printf("Passageiro entrando no ônibus\n");
  numeroDePessoasNoOnibus++;
  passageiroEntrouNoOnibus = true;
}

bool tem_gente_no_ponto()
{
  if (numeroDePessoasNoPonto > 0)
  {
    return true;
  }

  return false;
}

void *onibus()
{

  //O mesmo ônibus anda infinitamente.
  while (1)
  {
    pthread_mutex_lock(&mutex);

    onibus_chegando();

    if (tem_gente_no_ponto())
    {
      pthread_cond_broadcast(&onibusAcabouDeChegar);      //Avisa a todos que o ônibus acabou de chegar somente se tiver gente no ponto
      pthread_cond_wait(&onibusProntoParaPartir, &mutex); //Esperamos o ônibus ficar pronto para partir
    }

    onibus_saindo();

    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

void *passageiro()
{
  //define as variáveis de passageiro (controle da funcionalidade do código)
  bool passageiroPodeEntrarNoOnibus = false;
  bool passageiroEntrouNoOnibus = false;

  while (!passageiroEntrouNoOnibus)
  {
    pthread_mutex_lock(&mutex);

    while (!onibusNoPonto)
    {

      //Se o passageiro ainda não pode entrar no ônibus.
      if (!passageiroPodeEntrarNoOnibus)
      {
        passageiro_esperando_o_onibus();
      }

      //Dizemos que o passageiro pode entrar no ônibus para evitar que ele entre mais de uma vez.
      passageiroPodeEntrarNoOnibus = true;

      //esperamos pelo sinal de que o ônibus acabou de chegar.
      pthread_cond_wait(&onibusAcabouDeChegar, &mutex);
    }

    if (passageiroPodeEntrarNoOnibus && numeroDePessoasNoOnibus < LOTACAO)
    {
      passageiro_vai_viajar(passageiroEntrouNoOnibus); //passageiro agora entrou no ônibus.
      if (numeroDePessoasNoOnibus == min(numeroDePessoasNoPonto, LOTACAO))
      {
        //Se já chegou ao mínimo entre o limite do ônibus e o número de pessoas esperando, avisamos que o ônibus está pronto para partir
        pthread_cond_signal(&onibusProntoParaPartir);
      }
    }

    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

int main(void)
{
  //Cria as threads de passageiros
  pthread_t passageirosThreads[200];
  for (int i = 0; i < 200; i++)
  {
    pthread_create(&passageirosThreads[i], NULL, passageiro, NULL);
  }

  //Cria a thread de ônibus.
  pthread_t onibusThread;
  pthread_create(&onibusThread, NULL, onibus, NULL);

  pthread_join(onibusThread, NULL);

  return 0;
}
