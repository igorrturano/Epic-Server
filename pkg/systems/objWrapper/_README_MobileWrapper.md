# Guia de Utilização - Sistema de Personagens

## Como Usar a Classe MobileWrapper

### Introdução
A classe `MobileWrapper` é uma ferramenta essencial para trabalhar com personagens (jogadores e NPCs) no Epic Server. Este guia mostra como utilizá-la corretamente em diferentes situações.

### Importação e Configuração

```javascript
use uo;
include "ViewModel/MobileWrapper";

// Exemplo básico de criação
function ExemploBasico(player)
    var wrapper := MobileWrapper(player);
    
    // Sempre verificar se a criação foi bem-sucedida
    if (wrapper.GetObjRef().errortext)
        SendSysMessage(player, "Erro ao criar wrapper do personagem!");
        return 0;
    endif
    
    return wrapper;
endfunction
```

### Casos de Uso Práticos

#### 1. Verificação de Status do Personagem

```javascript
// Função para exibir status completo do personagem
function ExibirStatus(player)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return; endif
    
    SendSysMessage(player, "=== STATUS DO PERSONAGEM ===");
    SendSysMessage(player, "Nome: " + wrapper.GetName());
    SendSysMessage(player, "Vida: " + wrapper.GetCurrentHP() + "/" + wrapper.GetMaxHP());
    SendSysMessage(player, "Stamina: " + wrapper.GetCurrentStamina() + "/" + wrapper.GetMaxStamina());
    SendSysMessage(player, "Mana: " + wrapper.GetCurrentMana() + "/" + wrapper.GetMaxMana());
    SendSysMessage(player, "Ouro: " + wrapper.GetGold());
    SendSysMessage(player, "Peso: " + wrapper.GetWeight());
endfunction
```

#### 2. Sistema de Verificação de Habilidades

```javascript
// Verificar se o jogador tem habilidade suficiente para uma ação
function VerificarHabilidade(player, skill_name, valor_minimo)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return 0; endif
    
    var skill_atual := wrapper.GetSkill(skill_name);
    
    if (skill_atual >= valor_minimo)
        SendSysMessage(player, "Você possui habilidade suficiente!");
        return 1;
    else
        SendSysMessage(player, "Habilidade insuficiente. Necessário: " + valor_minimo + ", Atual: " + skill_atual);
        return 0;
    endif
endfunction

// Exemplo de uso
if (VerificarHabilidade(player, MAGERY, 75.0))
    // Permitir lançar magia avançada
endif
```

#### 3. Sistema de Treinamento de Habilidades

```javascript
// Função para treinar habilidade com bônus temporário
function TreinarHabilidade(player, skill_name, bonus_temporario)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return 0; endif
    
    // Aplicar bônus temporário
    wrapper.SetSkillMod(skill_name, bonus_temporario);
    
    SendSysMessage(player, "Bônus de treinamento aplicado!");
    SendSysMessage(player, "Habilidade atual: " + wrapper.GetSkill(skill_name));
    
    // Agendar remoção do bônus após 10 minutos
    start_script("remove_bonus", {player, skill_name, 600});
    
    return 1;
endfunction

// Script para remover bônus (arquivo separado: remove_bonus.src)
program RemoveBonus(params)
    var player := params[1];
    var skill_name := params[2];
    var tempo := params[3];
    
    sleep(tempo);
    
    var wrapper := MobileWrapper(player);
    if (!wrapper.GetObjRef().errortext)
        wrapper.SetSkillMod(skill_name, 0.0);
        SendSysMessage(player, "Bônus de treinamento expirou.");
    endif
endprogram
```

#### 4. Gerenciamento de Equipamentos

```javascript
// Equipar item e aplicar modificadores
function EquiparItem(player, item)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return 0; endif
    
    // Verificar se é uma arma ou armadura
    var item_wrapper;
    if (item.isArmor())
        item_wrapper := ArmorWrapper(item);
    elseif (item.isWeapon())
        item_wrapper := WeaponWrapper(item);
    else
        SendSysMessage(player, "Item não é equipável!");
        return 0;
    endif
    
    // Aplicar modificadores
    var result := wrapper.ApplyItemModifiers(item_wrapper);
    if (result.errortext)
        SendSysMessage(player, "Erro ao aplicar modificadores: " + result.errortext);
        return 0;
    endif
    
    SendSysMessage(player, "Item equipado com sucesso!");
    return 1;
endfunction

// Desequipar item e remover modificadores
function DesequiparItem(player, item)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return 0; endif
    
    var item_wrapper;
    if (item.isArmor())
        item_wrapper := ArmorWrapper(item);
    elseif (item.isWeapon())
        item_wrapper := WeaponWrapper(item);
    endif
    
    wrapper.RemoveItemModifiers(item_wrapper);
    SendSysMessage(player, "Modificadores removidos!");
    return 1;
endfunction
```

