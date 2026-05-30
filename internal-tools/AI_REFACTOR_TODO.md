# Product Requirements Document: NPC AI Dialogue System Redesign

**Project:** Eora RPG - NPC Dialogue System v2.0  
**Author:** Antonio Rapozo  
**Date:** December 28, 2024  
**Status:** Ready for Implementation  
**Target:** POL (Ultima Online) Game Server

---

## Executive Summary

The current NPC AI dialogue system (implemented by Freddio) suffers from three critical failures:

1. **Blocking implementation** causing NPC freezes during API calls
2. **Generic, repetitive responses** (90% identical) that break immersion
3. **No cost management** leading to budget exhaustion and system failure

Despite fast API response times (<1s), poor implementation and prompt engineering created a broken player experience. This PRD defines a complete redesign that maintains the speed while fixing quality, variety, and reliability.

**Key Metrics:**

- Current: 90% response similarity, frequent NPC freezing, $5 budget exhausted
- Target: <20% response similarity, zero perceived lag, <$60/month budget

---

## 1. Problem Statement

### 1.1 Evidence from Production

**Igor Turano's Feedback (12/25/2024):**

```
"Voltei atras de tudo, tava muito muito ruim"
"zero contexto, zero realidade com o servidor, com a nossa lore"
"Dava umas lagadinhas pq tinha um await, nao era asincrono"
"o npc ficava parado ate receber a resposta da openai"
"o prompt era: Voce eh um cidadao medieval" (too generic)
"E a chance da resposta ser igual era 90%"
"valia mais a pena escrever uma frase pronta, umas 10"
```

**Speed Clarification:**

```
"Era tao rapido q eu qria adicionar um delay para parecer natural"
"Travava por causa de token e ma implementacao"
"Ele esperava a resposta, dava um timeout e o npc nao se mexia"
```

### 1.2 Root Causes Identified

| Issue                | Root Cause                               | Impact                              |
| -------------------- | ---------------------------------------- | ----------------------------------- |
| NPC Freezing         | Synchronous HTTP calls block game thread | Players experience 2-10s lag        |
| Repetitive Responses | Generic prompts with zero variation      | 90% identical responses, boring     |
| Budget Failure       | No tracking/caps, $5 exhausted quickly   | System completely broke             |
| No Lore Integration  | Prompts don't include Eora world context | NPCs feel generic fantasy, not Eora |
| Wrong Use Cases      | Combat mobs using AI unnecessarily       | Expensive, slow, repetitive anyway  |

---

## 2. Current System Analysis

### 2.1 Code Structure (As-Is)

**Files:**

- `/pkg/systems/thor/openai.src` - API communication layer
- `/pkg/systems/thor/npcHandler.src` - Game logic handlers

**Current Flow:**

```
Player interacts with NPC
    ↓
handle_npc_dialogue() called (synchronous)
    ↓
HTTPRequest() blocks game thread (2-10 seconds)
    ↓
NPC frozen, player sees lag
    ↓
Response returned
    ↓
NPC speaks (finally)
```

**Prompt Example (Generic):**

```pol
var systemMessage := "Você é um taverneiro de RPG como Diablo.";
// That's it. No variation. No world context.
```

### 2.2 What Actually Works

✅ **API Speed:** OpenAI/Claude response time is <1 second (acceptable)  
✅ **Fallback Strategy:** Claude → OpenAI cascade exists  
✅ **Retry Logic:** 3 attempts with exponential backoff  
✅ **Character Limits:** 20-60 character constraints work well

### 2.3 What's Broken

❌ **Synchronous execution** blocks NPCs  
❌ **Zero prompt variation** creates identical responses  
❌ **No Eora lore** in prompts  
❌ **No cost tracking** or limits  
❌ **Combat AI usage** wastes money on repetitive taunts  
❌ **No heuristics** - AI used for everything including "hi"

---

## 3. Requirements

### 3.1 Functional Requirements

**FR-1: Asynchronous Execution**

- NPCs MUST respond immediately with placeholder text
- API calls MUST run in background without blocking
- NPC MUST speak final response when API returns
- Maximum perceived lag: 0 seconds (placeholder shown instantly)

**FR-2: Response Variety**

- Same NPC + same question MUST generate different responses
- Target: <20% similarity between consecutive responses
- Variation vectors: mood, activity, environment, style, relationship

**FR-3: Lore Integration**

