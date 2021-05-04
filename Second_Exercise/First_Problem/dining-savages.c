//Problema escolhido:
//5.1 The Dining Savages problem

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#define M 2 //Número de porções na panela.

int porcoes = M; //Define as porções como M

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t panelaVazia = PTHREAD_COND_INITIALIZER;
pthread_cond_t panelaCheia = PTHREAD_COND_INITIALIZER;

void come()
{
  printf("Selvagem está comendo.\n");
}

void tira_porcoes_da_panela()
{
  porcoes--;
}

//Dado um valor de m, enche a panela com m porções
void enche_a_panela()
{
  while (porcoes != M)
  {
    porcoes++;
  }
  printf("Panela está cheia.\n");
}

void *selvagem()
{

  while (true)
  {
    pthread_mutex_lock(&mutex);

    //Caso o número de porções seja 0, envia um sinal que a penal está vazia e espera por ela encher.
    while (porcoes == 0)
    {
      pthread_cond_signal(&panelaVazia);
      pthread_cond_wait(&panelaCheia, &mutex);
    }

    //Retira uma porção da panela e come.
    tira_porcoes_da_panela();
    pthread_mutex_unlock(&mutex);
    come();
  }
  return NULL;
}

void *cozinheiro()
{
  while (true)
  {
    pthread_mutex_lock(&mutex);

    //Enquanto o número de porções não for 0, espera pelo sinal de panela vazia.
    while (porcoes > 0)
    {
      pthread_cond_wait(&panelaVazia, &mutex);
    }

    //Quando a panela estiver vazia, o cozinheiro enche a panela e envia um sinal de panela cheia.
    enche_a_panela();
    pthread_cond_signal(&panelaCheia);
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

int main(void)
{

  //Cria a thread de cozinheiro
  pthread_t cozinheiroThread;
  pthread_create(&cozinheiroThread, NULL, cozinheiro, NULL);

  pthread_t selvagensThread[10];
  for (int i = 0; i < 10; i++)
  {
    //Cria 10 selvagens.
    pthread_create(&selvagensThread[i], NULL, selvagem, NULL);
  }
  //Da join na thread de cozinheiro.
  pthread_join(cozinheiroThread, NULL);

  return 0;
}
