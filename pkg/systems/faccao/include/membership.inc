use guilds;

// ── Helpers de membership — include leve, sem dependências externas ───────────
// Usado por scripts que precisam checar membership sem incluir faccao.inc completo.

// Retorna 1 se o mobile é membro honorário desta facção.
function IsHonoraryMember(faccao, mobile)
   return (GetObjProperty(mobile, "honorary_guild_id") == faccao.guildid);
endfunction

// Retorna 1 se o mobile é membro full desta facção (via POL nativo).
function IsFullMember(faccao, mobile)
   return (mobile.guild && mobile.guildid == faccao.guildid);
endfunction

// Retorna 1 se o mobile é membro (full ou honorário) desta facção.
function IsMemberOfFaccao(faccao, mobile)
   return (IsFullMember(faccao, mobile) || IsHonoraryMember(faccao, mobile));
endfunction

// Retorna struct de privilégios do membro. Honorários recebem struct zerado.
function GetMemberPrivs(faccao, mobile)
   if (IsHonoraryMember(faccao, mobile))
      return struct{
         "above_guards" := 0, "above_merchants" := 0,
         "above_members" := 0, "giveprivs" := 0,
         "contractnpc" := 0,  "fixitems" := 0,
         "unlockdoors" := 0,  "buyuniforms" := 0,
         "buycapes" := 0
      };
   endif
   return faccao.GetProp(cstr(mobile.serial));
endfunction