#### 5. Sistema de NPCs Inteligentes

```javascript
// Função para NPCs reagirem baseado nas habilidades do jogador
function NPCInteracao(npc, player)
    var wrapper := MobileWrapper(player);
    if (wrapper.GetObjRef().errortext) return; endif
    
    var magery := wrapper.GetSkill(MAGERY);
    var combat := wrapper.GetSkill(TACTICS);
    
    if (magery >= 80.0)
        NPCFalar(npc, "Vejo que você é um mago experiente, " + wrapper.GetName() + "!");
        // Oferecer itens/missões para magos
    elseif (combat >= 80.0)
        NPCFalar(npc, "Um guerreiro formidável! Tenho trabalho para você.");
        // Oferecer missões de combate
    else
        NPCFalar(npc, "Olá, jovem aventureiro!");
        // Oferecer missões básicas
    endif
endfunction
```

#### 6. Sistema de Diagnóstico e Debug

```javascript
// Função para administradores diagnosticarem problemas
function DiagnosticarPersonagem(admin, target)
    var wrapper := MobileWrapper(target);
    if (wrapper.GetObjRef().errortext)
        SendSysMessage(admin, "Erro: Personagem inválido!");
        return;
    endif
    
    SendSysMessage(admin, "=== DIAGNÓSTICO COMPLETO ===");
    SendSysMessage(admin, "Tipo: " + (wrapper.IsNPC() ? "NPC" : "Jogador"));
    SendSysMessage(admin, "Nome: " + wrapper.GetName());
    SendSysMessage(admin, "Atributos - STR:" + wrapper.GetStr() + " DEX:" + wrapper.GetDex() + " INT:" + wrapper.GetInt());
    
    // Verificar habilidades críticas
    foreach skill_name in (wrapper._skills.keys())
        var base_skill := wrapper.GetSkillBase(skill_name);
        var mod_skill := wrapper.GetSkillMod(skill_name);
        if (base_skill > 0 || mod_skill != 0)
            SendSysMessage(admin, skill_name + ": " + base_skill + " (+" + mod_skill + ")");
        endif
    endforeach
endfunction
```

### Boas Práticas de Utilização

#### 1. Sempre Verificar Erros
```javascript
var wrapper := MobileWrapper(player);
if (wrapper.GetObjRef().errortext)
    // Tratar erro apropriadamente
    return 0;
endif
```

#### 2. Usar Method Chaining para Eficiência
```javascript
// Aplicar múltiplas modificações de habilidades
wrapper.SetSkillBase(MAGERY, 75.0)
       .SetSkillMod(MAGERY, 10.0)
       .SetSkillBase(TACTICS, 60.0);
```

#### 3. Separar Lógica de Negócio
```javascript
// Criar funções específicas para cada funcionalidade
function PodeUsarMagia(player, nivel_necessario)
    var wrapper := MobileWrapper(player);
    return (!wrapper.GetObjRef().errortext && wrapper.GetSkill(MAGERY) >= nivel_necessario);
endfunction
```

### Integração com Outros Sistemas

#### Com Sistema de Combate
```javascript
function CalcularDano(attacker, defender)
    var att_wrapper := MobileWrapper(attacker);
    var def_wrapper := MobileWrapper(defender);
    
    // Usar dados dos wrappers para cálculos de combate
    var dano_base := att_wrapper.GetStr() + att_wrapper.GetSkill(TACTICS);
    var defesa := def_wrapper.GetDex() + def_wrapper.GetSkill(PARRY);
    
    return Math.max(1, dano_base - defesa);
endfunction
```

#### Com Sistema de Guilds
```javascript
function AdicionarMembroGuild(guild_master, novo_membro)
    var master_wrapper := MobileWrapper(guild_master);
    var membro_wrapper := MobileWrapper(novo_membro);
    
    if (master_wrapper.GetSkill(LEADERSHIP) >= 70.0)
        // Permitir adicionar membro
        SendSysMessage(guild_master, membro_wrapper.GetName() + " foi adicionado à guild!");
    else
        SendSysMessage(guild_master, "Liderança insuficiente para gerenciar guild!");
    endif
endfunction
```
