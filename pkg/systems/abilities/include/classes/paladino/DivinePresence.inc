include ":timedScripts:timedScripts";
include ":abilities:models/CombatAbility";

/**
 * Presenca Divina (Divine Presence) - ABILITY_ID::PRESENCA_DIVINA (403)
 *
 * Active (OnUse):
 * - Advanced party support ability with 4 commands via menu
 * - Banner detection increases range (2->6) and effectiveness (+50%)
 * - Immunity to fear (passive component via config)
 *
 * Commands:
 *   1. Cerquem! - If all party surrounds target: "heroismo" + "bencaodivina"
 *   2. Reviravolta! - If outnumbered: "heroismo" to nearby members
 *   3. Por Justica! - Random member gets guaranteed critical hit
 *   4. Vitoria Divina! - Cooldown reset (all if banner+religion, else <=180s)
 */
class DivinePresence(CombatAbility)

    function DivinePresence(this)
        Super(ABILITY_ID::PRESENCA_DIVINA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function CanActivate(this, mobile)
        if (!mobile.GetObjRef().party)
            Message("Voce nao tem aliados!", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
            return false;
        endif
        return true;
    endfunction

    function ExecuteOn(this, mobile)
        var paladin := mobile.GetObjRef();
        var festa := paladin.party;
        var paladin_religion := mobile.GetReligion();

        var has_banner := mobile.HasReligiousBannerEquipped();
        var range := has_banner ? 6 : 2;

        // Show menu
        Message("Escolha seu comando:", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("1 - Cerquem!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("2 - Reviravolta!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("3 - Por Justica!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("4 - Vitoria Divina!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);

        var input := CInt(RequestInput(paladin, paladin.backpack, "Escolha (1-4):"));
        if (!(input >= 1 && input <= 4))
            Message("Comando invalido!", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
            return false;
        endif

        var theology := mobile.GetSkill(THEOLOGY);

        case (input)
            1: // Cerquem! (Surround)
                Message("*ordena cerco*").SendAbove(paladin);
                Message("Escolha o alvo para seus aliados cercarem.", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);

                var alvo := Target(paladin);
                if (!alvo)
                    Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
                    return false;
                endif

                // Count members within 2 tiles of target
                var count := 1; // Paladin counts
                foreach member in (festa.members)
                    if (member.serial != paladin.serial)
                        if (Distance(paladin, member) < range)
                            if (Distance(member, alvo) < 2)
                                count := count + 1;
                            endif
                        endif
                    endif
                endforeach

                // All party members must surround target
                if (count == festa.members.size())
                    foreach member in (festa.members)
                        Message("Seus ataques estao poderosos!", Color(SYSTEM_COLOR::INFO)).SendSys(member);
                        TS_StartTimer(member, "heroismo", CInt(theology));
                        var memberWrapper := MobileWrapper(member);
                        if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                            TS_StartTimer(member, "bencaodivina", 10);
                        endif
                    endforeach
                endif

            2: // Reviravolta! (Turnaround)
                Message("*ordena reviravolta*").SendAbove(paladin);
                var inimigos := mobile.GetNearbyHostiles(range);

                if (len(inimigos) < CInt(festa.members.size()) / 2)
                    Message("Voce nao esta em desvantagem.", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
                    return false;
                endif

                foreach member in (festa.members)
                    if (Distance(paladin, member) < range)
                        var bonus := theology;
                        var memberWrapper := MobileWrapper(member);
                        if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                            bonus := CInt(bonus * 1.5);
                        endif
                        TS_StartTimer(member, "heroismo", CInt(bonus));
                    endif
                endforeach

            3: // Por Justica! (For Justice)
                Message("*guia aliado para gloria*").SendAbove(paladin);

                var candidates := array{};
                foreach member in (festa.members)
                    if (member.serial != paladin.serial)
                        if (Distance(paladin, member) < range)
                            candidates.append(member);
                        endif
                    endif
                endforeach

                if (candidates.size() < 1)
                    Message("Nenhum aliado proximo!", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
                    return false;
                endif

                var selected := candidates[RandomInt(candidates.size()) + 1];
                var memberWrapper := MobileWrapper(selected);
                memberWrapper.SetCriticalChanceMod(100);

                if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                    TS_StartTimer(selected, "bencaodivina", 15);
                else
                    TS_StartTimer(selected, "bencaodivina", 10);
                endif

            4: // Vitoria Divina! (Divine Victory - Cooldown reset)
                Message("*proclama vitoria divina*").SendAbove(paladin);

                foreach member in (festa.members)
                    if (Distance(paladin, member) < range)
                        var cds := TS_GetCooldownList(member);
                        var memberWrapper := MobileWrapper(member);

                        foreach cd in cds
                            if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                                TS_LowerDuration(member, cd, -1, 1);
                            elseif (cd.duration <= 180)
                                TS_LowerDuration(member, cd, -1, 1);
                            endif
                        endforeach
                    endif
                endforeach
        endcase

        return true;
    endfunction

endclass
