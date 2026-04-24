use uo;
use os;
use util;
use vitals;

include ":timedscripts:timedScripts";
include ":attributes:attributes";
include "include/client";
include ":medicine:util/diseaseConstants";
include ":charactercreation:resists";

function ScheduleNextDiseaseEffect(subject, disease_grade)
	// Base: 3-5 horas para leve, 1-3 horas para moderado, 30min-1hora para grave
	var min_minutes, max_minutes;

	case (disease_grade)
		DISEASE_GRADES::LIGHT:
			min_minutes := 180; // 3 horas
			max_minutes := 300; // 5 horas
			break;
		DISEASE_GRADES::MODERATE:
			min_minutes := 60; // 1 hora
			max_minutes := 180; // 3 horas
			break;
		DISEASE_GRADES::SERIOUS:
			min_minutes := 30; // 30 minutos
			max_minutes := 60; // 1 hora
			break;
		default: min_minutes := 120; max_minutes := 240;
	endcase

	// Adiciona variação aleatória no intervalo definido
	var next_effect_minutes := RandomInt(max_minutes - min_minutes + 1) + min_minutes;

	// Converte para segundos do jogo
	var next_effect_time := next_effect_minutes * 60;

	// Programa o próximo efeito
	TS_StartTimer(subject, "diseaseeffect", next_effect_time, 1);
	return 1;
endfunction

function InfectMobile(subject,duration,attacker,level,diseases,disease_grade, deveIgnorarResistencia := 0)

	var dif := level * 8;
    var bonus_resistencia := 0;
    var imune := 0;

// Calcula bônus de resistência se não deve ignorar
    if (!deveIgnorarResistencia)
        bonus_resistencia := CalcularBonusResistencia(subject);
        
        // Verifica imunidade (valor 999)
        if (bonus_resistencia == 999)
            imune := 1;
            SendSysMessageEx(subject, "Sua máscara de peste e conhecimento em medicina o protegem completamente da doença.", SSM_INFO);
            return 0;
        endif
        
        // Aplica o bônus na dificuldade (reduz a dificuldade)
        // O bônus reduz a dificuldade do teste, tornando mais fácil resistir
        dif := dif - bonus_resistencia;
        
        // Garante que a dificuldade não fique negativa ou menor que RESIST_EASY
        if (dif < 0)
            dif := 0;
        endif
    endif
    

	var resistcheck := deveIgnorarResistencia ? 0 : CheckResist(subject, FORTITUDE, dif);
	
	
	if (resistcheck > 0)
		SendSysMessageEx(subject, "Você resiste aos efeitos da doença.", SSM_INFO);
	else
		
		if (duration == 0)
			duration := level*1000;
		endif
		
		var timer_disease := TS_GetTimerInfo(subject, "disease");
		if (timer_disease)//SE JÁ ESTÁ INFECTADO
			var duration_part := duration / 3;
			duration_part := CInt(duration_part);
			var level_infection := timer_disease.level;
			var diseases_system := GetObjProperty(subject, "Diseases_System");
			var falhou_cura := diseases_system.falhou_cura;
			var limit_duration := TS_GetTimerDuration(subject, "disease") ;
				if (falhou_cura && limit_duration <= 6000)
					UpdateDiseaseProperty(subject, "falhou_cura", 0);
					TS_AddDuration(subject, "disease", duration_part);
					return;
				elseif (level <= level_infection && limit_duration <= 6000)
					TS_AddDuration(subject, "disease", duration_part);
					SendSysMessageEx(subject, "AO TEMPO FOI ADICIONADO "+duration_part+" SEGUNDOS, LEMBRE OS SCRIPTERS QUE PRECISA CORRIGIR PARA ATUALIZAR NA BARRA BUFF/DEBUFF", SSM_FAIL);
					return;
				elseif (level > level_infection)
					TS_LowerDuration(subject, "disease", -1);
					SendSysMessageEx(subject, "Você contraiu uma infecção ainda mais gravosa!", SSM_FAIL);
					sleepms(2);
					InfectMobile(subject,duration,attacker,level,diseases,disease_grade);
				else
					SendSysMessageEx(subject, "A sua infecção não tem como ficar pior", SSM_FAIL);
					return;
				endif
		endif
		
		if (diseases == "random")
			GenerateRandomDiseases(level);
		endif
		
		UpdateDiseaseProperty(subject, "diseases", diseases);
		UpdateDiseaseProperty(subject, "grade", disease_grade);
		
		var retornoTimer := TS_StartTimer(subject, "disease",duration,level, attacker); // ALTERAR TESTE

		if(!retornoTimer)
			return retornoTimer;
		endif

	endif
	
	return 1;
endfunction

