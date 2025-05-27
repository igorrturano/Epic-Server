include ":achievements:model/Achievement";

const ACHIEVEMENT_CRAFTING := ACHIEVEMENT + "_CRAFTING";

class AchievementCrafting(Achievement)
    var craftedNormal;
    var craftedExceptional;

<<<<<<< HEAD:pkg/systems/achievements/model/Crafting.inc
    function Crafting(this, name)
=======
    function AchievementCrafting(this, name)
>>>>>>> 46062fc2b49c3141dbc0d4c15d368bc03f132839:pkg/systems/achievements/model/AchievementCrafting.inc
        Super(name);
        this.type := ACHIEVEMENT_CRAFTING;
        this.craftedNormal := 0;
        this.craftedExceptional := 0;
    endfunction

    function getCraftedNormal(this)
        return this.craftedNormal;
    endfunction

    function getCraftedExceptional(this)
        return this.craftedExceptional;
    endfunction

    function setCraftedNormal(this, value)
        this.craftedNormal := value;
    endfunction

    function setCraftedExceptional(this, value)
        this.craftedExceptional := value;
    endfunction

    function Serialize(this)
        var base_data := Achievement::Serialize(this);

		base_data.+craftedNormal := this.craftedNormal;
        base_data.+craftedExceptional := this.craftedExceptional;

		return base_data;
    endfunction

    function Deserialize(this, data)
		Achievement::Deserialize(this, data);
        this.craftedNormal := data.craftedNormal;
        this.craftedExceptional := data.craftedExceptional;
    endfunction

    function toString(this)
        return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, craftedNormal={this.craftedNormal}, craftedExceptional={this.craftedExceptional})";
    endfunction
endclass