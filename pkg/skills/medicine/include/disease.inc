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

function InfectMobile(subject,duration,attacker,level,diseases,disease_grade, deveIgnorarResistencia := false)

	if(disease_grade == "" || TypeOfInt(disease_grade) != OT_STRING)
		return error{"error_text" := "A gravidade é obrigatória."};
	endif

	//TODO verificar com o Frajolas se isto faz sentido.
	var dif := level * 8;
	var resistcheck := deveIgnorarResistencia ? false : CheckResist(subject, FORTITUDE, dif);

	if (resistcheck)
		SendSysMessageEx(subject, "Você resiste aos efeitos da doenca.", SSM_INFO);
	else
		SendSysMessageEx(subject, "Você não resiste aos efeitos da doenca.", SSM_FAIL);

		UpdateDiseaseProperty(subject, "diseases", diseases);
		UpdateDiseaseProperty(subject, "grade", disease_grade);
		var retornoTimer := TS_StartTimer(subject, "disease",duration,level, attacker); // ALTERAR TESTE

		if(!retornoTimer)
			return retornoTimer;
		endif

		var corpo_peste :=  GetEquipmentByLayer( subject, LAYER_PACK2 ); 

		if(!corpo_peste)
			var corpo_peste := CreateItemAtLocation(subject.x, subject.y, subject.z, 0xd1bc, 1);
			var isPesteEquipado := EquipItem(subject, corpo_peste);
			if(!isPesteEquipado)
				return  error{"error_text" := "Não foi possível criar aparência de pestilência."};
			endif
		endif

		UpdateDiseaseProperty(subject, "serial_peste_corpse", corpo_peste.serial);
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