function GetAllDiseases()

	return array{
				DISEASES::FEVER,
				DISEASES::THROMBOSIS,
				DISEASES::PUSTULES,
				DISEASES::LACERATION,
				DISEASES::ITCHING,
				DISEASES::COLIC,
				DISEASES::PALPITATIONS,
				DISEASES::NAUSEA,
				DISEASES::HEADACHE
				};
endfunction

// Função para gerar sintomas aleatórios baseados no nível
function GenerateRandomDiseases(level)
	var all_diseases := GetAllDiseases();

	var diseases := array{};
	var max_symptoms := Min(level * 2, 9); // Máximo de sintomas baseado no nível
	var chance_per_symptom := 40 + (level * 10); // Base 40% + 10% por nível

	// Embaralha o array de doenças
	all_diseases := RandomizeArray(all_diseases);

	// Seleciona sintomas aleatórios baseados no nível e chance
	foreach disease in all_diseases
		Sleepms(2);
		if (diseases.size() >= max_symptoms)
			break;
		endif

		if (RandomInt(100) < chance_per_symptom)
			diseases.append(disease);
		endif
	endforeach

	// Garante pelo menos um sintoma
	if (diseases.size() == 0)
		diseases.append(all_diseases[RandomInt(all_diseases.size()) + 1]);
	endif

	return diseases;
endfunction

// Função para embaralhar um array (Fisher-Yates shuffle)
function RandomizeArray(array_to_shuffle)
	var size := array_to_shuffle.size();
	var shuffled := array{};

	// Cria uma cópia do array original
	foreach item in array_to_shuffle
		shuffled.append(item);
		Sleepms(2);
	endforeach

	// Embaralha o array usando um loop while ao invés de for
	var i := size;
	while (i >= 2)
		var j := RandomInt(i) + 1;
		var temp := shuffled[i];
		shuffled[i] := shuffled[j];
		shuffled[j] := temp;
		i := i - 1;
		Sleepms(2);
	endwhile

	return shuffled;
endfunction

// Aplica efeitos baseados nas doenças e gravidade
function ApplyDiseaseEffects(mobile, diseases, grade)
	// Primeiro remove quaisquer efeitos existentes
	RemoveDiseaseEffects(mobile);

	var strength_mod := 0;
	var dex_mod := 0;
	var int_mod := 0;
	var regen_mod := 0;

	// Multiplicador de efeito baseado na gravidade
	var effect_mult := 1;
	if (grade == DISEASE_GRADES::MODERATE)
		effect_mult := 2;
	elseif (grade == DISEASE_GRADES::SERIOUS)
		effect_mult := 3;
	endif

	// Aplica efeitos baseados nos sintomas
	foreach disease in diseases
		Sleepms(2);
		case (disease)
			DISEASES::FEVER:
				strength_mod -= 5 * effect_mult;
				regen_mod -= 1 * effect_mult;
				break;
			DISEASES::THROMBOSIS:      dex_mod -= 5 * effect_mult; break;
			DISEASES::PUSTULES:
				// Efeito visual apenas
				break;
			DISEASES::LACERATION:
				strength_mod -= 3 * effect_mult;
				dex_mod -= 3 * effect_mult;
				break;
			DISEASES::ITCHING:     int_mod -= 3 * effect_mult; break;
			DISEASES::COLIC:
				strength_mod -= 2 * effect_mult;
				dex_mod -= 2 * effect_mult;
				int_mod -= 2 * effect_mult;
				break;
			DISEASES::PALPITATIONS: dex_mod -= 4 * effect_mult; break;
			DISEASES::NAUSEA:
				int_mod -= 4 * effect_mult;
				regen_mod -= 1 * effect_mult;
				break;
			DISEASES::HEADACHE:    int_mod -= 5 * effect_mult; break;
		endcase
	endforeach

	// Aplica modificações de atributos
	if (strength_mod != 0)
		AP_ModifyStatMod(mobile, STRENGTH, strength_mod);
		UpdateDiseaseProperty(mobile, "StrDiseaseMod", strength_mod);
	endif

	if (dex_mod != 0)
		AP_ModifyStatMod(mobile, DEXTERITY, dex_mod);
		UpdateDiseaseProperty(mobile, "DexDiseaseMod", dex_mod);
	endif

	if (int_mod != 0)
		AP_ModifyStatMod(mobile, INTELLIGENCE, int_mod);
		UpdateDiseaseProperty(mobile, "IntDiseaseMod", int_mod);
	endif

	if (regen_mod != 0)
		// Implementar mod de regeneração se seu sistema suportar
		UpdateDiseaseProperty(mobile, "RegenDiseaseMod", regen_mod);
	endif
endfunction