- All prompts MUST include Eora world context
- Race-specific behavior (Drows, Orcs, Bjorske, etc.)
- Faction relationships
- Current world state (post-Calamidade, improvised settlements)

**FR-4: Cost Management**

- Daily budget cap: 500 AI calls/day (~$15-20/month)
- Graceful degradation when budget exceeded
- Usage logging for monitoring
- Fallback to template responses when over budget

**FR-5: Heuristic Layer**

- Pattern matching for common interactions (greetings, introductions, thanks)
- Template responses for simple queries (location, price)
- NPC availability system (30-40% chance of being "too busy")
- Only use AI for complex/novel questions

**FR-6: Combat Exclusion**

- Combat mobs MUST NOT use AI
- Use config-based template responses only
- Boss mobs may use AI (optional future enhancement)

### 3.2 Non-Functional Requirements

**NFR-1: Performance**

- Player-perceived response time: <1 second
- API timeout: 10 seconds maximum
- Fallback response if timeout exceeded

**NFR-2: Reliability**

- System MUST NOT break when API budget exhausted
- Fallback responses always available
- No silent failures (log all errors)

**NFR-3: Maintainability**

- Configuration-driven (not hardcoded)
- Lore context in separate config files
- Easy to add new NPC types

**NFR-4: Localization**

- All responses in Brazilian Portuguese (PT-BR)
- Maintain linguistic quirks (avoid "olá", use "ei", reticências)

---

## 4. Technical Specifications

### 4.1 Architecture (To-Be)

```
┌─────────────────────────────────────────────────────────────┐
│                    PLAYER INTERACTION                        │
└─────────────────┬───────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────────────────────────┐
│  LAYER 0: NPC Availability Check (30-40% chance busy)       │
│  → Instant dismissal: "Tô ocupado... *vira as costas*"     │
└─────────────────┬───────────────────────────────────────────┘
                  ↓ (if available)
┌─────────────────────────────────────────────────────────────┐
│  LAYER 1: Pattern Recognition (Heuristics)                  │
│  → Greetings: "E aí... que foi?"                           │
│  → Introductions: "Não preciso saber seu nome..."          │
│  → Small talk: "Do jeito que dá... *suspira*"              │
│  → Thanks/Goodbye: "*acena*"                               │
└─────────────────┬───────────────────────────────────────────┘
                  ↓ (if no match)
┌─────────────────────────────────────────────────────────────┐
│  LAYER 2: Simple Classification                             │
│  → Location query: Template response from config            │
│  → Price query: "Mostra o que quer... *espera*"            │
│  → Transaction: Open vendor menu                            │
└─────────────────┬───────────────────────────────────────────┘
                  ↓ (if complex)
┌─────────────────────────────────────────────────────────────┐
│  LAYER 3: AI Response (Asynchronous)                        │
│  1. Immediate: "*pensa um momento*"                         │
│  2. Check budget                                             │
│  3. Build varied context (mood, activity, env, style)       │
│  4. Background API call                                      │
│  5. NPC speaks when response arrives                        │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 File Structure Changes

**New/Modified Files:**

```
/pkg/systems/ai/
├── config/
│   ├── lore_context.cfg          [NEW] - Eora world lore
│   ├── npc_behavior.cfg          [NEW] - NPC availability/personality
│   ├── heuristic_patterns.cfg    [NEW] - Pattern matching responses
│   └── npc_speech.cfg            [EXISTING] - Keep for combat
├── include/
│   ├── budget.inc                [NEW] - Budget tracking functions
│   ├── heuristics.inc            [NEW] - Pattern matching
│   ├── context.inc               [NEW] - Prompt building with variation
│   └── async.inc                 [NEW] - Async handling
├── openai.src                    [MODIFY] - Keep API layer, add async
├── npcHandler.src                [MODIFY] - Add layered decision tree
└── process_response.src          [NEW] - Background response handler
```

### 4.3 Data Structures

**Budget Tracking:**

```pol
GlobalProperty: "ai_daily_usage"
struct{
    "date" := "2024-12-28",
    "count" := 0,
    "max_calls" := 500,
    "cost_estimate_usd" := 0.0
}
```

**NPC State:**

```pol
ObjProperty: "npc_ai_state"
struct{
    "last_interaction" := <timestamp>,
    "interaction_count" := 0,
    "mood" := "cansado",
    "is_busy" := 0
}
```

**Player Relationship:**

```pol
ObjProperty: "player_relationships"
struct{
    "<player_serial>" := struct{
        "first_seen" := <timestamp>,
        "visit_count" := 0,
        "last_topic" := "location"
    }
}
```

### 4.4 Configuration Examples

**lore_context.cfg:**

```ini
[World]
Name=Eora
Era=Pós-Grande Calamidade
State=Assentamentos improvisados, recursos escassos, medo constante
Tone=Sombrio, desesperador, tenso

