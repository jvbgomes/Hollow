# Hollow — Jogo de Horror Psicológico Top-Down

Trabalho final da disciplina **Linguagem de Programação I (DIM0116)** — UFRN  
Desenvolvido em **C++17** com **SFML 2.6**

---

## Descrição

Hollow é um jogo de horror psicológico top-down 2D. O jogador controla um investigador preso em uma mansão abandonada. O objetivo é coletar as **5 páginas do diário** espalhadas pelos cômodos, encontrar a **chave da saída** e escapar antes de ser alcançado pelas criaturas que habitam a mansão.

---

## Como Compilar e Executar

**Dependências:** CMake 3.16+, SFML 2.6, compilador C++17 (GCC/Clang)

```bash
# Compilar (primeira vez)
cd build
cmake ..
make

# Rebuild incremental
cd build && make

# Executar (a partir da raiz do projeto)
./build/Hollow
```

---

## Controles

| Tecla | Ação |
|---|---|
| W / A / S / D | Mover |
| Shift | Correr (consome stamina) |
| E | Interagir com NPC / avançar diálogo / usar porta |
| Q | Arremessar lamparina de sal |
| F | Usar poção de vida |
| Esc | Pausar |

---

## Objetivo e Condições de Fim de Jogo

- **Vitória:** coletar as 5 páginas do diário + obter a chave (dica: converse com Thomas no porão) + chegar à saída no corredor final
- **Derrota:** receber 3 acertos de inimigos (vida chega a zero)

---

## Mapa da Mansão

Cada cômodo é um arquivo `.tmx` separado, criado no **Tiled Map Editor**. O `Map::load()` lê o `.tmx` diretamente — o dado de tiles em CSV está embutido no próprio arquivo, sem necessidade de CSVs externos. As conexões entre cômodos são feitas por zonas de transição no chão (ativadas com E).

```
                    ┌───────────────┐
                    │   VESTÍBULO   │  ← jogador começa aqui
                    └──────┬────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
   ┌──────┴──────┐  ┌──────┴──────┐  ┌─────┴──────┐
   │   QUARTO    │  │    HALL     │  │ BIBLIOTECA │
   │  DA CRIANÇA │  │  PRINCIPAL  │  │ (Eleanor)  │
   └─────────────┘  └──────┬──────┘  └────────────┘
                           │
               ┌───────────┴───────────┐
               │                       │
        ┌──────┴──────┐         ┌──────┴──────┐
        │   PORÃO     │         │ SALA ESTAR  │
        │  (Thomas)   │         └──────┬──────┘
        └──────┬──────┘                │
               │              ┌────────┴────────┐
        ┌──────┴──────┐       │                 │
        │ PORÃO FUNDO │  ┌────┴─────┐    ┌──────┴──────┐
        │ (5ª página) │  │  ÁREA    │    │   COZINHA   │
        └──────┬──────┘  │ EXTERNA  │    └─────────────┘
               │         └──────────┘
        ┌──────┴──────┐
        │  DEPÓSITO   │
        │   (chave)   │
        └──────┬──────┘
               │
        ┌──────┴──────────────┐
        │  CORREDOR DE SAÍDA  │ ──► SAÍDA (vitória)
        └─────────────────────┘
```

Cada cômodo possui as seguintes layers de tiles:

| Layer | Função |
|---|---|
| Floor / Walls / Furniture / Props | Renderização visual (tiles 16×16) |
| Collision | Invisível — define bloqueio de movimento independentemente da aparência visual |

---

## Arquitetura de Classes

### Hierarquia de Herança

```
Entity  (abstrata — update(), draw())
├── Player       — vida, stamina, inventário, movimento WASD, arremesso (Q)
├── NPC          — filas de diálogo indexadas por opção de resposta
└── Enemy        — sistema de sprite em 8 direções
    ├── Shadow   — patrulha por waypoints, morre com 1 acerto de lamparina
    ├── Spectre  — cone de visão configurável, perseguição em linha reta, morre com 2 acertos
    └── Boss     — A Entidade, imortal, velocidade crescente com o tempo
```

### Estruturas de Dados Manuais

Implementadas sem uso de `std::list` ou `std::queue` — requisito da disciplina.

| Classe | Tipo | Descrição |
|---|---|---|
| `ItemList` | Lista encadeada manual | Armazena itens coletáveis (páginas, lamparinas, chave, poções). Cada nó possui timer de expiração. |
| `EventQueue` | Fila manual | Gerencia mensagens do HUD com tempo de exibição fixo. |

As filas de diálogo de cada `NPC` também são gerenciadas manualmente, indexadas por opção de resposta.

### Outras Classes

| Classe | Responsabilidade |
|---|---|
| `Game` | Loop principal e máquina de estados: `Intro → Menu → Playing → Paused → Victory / GameOver` |
| `Map` | Leitura de `.tmx` (Tiled), renderização de tiles 16×16, colisão tile-based via layer `Collision` |
| `Projectile` | Lamparina de sal arremessada com Q — sem efeito no Boss |
| `DialogueBox` | Caixa estilo RPG com portrait, nome do falante, typewriter e opções de escolha |
| `PageReader` | Tela de leitura do conteúdo completo das páginas do diário (lore) |
| `HUD` | Corações de vida, barra de stamina, contagem de páginas/lamparinas/poções e cronômetro pixel-art |
| `AudioManager` | Trilha ambiente por contexto (`sf::Music`) e efeitos sonoros pontuais (`sf::Sound`) |
| `HitEffect` | Partícula visual gerada no ponto de impacto de inimigos atingidos por lamparina |

---

## NPCs

