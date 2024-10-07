# Novo Sistema de Combate para o Epic! Shard
## Agora temos 3 tipos de danos efetivos: Cortante, Perfurante e Contusivo.
* Determinados NPCs e equipamentos possuem melhor resistência pra cada tipo de dano (postura de ataque).

Ex.: Caveira será imune contra dano de perfuração, assim como um golem de pedra (possívelmente) será imune contra cortante.

* NPCs possuem ataque e resistência baseado na mesma lógica dos players. Dano Cortante, Perfurante, Contundentes, tier de dano baseado em arma OneHand, TwoHand, DualWield, etc.

Agora o PvE está bem mais próximo do que é o PvP, com diferenças bem pontuais.

## O jogador pode escolher uma postura de combate
* Usando o comando ".golpe" abre um gump para o jogador alterar suas posturas de combate. O dano base calculado será a rolagem de dados que a arma tiver com aquela tipo.

Ex.: O jogador poderá usar uma espada com dano Cortante ou optar por Perfurante. Elas serão ajustadas baseado no realismo de cada uma.

Isso criará um combate mais tático, onde em determinada situação o dano menor (da arma) será mais efetivo, caso a resistência do defensor seja pior naquela situação.

Os NPCs também trocarão de postura ao perceberem que o jogador recebe mais dano com determinada postura.

## NPCs com armadura (com graphic humanoide) funcionam da mesma forma que os players
* A defesa do NPC agora é baseada na região atingida e se há equipamento ou não naquela parte do corpo. Caso o golpe atinja o braço e não haja armadura na região, não haverá redutor de defesa. (Ver sobre isso no Novo Sistema de Equipamentos em [/pkg/systems/equipsys/_README.md](/pkg/systems/equipsys/_README.md))