[Races]
Drows=Tratados com receio e desconfiança
Orcs=Esperança de ajuda mas preconceito pela aparência
Bjorske=Povos do norte, mencione o Norte
Pequilho=Desprezados pelo tamanho, comparados a Hobbits
Aiglanos=Povo majoritário, todos em apuros juntos
Anoes=Ofereça bebida empoeirada para trabalhos
PovosDoLeste=Pergunte sarcasticamente sobre dragões

[Merchant]
BaseContext=Você é um comerciante Aiglano em um assentamento improvisado. \
Tudo foi construído às pressas após a Grande Calamidade. \
Suas mercadorias são escassas, caras, e você está sempre cansado e nervoso.

Knowledge=Rotas comerciais locais, preços de mercadorias básicas, \
rumores sobre segurança das estradas, tensões entre facções.

ResponseConstraints=20-60 caracteres, use reticências no lugar de vírgulas, \
evite "olá" ou "oi", prefira "ei", "hmm", "e então".
```

**heuristic_patterns.cfg:**

```ini
[Greeting]
Keywords=oi,olá,ola,bom dia,boa tarde,hello,hi
Responses=E aí... que foi? *entediado*|Fala logo... *impaciente*|\
Quer algo ou só tá passeando? *cruza os braços*|Hmm... *espera você falar*

[Introduction]
Patterns=meu nome é,me chamo,sou o,sou a,my name is,i am,i'm
Responses=Não preciso saber seu nome... tem algum problema?|\
Legal... e daí? *desinteressado*|Tá... que que você quer? *impaciente*|\
Nome não paga conta... quer algo ou não? *ríspido*

[SmallTalk]
Keywords=como vai,tudo bem,how are you,tá tudo bem
Responses=Do jeito que dá... *suspira*|\
Sobrevivendo... que nem todo mundo aqui *cansado*|\
Que diferença faz? *dá de ombros*|Tá vendo a situação... *aponta ao redor*
```

**npc_behavior.cfg:**

```ini
[Global]
BusyChatChance=40
BusyTransactionChance=5
AIBudgetDaily=500
MinInputLengthForAI=30

[Merchant]
BusyModifier=20
GreetingTolerance=Low
LoreKnowledge=Medium
BusyResponses=Tô ocupado agora... *organiza mercadorias*|\
Depois a gente fala... tenho muito trabalho|\
Não vê que tô fazendo inventário? *ignora*

[Taverneiro]
BusyModifier=10
GreetingTolerance=Medium
LoreKnowledge=High
BusyResponses=Tá vendo que tem gente esperando? *limpa canecas*|\
Depois... tô sem tempo pra conversa *cansado*
```

### 4.5 Prompt Engineering Specifications

**Variation Dimensions:**

1. **Mood (randomly selected each interaction):**

   - cansado, irritado, desconfiado, apressado, melancólico, nervoso

2. **Current Activity:**

   - organizando mercadorias empoeiradas
   - contando moedas com desconfiança
   - limpando uma ferramenta velha
   - olhando nervoso para a porta

3. **Environmental Detail:**

   - uma rajada de vento frio entra pela janela quebrada
   - ouve-se gritos distantes lá fora
   - a luz da vela tremula e quase apaga
   - alguém corre apressado na rua

4. **Response Style:**

   - Seja mais direto e ríspido que o normal
   - Fale com reticências, como se estivesse cansado demais
   - Demonstre nervosismo através de gestos entre asteriscos
   - Seja sarcasticamente educado, mas claramente desinteressado

5. **Relationship Context:**
   - First meeting: "É a primeira vez que vê esse [race]"
   - Returning: "Você já viu esse [race] antes aqui"

**Prompt Template:**

```
[BASE WORLD CONTEXT from lore_context.cfg]

[NPC TYPE CONTEXT from lore_context.cfg]

[RACE-SPECIFIC BEHAVIOR if applicable]

