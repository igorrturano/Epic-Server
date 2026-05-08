include ":abilities:models/CombatAbility";

/**
 * Aura de Fe (Aura of Faith) - ABILITY_ID::AURA_DE_FE (402)
 *
 * Active (OnUse):
 * - Party support ability with 4 commands via menu
 * - Banner detection increases range (2->6) and effectiveness (+50%)
 *
 * Commands:
 *   1. Formacao - Set #tempparrybonus on nearby party members
 *   2. Ate o fim - Heal bloodied party members (Theology/10)
 *   3. Pela Gloria - Apply "heroismo" timer (Theology/2 seconds)
 *   4. Protejam-se - Set "BarreiraDeEscudos" party property
 */
class AuraOfFaith(CombatAbility)

    function AuraOfFaith(this)
        Super(ABILITY_ID::AURA_DE_FE);
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
        Message("1 - Formacao!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("2 - Ate o fim!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("3 - Pela Gloria!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
        Message("4 - Protejam-se!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);

        var input := CInt(RequestInput(paladin, paladin.backpack, "Escolha (1-4):"));
        if (!(input >= 1 && input <= 4))
            Message("Comando invalido!", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
            return false;
        endif

        var theology := mobile.GetSkill(THEOLOGY);

        case (input)
            1: // Formacao
                Message("*ordena formacao*").SendAbove(paladin);
                foreach member in (festa.members)
                    if (member.serial != paladin.serial)
                        if (Distance(paladin, member) < range)
                            // Check if member maintains formation (within 2 tiles of another member)
                            var ok := 0;
                            foreach member2 in (festa.members)
                                if (Distance(member, member2) < 2)
                                    ok := 1;
                                    break;
                                endif
                            endforeach

                            if (!ok)
                                Message("Seus aliados nao mantiveram a formacao", Color(SYSTEM_COLOR::FAIL)).SendSys(paladin);
                                return false;
                            endif

                            var bonus := CInt(theology / 2);
                            var memberWrapper := MobileWrapper(member);
                            if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                                bonus := CInt(bonus * 1.5);
                            endif
                            memberWrapper.SetProperty("#tempparrybonus", bonus);
                        endif
                    endif
                endforeach

            2: // Ate o fim (Resilience)
                Message("*ordena resiliencia*").SendAbove(paladin);
                foreach member in (festa.members)
                    if (member.serial != paladin.serial)
                        if (Distance(paladin, member) < range)
                            if (isBloodied(member))
                                var memberWrapper := MobileWrapper(member);
                                var heal_amount := CInt(memberWrapper.GetCurrentHP() - memberWrapper.GetMaxHP() + theology / 10);
                                if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                                    heal_amount := CInt(heal_amount * 1.5);
                                endif
                                applyEpicHealing(member, heal_amount);
                            endif
                        endif
                    endif
                endforeach

            3: // Pela Gloria (Heroism)
                Message("*inspira seus aliados*").SendAbove(paladin);
                var duracao := CInt(theology / 2);

                foreach member in (festa.members)
                    if (member.serial != paladin.serial)
                        if (Distance(paladin, member) < range)
                            Message("Seus ataques estao poderosos!", Color(SYSTEM_COLOR::INFO)).SendSys(member);
                            var memberWrapper := MobileWrapper(member);
                            var memberDuracao := duracao;
                            if (has_banner && memberWrapper.GetReligion() == paladin_religion)
                                memberDuracao := CInt(duracao * 1.5);
                            endif
                            TS_StartTimer(member, "heroismo", memberDuracao);
                        endif
                    endif
                endforeach

            4: // Protejam-se (Shield Barrier)
                Message("Seu escudo e uma barreira!", Color(SYSTEM_COLOR::INFO)).SendSys(paladin);
                foreach member in (festa.members)
                    Message("Fique proximo do seu aliado com o escudo para se proteger!", Color(SYSTEM_COLOR::INFO)).SendSys(member);
                endforeach

                var barreira := struct;
                barreira.+blocker := paladin.serial;
                var chance := CInt(theology / 4);
                if (has_banner)
                    chance := CInt(chance * 1.5);
                endif
                barreira.+chance := chance;
                festa.setprop("BarreiraDeEscudos", barreira);
                SetCooldown(paladin, "BarreiraDeEscudos", 60);
        endcase

        return true;
    endfunction

endclass
