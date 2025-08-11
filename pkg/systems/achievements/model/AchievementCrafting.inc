include ":achievements:model/Achievement";

const ACHIEVEMENT_CRAFTING := ACHIEVEMENT + "_CRAFTING";

class AchievementCrafting(Achievement)
    function AchievementCrafting(this, key)
        Super(key);
        this.type := ACHIEVEMENT_CRAFTING;
    endfunction

    function Serialize(this)
        var base_data := Achievement::Serialize(this);

		return base_data;
    endfunction

    function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		Achievement::Deserialize(this, data);
    endfunction

    function toString(this)
        return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level})";
    endfunction
endclass

// TODO: Itens craftados
// TODO: Armas forjadas
	//Pensar se faz de cada item também
// TODO: Escudos forjados
	//Pensar se faz de cada item também
// TODO: Poções criadas
// TODO: Equipamentos reparados

enum ACHIEVDATA_CRAFTING
	ACHIEVDATA_CRAFTED_NORMAL := "craftednormal",
	ACHIEVDATA_CRAFTED_EXCEPTIONAL := "craftedexceptional",
	ACHIEVDATA_FORGED_WEAPONS := "forgedweapons",
	ACHIEVDATA_FORGED_SHIELDS := "forgedshields",
	ACHIEVDATA_CREATED_POTIONS := "createdpotions",
	ACHIEVDATA_REPAIRED_EQUIPMENT := "repairedequipment"
endenum