Seu humor atual: [RANDOM MOOD]
Você está: [RANDOM ACTIVITY]
No ambiente: [RANDOM ENVIRONMENT]
[RELATIONSHIP CONTEXT]

Estilo de resposta: [RANDOM STYLE]

Restrições: [CHARACTER LIMITS] caracteres máximo, use reticências,
evite formalidades, mantenha tom do mundo sombrio.
```

---

## 5. Implementation Plan

### 5.1 Phase 1: Foundation (Week 1)

**Goal:** Fix blocking behavior, add budget tracking

**Tasks:**

1. Create async infrastructure (`async.inc`, `process_response.src`)
2. Implement budget tracking (`budget.inc`)
3. Modify `openai.src` to support async calls
4. Update `npcHandler.src` to use async pattern
5. Add placeholder responses ("_pensa um momento_")

**Deliverables:**

- NPCs no longer freeze
- Budget cap prevents overspending
- Usage logged to file

**Testing:**

- [ ] Interact with NPC - should see placeholder immediately
- [ ] Check NPC doesn't freeze during API call
- [ ] Verify response appears after 1-2 seconds
- [ ] Exceed daily budget - should see fallback responses
- [ ] Check `ai_usage.log` file created and updated

### 5.2 Phase 2: Prompt Quality (Week 2)

**Goal:** Add lore integration and response variety

**Tasks:**

1. Create `lore_context.cfg` with Eora world lore
2. Implement prompt variation system (`context.inc`)
3. Add race-specific behavior
4. Build varied context builder function
5. Test response diversity

**Deliverables:**

- Prompts include Eora context
- Responses vary by mood/activity/environment
- Race-appropriate NPC behavior

**Testing:**

- [ ] Ask same NPC same question 10 times
- [ ] Verify <20% response similarity
- [ ] Drow interaction mentions race appropriately
- [ ] Responses reference Calamidade when relevant
- [ ] NPCs sound tired/nervous (world tone)

### 5.3 Phase 3: Heuristics Layer (Week 3)

**Goal:** Reduce AI usage for simple interactions

**Tasks:**

1. Create `heuristic_patterns.cfg`
2. Implement pattern matching (`heuristics.inc`)
3. Add NPC availability system
4. Build layered decision tree in `npcHandler.src`
5. Add simple classification (location, price queries)

**Deliverables:**

- Greetings return instant template responses
- NPCs sometimes "too busy" to chat
- Location/price queries don't use AI
- AI only for complex questions

**Testing:**

- [ ] Say "hi" - get instant dismissal (no AI)
- [ ] Introduce yourself - get "don't care" response (no AI)
- [ ] Ask "where is bank" - get template (no AI)
- [ ] Ask complex lore question - use AI
- [ ] Verify 70-80% interactions don't use AI

### 5.4 Phase 4: Combat Cleanup (Week 4)

**Goal:** Remove AI from combat completely

**Tasks:**

1. Disable AI in `handle_combat_speech()`
2. Verify config-based combat templates work
3. Remove combat-specific AI code
4. Update boss mob system (optional)

**Deliverables:**

- Combat uses only config templates
- Faster combat interactions
- Reduced AI budget usage

**Testing:**

- [ ] Attack mob - verify instant taunt response
- [ ] No API calls during combat
- [ ] Config templates provide variety
- [ ] Boss mobs have special responses (optional)

---

## 6. Success Metrics

### 6.1 Performance Metrics

| Metric             | Current           | Target            | Measurement                         |
| ------------------ | ----------------- | ----------------- | ----------------------------------- |
| Perceived Lag      | 2-10 seconds      | <1 second         | Player sees placeholder immediately |
| API Response Time  | 1-2 seconds       | 1-2 seconds       | No change needed                    |
| Response Variety   | 10% unique        | >80% unique       | Compare consecutive responses       |
| AI Usage Reduction | 100% interactions | <30% interactions | Heuristics handle 70%+              |
| Monthly Cost       | Unpredictable     | <$20              | Budget tracking logs                |

### 6.2 Quality Metrics

| Metric           | Current   | Target           | Measurement                         |
| ---------------- | --------- | ---------------- | ----------------------------------- |
| Lore Accuracy    | 0%        | 100%             | Responses mention Eora context      |
| Tone Consistency | Generic   | Dark/tense       | Responses reflect world state       |
| Character Voice  | Identical | Varied by mood   | Same NPC sounds different each time |
| Linguistic Style | Formal    | Eora-appropriate | Uses reticências, avoids "olá"      |

### 6.3 System Health Metrics

| Metric         | Current                | Target        | Measurement                    |
| -------------- | ---------------------- | ------------- | ------------------------------ |
| System Uptime  | Breaks when $ runs out | 100%          | Never breaks even over budget  |
| Error Rate     | Unknown                | <1%           | Log failures                   |
| Fallback Usage | None                   | Available     | Template responses when needed |
| Budget Alerts  | None                   | Daily reports | Log monitoring                 |

---

## 7. Testing Criteria

### 7.1 Unit Tests

**Budget System:**

```pol
Test: exceed_daily_budget
- Set budget to 5 calls
- Make 10 AI requests
- Verify: First 5 use AI, next 5 use fallback
- Verify: GlobalProperty count = 5