| NPC | Local | Papel |
|---|---|---|
| Eleanor | Biblioteca | Dá dicas sobre a localização das páginas |
| Thomas | Porão | Sabe onde está a chave — revela com a pergunta certa |
| A Criança | Quarto | Fala em enigmas sobre A Entidade |

A aparência espectral dos NPCs é obtida via `sf::Sprite::setColor(sf::Color(180,180,180,200))` — dessaturação e transparência por multiplicação de cor, sem editar os assets originais.

---

## Inimigos

| Inimigo | Comportamento | Resistência |
|---|---|---|
| Shadow | Patrulha por waypoints | 1 acerto de lamparina |
| Spectre | Cone de visão (90°); ao detectar o jogador, persegue em linha reta | 2 acertos |
| Boss | Perseguição constante e incondicional; velocidade aumenta com o tempo | Imortal |

O **cone de visão do Spectre** é calculado pelo método `canSeePlayer()` — verifica distância e ângulo do vetor Spectre→Jogador em relação à direção que o Spectre encara. É renderizado como `sf::VertexArray` (TriangleFan) semi-transparente.

O **Boss** aparece ao coletar a última página e persiste em todos os cômodos visitados a partir desse momento. Inimigos normais eliminados em um cômodo **não reaparecem** ao retornar.

---

## Funcionalidades Implementadas

### Mecânicas de Jogo

- **Sistema de stamina** — consumida ao correr com Shift; regenera automaticamente em repouso ★
- **Poções de vida** — coletáveis e ativáveis com F para recuperar 1 ponto de vida
- **Animação de ataque em fases** — ao pressionar Q, o Player passa pelos estados `Preparing → Throwing` antes de lançar a lamparina; novos inputs são bloqueados durante a animação
- **Invulnerabilidade temporária** — após receber dano, o Player pisca por 1 segundo e não pode ser atingido novamente nesse intervalo
- **Persistência de inimigos** — inimigos eliminados em um cômodo não reaparecem ao retornar

### Visuais e Apresentação

- **Sistema de iluminação dinâmica** — cada cômodo possui fontes de luz individuais com raio, intensidade de flicker e velocidade de oscilação configuráveis; renderizadas sobre um `sf::RenderTexture` de luz
- **Efeito vignette** — escurecimento radial nas bordas da tela, com tint ajustável por contexto
- **Partículas de poeira flutuante** — visíveis no menu principal e na cena introdutória, com movimento senoidal independente por partícula
- **HitEffect** — partícula de impacto gerada na posição do inimigo ao ser atingido
- **Popup de objetivo** (`ObjPopup`) — notificação deslizante com ícone, cabeçalho e status ao coletar itens importantes

### Fluxo de Jogo

- **Cena introdutória animada** — múltiplas fases com textos, animação de portão se fechando e fade; pode ser pulada com ESC
- **Seleção de personagem** — menu de escolha entre sprite masculino e feminino antes de iniciar
- **Tela de pausa** — opções de continuar, reiniciar ou sair (ativada com ESC durante o jogo)
- **Leitura de páginas** (`PageReader`) — ao coletar uma página, abre tela dedicada com o texto completo do lore
- **Sistema de áudio por contexto** (`AudioManager`) — troca automática de trilha conforme o estado do jogo ★

---

## Funcionalidades Extras (★)

As seguintes funcionalidades vão além do requisito mínimo da disciplina:

| # | Funcionalidade |
|---|---|
| ★ | Sistema de stamina com regeneração |
| ★ | Timer de expiração de itens em cada nó da `ItemList` |
| ★ | `AudioManager` com trilhas por contexto e SFX pontuais |

---

## Divisão de Tarefas

### João Victor
- `Entity`, `Player`, `Enemy`, `Shadow`, `Spectre`, `Boss`, `Projectile`, `Map`
- Sistema de sprite em 8 direções para inimigos
- Cone de visão do Spectre
- Animação de ataque do Player (`AttackState`)
- Sistema de invulnerabilidade e piscar ao receber dano
- Sistema de iluminação dinâmica (`m_lightMap`, `LightSource`, flicker)
- Persistência de inimigos por cômodo (`m_clearedRooms`)
- Construção dos mapas no Tiled, ajustes de colisão e testes de gameplay
- Sistema de stamina ★
- Timer de expiração de itens ★

### Radla
- `Item`, `ItemList` (lista encadeada manual)
- `EventQueue` (fila manual)
- `NPC`, `DialogueBox` (typewriter + opções de escolha)
- `PageReader` (leitura de lore das páginas)
- `HUD` (vida, stamina, páginas, lamparinas, cronômetro)
- `HitEffect` (efeito de impacto)
- `ObjPopup` (popup de objetivo)
- Telas de menu, vitória e game over
- Diálogos completos de Eleanor, Thomas e A Criança
- Coleta automática de itens

### Dupla
- `Game` (loop principal, máquina de estados, cena introdutória, seleção de personagem)
- `AudioManager` ★
- Assets visuais (tileset da mansão e sprites de personagens)
- Efeito vignette e partículas de poeira
- Playtesting e ajustes finais

---

## Assets de Terceiros

| Asset | Fonte | Licença |
|---|---|---|
| Tileset "Mansion of Shadow" (16×16) | itch.io | Comercial |
| Sprites de personagem (Player/NPCs) | "Top-Down Character Sprites" — itch.io | CC0 |
| Sprites de inimigos (Shadow, Spectre, Boss) | Gerados via PixelLab (IA de pixel art) | — |
| Áudio (trilha e SFX) | freesound.org / packs itch.io | Gratuito |