// Função para remover efeitos de doença
function RemoveDiseaseEffects(mobile)
	var diseases_system := GetObjProperty(mobile, "Diseases_System");
	var falhou_cura := diseases_system.falhou_cura;

	// Remove modificadores de atributos anteriores
	var str_mod := diseases_system.StrDiseaseMod;
	if (str_mod)
		AP_ModifyStatMod(mobile, STRENGTH, -str_mod);
		UpdateDiseaseProperty(mobile, "StrDiseaseMod", 0);
	endif

	var dex_mod := diseases_system.DexDiseaseMod;
	if (dex_mod)
		AP_ModifyStatMod(mobile, DEXTERITY, -dex_mod);
		UpdateDiseaseProperty(mobile, "DexDiseaseMod", 0);
	endif

	var int_mod := diseases_system.IntDiseaseMod;
	if (int_mod)
		AP_ModifyStatMod(mobile, INTELLIGENCE, -int_mod);
		UpdateDiseaseProperty(mobile, "IntDiseaseMod", 0);
	endif

	var regen_mod := diseases_system.RegenDiseaseMod;
	if (regen_mod)
		// Remover mod de regeneração se implementado
		UpdateDiseaseProperty(mobile, "RegenDiseaseMod", 0);
	endif
endfunction

function UpdateDiseaseProperty(mobile, property_name, new_value)
    var diseases_system := GetObjProperty(mobile, "Diseases_System");
    if (!diseases_system)
        diseases_system := struct{};
    endif
    
    // Atualiza apenas a propriedade especificada
    diseases_system[property_name] := new_value;
    
    // Salva de volta
    SetObjProperty(mobile, "Diseases_System", diseases_system);
    
    return 1;
endfunction

// Função para calcular bônus baseado na skill de Medicine
function CalcularBonusMedicine(medicine_skill)
    if (medicine_skill >= 100)
        return 6;
    elseif (medicine_skill >= 90)
        return 5;
    elseif (medicine_skill >= 80)
        return 4;
    elseif (medicine_skill >= 70)
        return 3;
    elseif (medicine_skill >= 60)
        return 2;
    elseif (medicine_skill >= 50)
        return 1;
    else
        return 0;
    endif
endfunction

// Função para verificar se o próprio subject tem incensário/bastão equipado
function VerificarIncensarioProprio(subject)
    var equip_waist := GetEquipmentByLayer(subject, 0x0C);
    var medicine_skill := AP_GetSkill(subject, MEDICINE);
    
    // Verifica incensário equipado na cintura
    if (equip_waist && equip_waist.graphic == PROTECTION_ITEMS::CENSER)
        return CalcularBonusMedicine(medicine_skill);
    endif
    
    return 0;
endfunction

// Função para verificar se há alguém com incensário próximo
function VerificarIncensarioProximo(subject)
    var bonus_max := 0;
    var mobiles_proximos := ListMobilesNearLocation(subject.x, subject.y, subject.z, 5, subject.realm);
    
    foreach mobile in mobiles_proximos
        var equip_waist := GetEquipmentByLayer(mobile, 0x0C); // Layer da cintura
        
        // Verifica incensário (range 5) - equipado na cintura
        if (equip_waist && equip_waist.graphic == PROTECTION_ITEMS::CENSER)
            var medicine_skill := AP_GetSkill(mobile, MEDICINE);
            var bonus_atual := CalcularBonusMedicine(medicine_skill);
            var distancia := Distance(subject, mobile);
            
            if (distancia <= 5 && bonus_atual > bonus_max)
                bonus_max := bonus_atual;
            endif
        endif
    endforeach
    
    return bonus_max;
endfunction

// Função para verificar bônus de resistência a doenças
function CalcularBonusResistencia(subject)
    var primarias := GetPrimarias(subject);
    var medicine_skill := AP_GetSkill(subject, MEDICINE);
    var equip_helm := GetEquipmentByLayer(subject, 0x06); // Layer de helm/capacete
    var bonus_total := 0;
    
    // Verifica máscara de peste
    if (equip_helm && (equip_helm.objtype == PROTECTION_ITEMS::PLAGUE_MASK_1 || equip_helm.objtype == PROTECTION_ITEMS::PLAGUE_MASK_2))
        // Se tem Medicine primária, fica imune (retorna valor especial)
        if ("Medicine" in primarias || subject.cmdlevel > 3)
            return 999; // Valor especial indicando imunidade
        // Se não é primária, calcula bônus baseado na skill
        else
            bonus_total += CalcularBonusMedicine(medicine_skill);
        endif
    endif
    
    // Verifica se o próprio subject está com incensário/bastão equipado
    var bonus_proprio := VerificarIncensarioProprio(subject);
    bonus_total += bonus_proprio;
    
    // Verifica se há incensário ou bastão equipado em pessoas próximas
    var bonus_incensario := VerificarIncensarioProximo(subject);
    bonus_total += bonus_incensario;
    SendSysMessageEx(subject, "Seu bônus é "+bonus_total); 
    return bonus_total;
endfunction