use cfgfile;
// use datafile;
// ============================================================================
// epicai.inc - external API surface for the epicAI package
// ============================================================================
// Minimal include after the SP2 split. Per-tick brain utilities moved to
// epicNPCCombat.inc; cold-init moved to epicNPCInitialization.inc.
// This file holds ONLY functions that external pkgs (spells, death hooks,
// abilities, GM tools) reference via include ":epicAI:epicai".
// ============================================================================


function ParseTemplateName(template_name)

  if ( template_name.IsA(POLCLASS_NPC) )
    template_name := template_name.npctemplate;
  endif

  //Find positions of ":" characters
  var colon_a := Find(template_name, ":", 1);
  var colon_b := Find(template_name, ":", 2);

  var parsed := struct{"package", "template"};

  if ( !colon_a || !colon_b )
    //Not a complete package name - use default npcdesc.cfg
    parsed.package := "";
    parsed.template := template_name;
  else
    parsed.package := template_name[colon_a+1, colon_b-2];
    parsed.template := template_name[colon_b+1, Len(template_name)];
  endif

  return parsed;
endfunction