Test: budget_resets_daily
- Make 5 calls on Day 1
- Change system date to Day 2
- Make 1 call
- Verify: GlobalProperty count = 1 (reset)
```

**Heuristics:**

```pol
Test: greeting_pattern_match
- Input: "oi", "olá", "hello", "bom dia"
- Verify: Returns instant template (no AI call)
- Verify: Response from heuristic_patterns.cfg

Test: introduction_pattern_match
- Input: "meu nome é Antonio"
- Verify: Returns "Não preciso saber..." variant
- Verify: No AI call made
```

**Async Execution:**

```pol
Test: npc_responds_immediately
- Call handle_npc_dialogue()
- Verify: Function returns within 100ms
- Verify: NPC.Say() called with placeholder
- Verify: Background script started
```

### 7.2 Integration Tests

**Full Interaction Flow:**

```pol
Test: simple_greeting
Player: "hi"
Expected: NPC responds instantly with dismissal (no AI)

Test: complex_lore_question
Player: "O que aconteceu durante a Calamidade?"
Expected:
  1. NPC says "*pensa um momento*" (instant)
  2. API called in background
  3. NPC speaks detailed lore response (1-2s later)
  4. Response mentions Eora/Calamidade context
  5. Budget counter incremented

Test: npc_too_busy
Player: Multiple interactions with same NPC
Expected: 30-40% chance NPC says "Tô ocupado..."
```

### 7.3 Edge Cases

```pol
Test: api_timeout
- Mock API to timeout after 10s
- Verify: NPC gives fallback response
- Verify: Error logged

Test: api_failure
- Mock API to return error
- Verify: Fallback to OpenAI
- Verify: If both fail, template response

Test: budget_exhausted_mid_day
- Reach daily limit
- Player asks complex question
- Verify: Template response given
- Verify: No API call attempted
- Verify: Player doesn't see error

