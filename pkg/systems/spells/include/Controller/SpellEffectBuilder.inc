use uo;

// Ephemeral authoring helpers for SC_ApplyEffect opts. Builder classes are
// constructed inside a SpellScript, chained, then flattened via ToOpts()
// into the plain struct that SC_ApplyEffect persists. The builders MUST NOT
// be persisted themselves — eScript class instances lose method dispatch
// across Start_Script / ObjProperty boundaries, so the persisted opts
// stays a flat struct by design (see docs/spells-pipeline.md).

// Presentation knobs played by scTickDriver each tick. All sub-fields
// optional; the driver checks each before firing.
class TickPresentation()
	var _fx;            // struct{ id, speed, frames } | unset
	var _sound := 0;
	var _periodic;      // struct{ text, every, color } | unset
	var _final_overhead := "";
	var _final_emote;   // struct{ text, color } | unset
	var _caster_feedback := 0;

	function TickPresentation(this)
	endfunction

	function SetFx(this, id, speed := 10, frames := 15)
		this._fx := struct{ id := id, speed := CInt(speed), frames := CInt(frames) };
		return this;
	endfunction

	function SetSound(this, sound_id)
		this._sound := CInt(sound_id);
		return this;
	endfunction

	function SetPeriodicEmote(this, text, every := 1, color := SSM_FAIL)
		this._periodic := struct{ text := text, every := CInt(every), color := color };
		return this;
	endfunction

	function SetFinalOverhead(this, text)
		this._final_overhead := text;
		return this;
	endfunction

	function SetFinalEmote(this, text, color := SSM_INFO)
		this._final_emote := struct{ text := text, color := color };
		return this;
	endfunction

	function SetCasterFeedback(this, value := 1)
		this._caster_feedback := CInt(value);
		return this;
	endfunction

	// Emit the flat struct that scTickDriver reads. Called by
	// SpellEffectBuilder.SetTickPresentation; spell scripts don't call it.
	function ToStruct(this)
		var s := struct{};
		if (this._fx)                  s.+fx := this._fx; endif
		if (this._sound)               s.+sound := this._sound; endif
		if (this._periodic)            s.+periodic_emote := this._periodic; endif
		if (this._final_overhead != "") s.+final_overhead := this._final_overhead; endif
		if (this._final_emote)         s.+final_emote := this._final_emote; endif
		if (this._caster_feedback)     s.+caster_feedback := this._caster_feedback; endif
		return s;
	endfunction
endclass

class SpellEffectBuilder()
	var _category := "";
	var _displayname := "";
	var _desc := "";
	var _tick_damage;
	var _tick_heal;
	var _tick_interval := 0;
	var _tick_presentation;
	var _tick_script := "";
	var _tick_params;
	var _maintainable := 0;
	var _on_apply_script := "";
	var _on_apply_params;
	var _on_expire_script := "";
	var _on_expire_params;
	var _properties;        // dictionary, ObjProperty escape hatch
	var _armor_hit_scripts; // array, for armor-hit triggers

	function SpellEffectBuilder(this)
	endfunction

	function SetCategory(this, name)
		this._category := name;
		return this;
	endfunction

	function SetDisplayName(this, name)
		this._displayname := name;
		return this;
	endfunction

	function SetDesc(this, text)
		this._desc := text;
		return this;
	endfunction

	// damage_struct: struct{ amount := <int>, type := DAMAGE_TYPE::XXX }
	function SetTickDamage(this, damage_struct)
		this._tick_damage := damage_struct;
		return this;
	endfunction

	// heal_struct: struct{ base := <int>, remainder := <int> }
	// Driver applies `base + (tick_number <= remainder ? 1 : 0)` per tick,
	// matching the per-tick rounding pattern druidheal used.
	function SetTickHeal(this, heal_struct)
		this._tick_heal := heal_struct;
		return this;
	endfunction

	function SetTickInterval(this, seconds)
		this._tick_interval := CInt(seconds);
		return this;
	endfunction

	// Accepts a TickPresentation instance or a raw struct.
	function SetTickPresentation(this, tp)
		if (tp is @TickPresentation)
			this._tick_presentation := tp.ToStruct();
		else
			this._tick_presentation := tp;
		endif
		return this;
	endfunction

	// Escape hatch for non-trivial ticks (santuario, manashield, etc.).
	function SetTickScript(this, script_path, params := 0)
		this._tick_script := script_path;
		this._tick_params := params;
		return this;
	endfunction

	function SetMaintainable(this, value := 1)
		this._maintainable := CInt(value);
		return this;
	endfunction

	function SetOnApply(this, script_path, params := 0)
		this._on_apply_script := script_path;
		this._on_apply_params := params;
		return this;
	endfunction

	function SetOnExpire(this, script_path, params := 0)
		this._on_expire_script := script_path;
		this._on_expire_params := params;
		return this;
	endfunction

	function SetProperties(this, dict)
		this._properties := dict;
		return this;
	endfunction

	function SetArmorHitScripts(this, arr)
		this._armor_hit_scripts := arr;
		return this;
	endfunction

	// Flatten to the plain struct SC_ApplyEffect persists. Only sets fields
	// that were actually configured so the resulting struct matches what a
	// hand-written `opts.+field := ...` chain would produce.
	function ToOpts(this)
		var opts := struct{};
		if (this._category != "")        opts.+buff_category := this._category; endif
		if (this._displayname != "")     opts.+displayname := this._displayname; endif
		if (this._desc != "")            opts.+desc := this._desc; endif
		if (this._tick_damage)           opts.+tick_damage := this._tick_damage; endif
		if (this._tick_heal)             opts.+tick_heal := this._tick_heal; endif
		if (this._tick_interval > 0)     opts.+tick_interval := this._tick_interval; endif
		if (this._tick_presentation)     opts.+tick_presentation := this._tick_presentation; endif
		if (this._tick_script != "")     opts.+tick_script := this._tick_script; endif
		if (this._tick_params)           opts.+tick_params := this._tick_params; endif
		if (this._maintainable)          opts.+maintainable := this._maintainable; endif
		if (this._on_apply_script != "") opts.+on_apply_script := this._on_apply_script; endif
		if (this._on_apply_params)       opts.+on_apply_params := this._on_apply_params; endif
		if (this._on_expire_script != "") opts.+on_expire_script := this._on_expire_script; endif
		if (this._on_expire_params)      opts.+on_expire_params := this._on_expire_params; endif
		if (this._properties)            opts.+properties := this._properties; endif
		if (this._armor_hit_scripts)     opts.+armor_hit_scripts := this._armor_hit_scripts; endif
		return opts;
	endfunction
endclass
