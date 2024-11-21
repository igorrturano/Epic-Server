// Este é um arquivo zumbi!
// Tá morto, só não pode remover ainda, se não vai quebrar o craft e outros scripts.
// Porém, nada aqui tem serventia mais...

use os;
use cfgfile;

function ReadResourceModifiers(mod)
    var cfg_file := ReadConfigFile(":crafting:config/resourceList");
    if (cfg_file.errortext)
        Syslog("Erro ao ler resourceList.cfg: " + cfg_file.errortext);
        return 0;
    endif

    var resource_modifiers := dictionary;
    foreach resource_id in GetConfigStringKeys(cfg_file)
        var elem := FindConfigElem(cfg_file, resource_id);
        if (elem)
            var aos_resist_mod;
            if(mod == "resist")
                aos_resist_mod := GetConfigStringArray(elem, "AoSResistMod");
            elseif(mod == "damage")
                aos_resist_mod := GetConfigStringArray(elem, "AoSDamageMod");
            endif

            var hp_mod := CInt(GetConfigString(elem, "hpmod")); // TODO: Remover daqui e substituir por uam função que calcule no item, baseado em um fórmula matemática envolvendo tier, peso, etc.
            if (aos_resist_mod.size() >= 5)
                resource_modifiers[Lower(resource_id)] := struct{
                    "Physical" := CInt(aos_resist_mod[1] * 100),
                    "Fire" := CInt(aos_resist_mod[2] * 100),
                    "Cold" := CInt(aos_resist_mod[3] * 100),
                    "Poison" := CInt(aos_resist_mod[4] * 100),
                    "Energy" := CInt(aos_resist_mod[5] * 100),
                    "HP" := hp_mod
                };
            endif
        endif
    endforeach

    return resource_modifiers;
endfunction

function TabelaMaterialResistMod(item)
    var material := Lower(Hex(item.GetProp("material")));
    var resource_modifiers := ReadResourceModifiers("resist");

    if (material in resource_modifiers)
        return resource_modifiers[material];
    endif

    // Valor padrão caso o material não seja encontrado
    return array{0, 0, 0, 0, 0, 0};
endfunction

function TabelaMaterialDamageMod(item)
    var material := Lower(Hex(item.GetProp("material")));
    var resource_modifiers := ReadResourceModifiers("damage");

    if (material in resource_modifiers)
        return resource_modifiers[material];
    endif

    return array{0, 0, 0, 0, 0, 0};
endfunction