Test: malformed_input
- Input: empty string, very long string (>500 chars)
- Verify: Graceful handling, no crash
```

### 7.4 Player Experience Tests

**Manual QA Checklist:**

- [ ] Ask same merchant same question 5 times - responses feel different
- [ ] Interact with Drow character - NPC shows racial behavior
- [ ] Ask "where is bank" - get instant answer without thinking pause
- [ ] Ask complex lore question - NPC thinks, then gives detailed answer
- [ ] Talk to NPC during busy hours - sometimes too busy to chat
- [ ] Responses never include generic "Saudações, viajante" type phrases
- [ ] All responses use reticências, avoid formal language
- [ ] Tone consistently reflects dark/tense Eora world

---

## 8. Rollback Plan

**If implementation fails:**

1. **Keep old version backup:**

   ```bash
   cp -r pkg/systems/thor pkg/systems/thor.backup
   ```

2. **Rollback command:**

   ```bash
   rm -rf pkg/systems/ai
   mv pkg/systems/thor.backup pkg/systems/thor
   ```

3. **Quick disable:**

   ```pol
   // In npcHandler.src, add at top:
   const AI_ENABLED := 0; // Set to 0 to disable completely

   if (!AI_ENABLED)
       return "Desculpe, estou sem palavras agora.";
   endif
   ```

---

## 9. Monitoring & Maintenance

### 9.1 Daily Monitoring

**Check these logs:**

```
/logs/ai_usage.log - Budget tracking
/logs/ai_errors.log - API failures
/logs/npc_interactions.log - Interaction patterns
```

**Key Questions:**

- Are we staying under budget?
- What % of interactions use AI vs heuristics?
- Are there repeated API errors?
- Which NPCs are most popular?

### 9.2 Weekly Review

**Metrics to Review:**

- Total AI calls vs budget
- Response variety score (manual sampling)
- Player feedback (if available)
- Cost trends

**Tuning Opportunities:**

- Adjust busy_chance if NPCs too often unavailable
- Add new heuristic patterns if seeing common questions
- Expand lore context if responses feel generic
- Adjust budget if consistently under/over

### 9.3 Monthly Optimization

**Analysis:**

- Which NPC types use most AI?
- What questions are most common?
- Can we template more responses?
- Are costs predictable?

**Actions:**

- Update heuristic_patterns.cfg with common questions
- Add new lore to lore_context.cfg
- Fine-tune variation parameters
- Adjust budget cap if needed

---

## 10. Future Enhancements (Out of Scope)

**Not included in this implementation, but consider later:**

1. **MemoryKit Integration**

   - Episodic memory (NPCs remember past interactions)
   - Semantic memory (deep lore retrieval)
   - Multi-layer memory system

2. **Local LLM Hosting**

   - Run Llama 3 locally for zero ongoing cost
   - Requires server resources evaluation

3. **Fine-Tuning**

   - Train custom model on Eora lore
   - Improve response quality for specific NPCs

4. **Advanced NPC States**

   - Emotional state persistence
   - Dynamic personality evolution
   - Quest-aware responses

5. **Player Relationship System**
   - Reputation affects NPC willingness to talk
   - NPCs remember player choices
   - Faction-based dialogue trees

---

## 11. Acceptance Criteria

**This implementation is complete when:**

✅ **Functionality:**

- [ ] NPCs respond instantly with placeholder, no blocking
- [ ] Background API calls complete within 10 seconds
- [ ] Budget cap prevents spending >$20/month
- [ ] Heuristics handle 70%+ of simple interactions
- [ ] Combat uses only config templates (no AI)

✅ **Quality:**

- [ ] Same question yields <20% similar responses
- [ ] All responses include Eora lore context
- [ ] Responses reflect dark/tense world tone
- [ ] Linguistic style matches spec (reticências, informal)
- [ ] Race-specific behavior works correctly

✅ **Reliability:**

- [ ] System never breaks when budget exhausted
- [ ] Fallbacks work for all API failure modes
- [ ] Error logging captures all issues
- [ ] No silent failures

✅ **Documentation:**

- [ ] Configuration files documented
- [ ] Code comments explain async flow
- [ ] Usage logs track all AI calls
- [ ] Monitoring dashboard setup (optional)

---

## 12. Dependencies

**External APIs:**

- OpenAI API (current)
- Anthropic Claude API (fallback)
- API keys stored in environment variables

**POL Server:**

- Minimum POL version: [specify if known]
- Required modules: uo, os, util, cfgfile, polsys

**Configuration:**

- OPENAI_KEY environment variable
- ANTHROPIC_KEY environment variable (optional fallback)

---

## 13. Risk Assessment

| Risk                                            | Probability | Impact | Mitigation                             |
| ----------------------------------------------- | ----------- | ------ | -------------------------------------- |
| Async implementation breaks existing code       | Medium      | High   | Extensive testing, rollback plan       |
| Budget tracking has bugs                        | Low         | Medium | Conservative cap, daily monitoring     |
| Heuristics too aggressive (blocks valid AI use) | Medium      | Low    | Start conservative, tune based on logs |
| Prompt variation doesn't improve quality        | Low         | Medium | A/B test before full rollout           |
| API costs exceed estimates                      | Low         | Medium | Hard daily cap prevents runaway costs  |
| Players dislike new NPC behavior                | Low         | High   | Gather feedback, iterate               |

---

## 14. Glossary

**Eora** - The fantasy world/setting for this RPG server  
**Calamidade** - Major catastrophic event in Eora lore  
**POL** - Penultimate Online (Ultima Online emulator)  
**NPC** - Non-Player Character  
**Heuristic** - Pattern-matching rule (no AI needed)  
**Template Response** - Pre-written response from config  
**Async** - Asynchronous (non-blocking) execution  
**Fallback** - Alternative response when primary system fails

---

**Document Status:** ✅ Ready for Implementation by Claude Code

**Next Steps:**

1. Review and approve PRD
2. Begin Phase 1 implementation
3. Test thoroughly before production deployment
4. Monitor metrics post-launch
