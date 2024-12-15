# Projeto 1: Simulação da Avaliação de Padawans por Yoda

---

## Descrição

Este projeto simula um cenário inspirado no universo de Star Wars, onde Padawans passam por um processo de avaliação conduzido pelo Mestre Yoda para se tornarem Jedi. O programa utiliza conceitos de programação concorrente, implementando threads e semáforos para sincronizar as interações entre Padawans, Yoda e o público que assiste aos testes.

### Bibliotecas Utilizadas

- `pthread.h`
- `semaphore.h`
- `stdio.h`
- `stdlib.h`
- `unistd.h`

---

## Compilação

Para compilar o programa, utilize o comando:

```bash
make co
```

Este comando irá compilar todos os módulos e gerar o executável `main`.

### Limpeza dos Arquivos Compilados

Para limpar os arquivos objeto e o executável gerado, utilize:

```bash
make cl
```

---

## Uso

Para executar o programa, utilize o comando:

```bash
make r
```

O programa irá iniciar a simulação, exibindo as interações entre Padawans, Yoda e o público durante o processo de avaliação.

### Exemplo de Saída

```bash
make r
Padawan 0 está aguardando para entrar no salão
Yoda liberou a entrada do Padawan
Yoda liberou a entrada do público, 5 vagas
Padawan 0 entrou no salão para realizar os testes
Padawan 0 está cumprimentando os mestres avaliadores
Espectador 0 está aguardando para entrar no salão
Espectador 1 está aguardando para entrar no salão
Padawan 0 terminou de cumprimentar os mestres avaliadores
Padawan 0 está pronto para ser avaliado
Yoda está iniciando os testes
Yoda está avaliando o Padawan
Yoda avaliou o Padawan com nota 85/100
Padawn aprovado 👍
Padawan 0 está aguardando a avaliação
Padawan 0 recebeu a avaliação
Padawan 0 está realizando a avaliação
...
```

---

## Funcionamento

### Visão Geral

O programa cria threads para representar:

- **Yoda**: Responsável por avaliar os Padawans.
- **Padawans**: Alunos que desejam se tornar Jedi.
- **Público**: Espectadores que assistem às avaliações.

A interação entre essas entidades é sincronizada usando semáforos para controlar o acesso aos recursos compartilhados e garantir a ordem correta das operações.

### Estrutura do Código

- **main.c**: Ponto de entrada do programa. Inicializa os semáforos e cria as threads para Yoda, Padawans e Público.
- **Modules**:
  - **common**:
    - `globals.h` e `main.c`: Declaração e definição dos semáforos e variáveis globais usadas para a sincronização.
  - **padawan**:
    - `interface.h` e `main.c`: Contém a lógica das ações realizadas pelos Padawans durante o teste.
  - **publico**:
    - `interface.h` e `main.c`: Contém a lógica do comportamento dos espectadores do público.
  - **yoda**:
    - `interface.h` e `main.c`: Contém a lógica das ações de Yoda durante a avaliação dos Padawans.

### Detalhamento das Interações

1. **Inicialização**:

   - O programa define o número de Padawans (`NUM_PADAWAN`) e de espectadores (`NUM_PUBLICO`).
   - Inicializa os semáforos e variáveis globais necessárias para a sincronização.

2. **Criação das Threads**:

   - São criadas threads para Yoda, cada Padawan e cada espectador do público.
   - As threads dos Padawans e do Público recebem seus identificadores como argumentos.

3. **Execução das Threads**:

   - **Yoda**:

     - Controla o fluxo de entrada dos Padawans e do público no salão.
     - Aguarda que o Padawan esteja pronto para iniciar a avaliação.
     - Realiza a avaliação e determina o resultado (aprovado ou reprovado).
     - Caso o Padawan seja aprovado, ocorre o corte da trança (símbolo da promoção a Jedi).
     - Garante que todas as etapas sejam sincronizadas corretamente usando semáforos.

   - **Padawan**:

     - Aguarda autorização para entrar no salão.
     - Cumprimenta os mestres avaliadores.
     - Prepara-se para a avaliação e aguarda sua vez.
     - Realiza a avaliação e aguarda o resultado.
     - Se aprovado, interage com Yoda para o corte da trança.
     - Sai do salão após o término do processo.

   - **Público**:

     - Aguarda disponibilidade para entrar no salão, respeitando o limite de vagas.
     - Assiste aos testes quando permitido.
     - Sai do salão.

4. **Sincronização**:

   - **Semáforos** são utilizados para controlar o acesso e a sequência das ações:

     - `sem_padawan_entrada`: Controla quando o Padawan pode entrar no salão.
     - `sem_publico_entrada`: Controla quando os espectadores podem entrar.
     - `sem_padawan_pronto`: Indica que o Padawan está pronto para ser avaliado.
     - `sem_avaliacao_andamento`: Sincroniza o início da avaliação com o Padawan e o público.
     - `sem_avaliacao_resultado`: Controla a comunicação do resultado ao Padawan.
     - `sem_aguarda_corte_trança`: Utilizado quando o Padawan é aprovado e aguarda o corte da trança.
     - `sem_padawan_saiu`: Indica que o Padawan saiu do salão.
     - `sem_vagas_publico_utilizadas`: Controla o número de vagas ocupadas pelo público.

5. **Finalização**:

   - O programa aguarda a conclusão das threads dos Padawans.
   - Yoda imprime uma mensagem final baseada no desempenho dos Padawans.
   - As threads do público são canceladas, pois seu trabalho termina com a avaliação dos Padawans.

### Mensagens de Yoda

Ao final das avaliações, Yoda exibe uma mensagem que varia de acordo com o número de Padawans aprovados:

- **Todos aprovados**: Mensagem de grande orgulho e emoção.
- **Nenhum aprovado**: Expressa tristeza e desapontamento, mas encoraja a persistência.
- **Mais da metade aprovados**: Mistura de orgulho pelos aprovados e encorajamento aos demais.
- **Menos da metade aprovados**: Reconhece os aprovados, mas destaca o desapontamento com os reprovados.

---

## Autor

Matheus Santos de Andrade
João Vitor Girotto

## Data

15/12/2024
