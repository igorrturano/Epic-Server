# Epic Client UX — Scaffold + Handshake + Stats + Hotbar

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stand up `Game.Core` and `Epic.Content` projects, implement the Razor handshake, replace the vanilla `0x11` stats packet with Epic's custom layout, and send stub hotbar responses — so epicuo connects without visual errors and the stats window + hotbar look correct from day one.

**Architecture:** `Game.Core` (pure C#, zero engine refs, compiler-enforced) + `Epic.Content` (our UO adapter, references `Game.Core` + `Server` + `UOContent`). `EpicPlayer : PlayerMobile` is the custom player mobile. Outgoing packets use `SpanWriter` / `ns.Send(span)`. Incoming packets registered via `IncomingPackets.Register` with unsafe function pointers. Stats packet override via `OnNetStateChanged` + `Delta` on `EpicPlayer`.

**Tech Stack:** .NET 10, C# 14, ModernUO (Server + UOContent), xUnit 2.x

## Global Constraints

- `Game.Core.csproj` must have zero `<ProjectReference>` to `Server` or `Epic.Content`. Compilation fails if this is violated.
- All identifiers in English. Zero Portuguese hardcoded strings in C# source.
- Packet byte layout for `0x11` must match `NEW_STATUS_OFFSETS` in `pkg/packetHooks/StatHook/sendStats.src` exactly — epicuo's client parser expects this layout.
- Hotbar subcommand values come from `pkg/packetHooks/hotbar/include/shared_constants.inc`.
- ModernUO is already running locally at `/home/igorrturano/epic/modern-epic/modernuo`.
- `IncomingPackets.Register` uses unsafe function pointers (`delegate*<NetState, SpanReader, void>`). Handler methods must be `static`.
- **Every failure message must include actionable guidance.** No message may state only what went wrong — it must also say what the player can do next. This is enforced by `ActionFeedback(string Failure, string Guidance)` in `Game.Core/Feedback/Models/`. Create this type in Task 0. Any code path that emits feedback with empty `Guidance` throws at runtime. See migration spec §10 for rationale and examples.

---

## File Map

### New projects
| File | Responsibility |
|---|---|
| `Projects/Game.Core/Game.Core.csproj` | Pure rules project — no engine refs |
| `Projects/Game.Core.Tests/Game.Core.Tests.csproj` | xUnit tests for Game.Core |
| `Projects/Epic.Content/Epic.Content.csproj` | Adapter layer — references Game.Core + Server + UOContent |

### Game.Core
| File | Responsibility |
|---|---|
| `Projects/Game.Core/Attributes/Models/AttributeSnapshot.cs` | Value type: every field the 0x11 packet reads |
| `Projects/Game.Core/Identity/Models/IdentitySnapshot.cs` | Name + disguise state |
| `Projects/Game.Core/Identity/Rules/IdentityRules.cs` | `DisplayName()` — Desconhecido logic |
| `Projects/Game.Core/Localization/Models/Language.cs` | `Language` + `TranslationMode` enums |

### Epic.Content
| File | Responsibility |
|---|---|
| `Projects/Epic.Content/Mobiles/EpicPlayer.cs` | `PlayerMobile` subclass with all serialized Epic fields |
| `Projects/Epic.Content/Adapters/MobileAdapter.cs` | `EpicPlayer` → `AttributeSnapshot` / `IdentitySnapshot` |
| `Projects/Epic.Content/Network/EpicPacketRegistry.cs` | Registers all incoming custom packet handlers at startup |
| `Projects/Epic.Content/Network/Packets/Handshake/RazorHandshakeHandler.cs` | Handles incoming `0xF0` |
| `Projects/Epic.Content/Network/Packets/Stats/ExtendedStatsPacket.cs` | Builds + sends our custom `0x11` (78 bytes) |
| `Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs` | Routes incoming `0xD7` subcommands |
| `Projects/Epic.Content/Network/Packets/Hotbar/HotbarStubResponses.cs` | Sends empty-but-valid `0x53`–`0x59` |
| `Projects/Epic.Content/Network/Packets/Hotbar/TooltipHandler.cs` | `0x60` → empty `0x61` response |
| `Projects/Epic.Content/Network/Packets/Hotbar/MinorPackets.cs` | Stubs for `0x62`, `0x63`, `0x5A`, `0x0070` |

---

## Task 0: Project Scaffold

**Files:**
- Create: `Projects/Game.Core/Game.Core.csproj`
- Create: `Projects/Game.Core.Tests/Game.Core.Tests.csproj`
- Create: `Projects/Epic.Content/Epic.Content.csproj`
- Modify: `ModernUO.slnx`

- [ ] **Step 1: Create `Game.Core.csproj`**

```xml
<!-- Projects/Game.Core/Game.Core.csproj -->
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
    <LangVersion>14</LangVersion>
    <AllowUnsafeBlocks>false</AllowUnsafeBlocks>
  </PropertyGroup>
  <!-- Intentionally NO ProjectReference to Server or Epic.Content -->
</Project>
```

- [ ] **Step 2: Create `Game.Core.Tests.csproj`**

```xml
<!-- Projects/Game.Core.Tests/Game.Core.Tests.csproj -->
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
    <LangVersion>14</LangVersion>
    <IsPackable>false</IsPackable>
  </PropertyGroup>
  <ItemGroup>
    <PackageReference Include="xunit" Version="2.*" />
    <PackageReference Include="xunit.runner.visualstudio" Version="2.*" />
    <PackageReference Include="Microsoft.NET.Test.Sdk" Version="*" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="../Game.Core/Game.Core.csproj" />
  </ItemGroup>
</Project>
```

- [ ] **Step 3: Create `Epic.Content.csproj`**

```xml
<!-- Projects/Epic.Content/Epic.Content.csproj -->
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
    <LangVersion>14</LangVersion>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
  </PropertyGroup>
  <ItemGroup>
    <ProjectReference Include="../Game.Core/Game.Core.csproj" />
    <ProjectReference Include="../Server/Server.csproj" />
    <ProjectReference Include="../UOContent/UOContent.csproj" />
  </ItemGroup>
</Project>
```

- [ ] **Step 4: Add projects to solution**

Open `ModernUO.slnx`. Copy the pattern of an existing `<Project>` entry and add entries for the three new projects.

- [ ] **Step 5: Create folder skeleton**

```bash
cd /home/igorrturano/epic/modern-epic/modernuo

mkdir -p Projects/Game.Core/Attributes/Models
mkdir -p Projects/Game.Core/Identity/{Models,Rules,Interfaces}
mkdir -p Projects/Game.Core/Localization/Models
mkdir -p Projects/Game.Core/Shared/Enums

mkdir -p Projects/Game.Core.Tests/Attributes
mkdir -p Projects/Game.Core.Tests/Identity

mkdir -p Projects/Epic.Content/Mobiles
mkdir -p Projects/Epic.Content/Adapters
mkdir -p Projects/Epic.Content/Network/Packets/{Handshake,Stats,Hotbar,Combat,Buffs,Equipment,Fog,Experience}
```

- [ ] **Step 6: Verify clean build**

```bash
cd /home/igorrturano/epic/modern-epic/modernuo
dotnet build ModernUO.slnx
```

Expected: `Build succeeded. 0 Error(s)` — empty projects compile fine.

- [ ] **Step 7: Commit**

```bash
git add Projects/Game.Core Projects/Game.Core.Tests Projects/Epic.Content ModernUO.slnx
git commit -m "feat: scaffold Game.Core, Game.Core.Tests, Epic.Content projects"
```

---

## Task 1: EpicPlayer Base

**Files:**
- Create: `Projects/Epic.Content/Mobiles/EpicPlayer.cs`

**Interfaces:**
- Produces: `EpicPlayer` with properties `EpicLevel`, `DeathPointsCurrent`, `DeathPointsMax`, `HungerValue`, `Fortitude`, `Reflexes`, `WillStat`, `AddictionStr`, `AddictionDex`, `AddictionInt`, `IsIncognito`, `IncognitoName` — consumed by Tasks 4 and 5.

- [ ] **Step 1: Create EpicPlayer.cs**

```csharp
// Projects/Epic.Content/Mobiles/EpicPlayer.cs
using Server;
using Server.Mobiles;
using Server.Network;
using Epic.Content.Network.Packets.Stats;

namespace Epic.Content.Mobiles;

[SerializationGenerator(0)]
public partial class EpicPlayer : PlayerMobile
{
    [SerializableField(0)] private int _epicLevel;
    [SerializableField(1)] private int _deathPointsCurrent;
    [SerializableField(2)] private int _deathPointsMax;
    [SerializableField(3)] private int _hungerValue;
    [SerializableField(4)] private int _fortitude;
    [SerializableField(5)] private int _reflexes;
    [SerializableField(6)] private int _willStat;
    [SerializableField(7)] private int _addictionStr;
    [SerializableField(8)] private int _addictionDex;
    [SerializableField(9)] private int _addictionInt;
    [SerializableField(10)] private bool _isIncognito;
    [SerializableField(11)] private string? _incognitoName;
    [SerializableField(12)] private int _epicLanguage;   // Language enum value
    [SerializableField(13)] private int _translationMode; // 0=Off 1=Subtitle 2=Dub

    public EpicPlayer(Serial serial) : base(serial) { }

    public EpicPlayer() : base()
    {
        _epicLevel = 1;
        _deathPointsMax = 3;
        _deathPointsCurrent = 3;
    }

    public override void OnNetStateChanged()
    {
        base.OnNetStateChanged();
        if (NetState != null)
        {
            ExtendedStatsPacket.Send(NetState, this, this);
        }
    }

    public override void Delta(MobileDelta flag)
    {
        base.Delta(flag);

        // After base sends the vanilla 0x11, send ours so the client
        // overwrites it with the Epic layout.
        const MobileDelta statsFlags =
            MobileDelta.Stat | MobileDelta.Hits | MobileDelta.Mana |
            MobileDelta.Stam | MobileDelta.Weight | MobileDelta.Gold;

        if ((flag & statsFlags) != 0 && NetState != null)
        {
            ExtendedStatsPacket.Send(NetState, this, this);
        }
    }
}
```

- [ ] **Step 2: Find the player mobile registration point**

```bash
grep -rn "PlayerMobile\|CreateMobile\|CharacterCreat" \
  /home/igorrturano/epic/modern-epic/modernuo/Projects/UOContent/Accounting \
  --include="*.cs" | head -20
```

Find where ModernUO instantiates the player mobile and replace `new PlayerMobile(serial)` with `new EpicPlayer(serial)`. This is typically in an account or character handler.

- [ ] **Step 3: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: Error about `ExtendedStatsPacket` not existing — that's fine, it comes in Task 5. Add a temporary stub at the bottom of `EpicPlayer.cs`:

```csharp
// TEMP — remove in Task 5
namespace Epic.Content.Network.Packets.Stats
{
    internal static class ExtendedStatsPacket
    {
        public static void Send(Server.Network.NetState ns, EpicPlayer for_, Server.Mobile target) { }
    }
}
```

- [ ] **Step 4: Build again — expect success**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 5: Commit**

```bash
git add Projects/Epic.Content/Mobiles/EpicPlayer.cs
git commit -m "feat: EpicPlayer skeleton — serialized Epic fields + Delta hook stubs"
```

---

## Task 2: Razor Handshake

**Files:**
- Create: `Projects/Epic.Content/Network/EpicPacketRegistry.cs`
- Create: `Projects/Epic.Content/Network/Packets/Handshake/RazorHandshakeHandler.cs`

**Interfaces:**
- Produces: `EpicPacketRegistry.Register()` — called once at server startup; wires all custom incoming handlers.

- [ ] **Step 1: Create EpicPacketRegistry.cs**

```csharp
// Projects/Epic.Content/Network/EpicPacketRegistry.cs
using Server.Network;
using Epic.Content.Network.Packets.Handshake;
using Epic.Content.Network.Packets.Hotbar;

namespace Epic.Content.Network;

public static class EpicPacketRegistry
{
    public static unsafe void Register()
    {
        // 0xF0 — Razor/EpicUO handshake. Variable length, allowed before login.
        IncomingPackets.Register(0xF0, &RazorHandshakeHandler.Handle, length: 0, ingameOnly: false);

        // 0xD7 — AOS extended commands (our custom subcommand space).
        // If ModernUO already registers 0xD7 for vanilla subcommands, check
        // IncomingPackets.Handlers[0xD7] before overwriting — extend instead.
        IncomingPackets.Register(0xD7, &HotbarRouter.Handle, length: 0, ingameOnly: true);
    }
}
```

- [ ] **Step 2: Find the startup hook and call Register()**

```bash
grep -rn "ServerStarted\|Configure\|\[Initialize\]\|EventSink" \
  /home/igorrturano/epic/modern-epic/modernuo/Projects/UOContent \
  --include="*.cs" | grep -v "obj/" | head -20
```

Add the call in whichever startup hook ModernUO uses. Common pattern:

```csharp
// In startup class / EventSink.ServerStarted handler:
EpicPacketRegistry.Register();
```

- [ ] **Step 3: Create RazorHandshakeHandler.cs**

Response bytes from `pkg/packetHooks/razor/handshake.src`: `SendPacket(who, "F0000CFE0000000000030003")`.

```csharp
// Projects/Epic.Content/Network/Packets/Handshake/RazorHandshakeHandler.cs
using System;
using Server;
using Server.Network;

namespace Epic.Content.Network.Packets.Handshake;

public static class RazorHandshakeHandler
{
    // F0 00 0C FE 00 00 00 00 00 03 00 03
    private static ReadOnlySpan<byte> HandshakeResponse =>
        [0xF0, 0x00, 0x0C, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03];

    public static void Handle(NetState state, SpanReader reader)
    {
        Utility.WriteLine(ConsoleColor.Cyan, $"[EpicUO] Handshake/HWID from {state}");

        // Short packets are HWID — just acknowledge, no response.
        if (reader.Remaining < 10)
            return;

        state.Send(HandshakeResponse);
    }
}
```

- [ ] **Step 4: Add temporary HotbarRouter stub so EpicPacketRegistry compiles**

```csharp
// Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs (TEMP stub)
using Server.Network;

namespace Epic.Content.Network.Packets.Hotbar;

public static class HotbarRouter
{
    public static void Handle(NetState state, SpanReader reader) { }
}
```

- [ ] **Step 5: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 6: E2E — connect epicuo**

Start ModernUO. Connect with epicuo. Server console must show:

```
[EpicUO] Handshake/HWID from <ip>
```

No disconnect on handshake = pass.

- [ ] **Step 7: Commit**

```bash
git add Projects/Epic.Content/Network/
git commit -m "feat: Razor handshake (0xF0) + EpicPacketRegistry"
```

---

## Task 3: AttributeSnapshot + IdentityRules (Game.Core)

**Files:**
- Create: `Projects/Game.Core/Localization/Models/Language.cs`
- Create: `Projects/Game.Core/Attributes/Models/AttributeSnapshot.cs`
- Create: `Projects/Game.Core/Identity/Models/IdentitySnapshot.cs`
- Create: `Projects/Game.Core/Identity/Rules/IdentityRules.cs`
- Test: `Projects/Game.Core.Tests/Attributes/AttributeSnapshotTests.cs`
- Test: `Projects/Game.Core.Tests/Identity/IdentityRulesTests.cs`

**Interfaces:**
- Produces: `AttributeSnapshot` record struct — consumed by Tasks 4 + 5.
- Produces: `IdentityRules.DisplayName(IdentitySnapshot, bool)` → `string` — consumed by Task 5.

- [ ] **Step 1: Create Language.cs**

```csharp
// Projects/Game.Core/Localization/Models/Language.cs
namespace Game.Core.Localization.Models;

public enum Language { Portuguese = 0, English = 1, Spanish = 2 }
public enum TranslationMode { Off = 0, Subtitle = 1, Dub = 2 }
```

- [ ] **Step 2: Create AttributeSnapshot.cs**

Field names map 1:1 to `NEW_STATUS_OFFSETS` in `pkg/packetHooks/StatHook/sendStats.src`.

```csharp
// Projects/Game.Core/Attributes/Models/AttributeSnapshot.cs
namespace Game.Core.Attributes.Models;

public readonly record struct AttributeSnapshot(
    string Name,
    int Hits,        int HitsMax,
    int Strength,    int Dexterity,   int Intelligence,
    int Stamina,     int StaminaMax,
    int Mana,        int ManaMax,
    int Weight,      int WeightMax,
    int Followers,   int FollowersMax,
    int Fortitude,   int Reflexes,    int Will,
    int DeathPoints, int DeathPointsMax,
    int AddictionStr, int AddictionDex, int AddictionInt,
    int Hunger,
    int Level,
    int Gold
);
```

- [ ] **Step 3: Write test for AttributeSnapshot**

```csharp
// Projects/Game.Core.Tests/Attributes/AttributeSnapshotTests.cs
using Game.Core.Attributes.Models;
using Xunit;

public class AttributeSnapshotTests
{
    [Fact]
    public void AttributeSnapshot_StoresAllFields()
    {
        var snap = new AttributeSnapshot(
            Name: "Kira",
            Hits: 80, HitsMax: 100,
            Strength: 65, Dexterity: 55, Intelligence: 40,
            Stamina: 55, StaminaMax: 55,
            Mana: 40, ManaMax: 40,
            Weight: 10, WeightMax: 200,
            Followers: 0, FollowersMax: 5,
            Fortitude: 5, Reflexes: 4, Will: 3,
            DeathPoints: 3, DeathPointsMax: 3,
            AddictionStr: 0, AddictionDex: 0, AddictionInt: 0,
            Hunger: 0, Level: 1, Gold: 0
        );

        Assert.Equal("Kira", snap.Name);
        Assert.Equal(65, snap.Strength);
        Assert.Equal(3, snap.DeathPointsMax);
        Assert.Equal(1, snap.Level);
    }
}
```

- [ ] **Step 4: Run — expect pass (record struct, no logic to implement)**

```bash
cd /home/igorrturano/epic/modern-epic/modernuo
dotnet test Projects/Game.Core.Tests/Game.Core.Tests.csproj -v normal
```

Expected: `1 passed`

- [ ] **Step 5: Create IdentitySnapshot.cs**

```csharp
// Projects/Game.Core/Identity/Models/IdentitySnapshot.cs
namespace Game.Core.Identity.Models;

public readonly record struct IdentitySnapshot(
    string RealName,
    bool IsIncognito,
    string? IncognitoName
);
```

- [ ] **Step 6: Write failing test for IdentityRules**

```csharp
// Projects/Game.Core.Tests/Identity/IdentityRulesTests.cs
using Game.Core.Identity.Models;
using Game.Core.Identity.Rules;
using Xunit;

public class IdentityRulesTests
{
    [Fact]
    public void DisplayName_NotIncognito_ReturnsRealName()
    {
        var snap = new IdentitySnapshot("Aria", IsIncognito: false, IncognitoName: null);
        Assert.Equal("Aria", IdentityRules.DisplayName(snap, viewerKnowsTrue: false));
    }

    [Fact]
    public void DisplayName_Incognito_NoCustomName_ReturnsDesconhecido()
    {
        var snap = new IdentitySnapshot("Aria", IsIncognito: true, IncognitoName: null);
        Assert.Equal("Desconhecido", IdentityRules.DisplayName(snap, viewerKnowsTrue: false));
    }

    [Fact]
    public void DisplayName_Incognito_WithCustomName_ReturnsCustomName()
    {
        var snap = new IdentitySnapshot("Aria", IsIncognito: true, IncognitoName: "Stranger");
        Assert.Equal("Stranger", IdentityRules.DisplayName(snap, viewerKnowsTrue: false));
    }

    [Fact]
    public void DisplayName_Incognito_ViewerKnowsTrue_ReturnsRealName()
    {
        var snap = new IdentitySnapshot("Aria", IsIncognito: true, IncognitoName: null);
        Assert.Equal("Aria", IdentityRules.DisplayName(snap, viewerKnowsTrue: true));
    }
}
```

- [ ] **Step 7: Run — expect compile error (IdentityRules missing)**

```bash
dotnet test Projects/Game.Core.Tests/Game.Core.Tests.csproj -v normal
```

Expected: `error CS0246: The type or namespace name 'IdentityRules' could not be found`

- [ ] **Step 8: Create IdentityRules.cs**

```csharp
// Projects/Game.Core/Identity/Rules/IdentityRules.cs
using Game.Core.Identity.Models;

namespace Game.Core.Identity.Rules;

public static class IdentityRules
{
    public static string DisplayName(IdentitySnapshot identity, bool viewerKnowsTrue)
    {
        if (!identity.IsIncognito || viewerKnowsTrue)
            return identity.RealName;

        return identity.IncognitoName ?? "Desconhecido";
    }
}
```

- [ ] **Step 9: Run — expect all pass**

```bash
dotnet test Projects/Game.Core.Tests/Game.Core.Tests.csproj -v normal
```

Expected: `5 passed`

- [ ] **Step 10: Commit**

```bash
git add Projects/Game.Core/ Projects/Game.Core.Tests/
git commit -m "feat: Game.Core — AttributeSnapshot, IdentitySnapshot, IdentityRules"
```

---

## Task 4: MobileAdapter

**Files:**
- Create: `Projects/Epic.Content/Adapters/MobileAdapter.cs`

**Interfaces:**
- Consumes: `EpicPlayer` (Task 1), `AttributeSnapshot` + `IdentitySnapshot` (Task 3)
- Produces: `MobileAdapter.ToAttributeSnapshot(EpicPlayer)` → `AttributeSnapshot` — consumed by Task 5.
- Produces: `MobileAdapter.ToIdentitySnapshot(EpicPlayer)` → `IdentitySnapshot` — consumed by Task 5.

- [ ] **Step 1: Create MobileAdapter.cs**

If any property name below doesn't compile, check the exact name in `Projects/Server/Mobiles/Mobile.cs` and `Projects/UOContent/Mobiles/PlayerMobile.cs`.

```csharp
// Projects/Epic.Content/Adapters/MobileAdapter.cs
using Epic.Content.Mobiles;
using Game.Core.Attributes.Models;
using Game.Core.Identity.Models;

namespace Epic.Content.Adapters;

public static class MobileAdapter
{
    public static AttributeSnapshot ToAttributeSnapshot(EpicPlayer p) => new(
        Name: p.Name ?? "",
        Hits: p.Hits,            HitsMax: p.HitsMax,
        Strength: p.Str,         Dexterity: p.Dex,      Intelligence: p.Int,
        Stamina: p.Stam,         StaminaMax: p.StamMax,
        Mana: p.Mana,            ManaMax: p.ManaMax,
        Weight: (int)p.TotalWeight, WeightMax: p.MaxWeight,
        Followers: p.Followers,  FollowersMax: p.FollowersMax,
        Fortitude: p.Fortitude,  Reflexes: p.Reflexes,  Will: p.WillStat,
        DeathPoints: p.DeathPointsCurrent, DeathPointsMax: p.DeathPointsMax,
        AddictionStr: p.AddictionStr, AddictionDex: p.AddictionDex, AddictionInt: p.AddictionInt,
        Hunger: p.HungerValue,
        Level: p.EpicLevel,
        Gold: p.TotalGold
    );

    public static IdentitySnapshot ToIdentitySnapshot(EpicPlayer p) => new(
        RealName: p.RawName ?? p.Name ?? "",
        IsIncognito: p.IsIncognito,
        IncognitoName: p.IncognitoName
    );
}
```

- [ ] **Step 2: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.` Fix any property-name mismatches by checking the actual `Mobile.cs` + `PlayerMobile.cs` property names.

- [ ] **Step 3: Commit**

```bash
git add Projects/Epic.Content/Adapters/MobileAdapter.cs
git commit -m "feat: MobileAdapter — EpicPlayer → AttributeSnapshot/IdentitySnapshot"
```

---

## Task 5: Extended Stats Packet (0x11 override)

**Files:**
- Create: `Projects/Epic.Content/Network/Packets/Stats/ExtendedStatsPacket.cs`
- Modify: `Projects/Epic.Content/Mobiles/EpicPlayer.cs` — remove the temporary stub

**Interfaces:**
- Consumes: `MobileAdapter.ToAttributeSnapshot()` + `MobileAdapter.ToIdentitySnapshot()` (Task 4), `IdentityRules.DisplayName()` (Task 3)
- Produces: Sends a valid 78-byte `0x11` packet matching `NEW_STATUS_OFFSETS` in `sendStats.src`

**Before starting:** Verify the epicuo client's 0x11 handler:
```bash
grep -n "0x11\|StatusBar\|MobileStatus\|\"0x11\"\|case 0x11" \
  /home/igorrturano/epic/epicuo/src/ClassicUO.Client/Network/PacketHandlers.cs | head -20
```

Confirm it uses the Epic custom layout (78 bytes) and not vanilla layout. If the handler reads the packet length first and branches, note which branch handles our 78-byte packet.

- [ ] **Step 1: Create ExtendedStatsPacket.cs**

Byte layout from `NEW_STATUS_OFFSETS` in `pkg/packetHooks/StatHook/sendStats.src`:

```csharp
// Projects/Epic.Content/Network/Packets/Stats/ExtendedStatsPacket.cs
using System;
using System.Buffers.Binary;
using Epic.Content.Adapters;
using Epic.Content.Mobiles;
using Game.Core.Identity.Rules;
using Server;
using Server.Network;

namespace Epic.Content.Network.Packets.Stats;

public static class ExtendedStatsPacket
{
    // 78 = last field (Gold at offset 74, 4 bytes) = 74 + 4
    private const int PacketSize = 78;

    public static void Send(NetState ns, EpicPlayer forPlayer, Mobile target)
    {
        if (ns.CannotSendPackets()) return;

        var snap     = MobileAdapter.ToAttributeSnapshot(forPlayer);
        var identity = MobileAdapter.ToIdentitySnapshot(forPlayer);
        bool viewerIsTarget = forPlayer.Serial == target.Serial;
        var displayName = IdentityRules.DisplayName(identity, viewerKnowsTrue: viewerIsTarget);

        Span<byte> packet = stackalloc byte[PacketSize];
        packet.Clear();

        // 0x11 header
        packet[0] = 0x11;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], (ushort)PacketSize);

        // Serial (offset 3, 4 bytes)
        BinaryPrimitives.WriteInt32BigEndian(packet[3..], target.Serial.Value);

        // Name (offset 7, 30 bytes, null-terminated ASCII)
        WriteAsciiFixed(packet[7..], displayName, 30);

        // Hits (offset 37, 38 — each 2 bytes)
        BinaryPrimitives.WriteUInt16BigEndian(packet[37..], (ushort)snap.Hits);
        BinaryPrimitives.WriteUInt16BigEndian(packet[39..], (ushort)snap.HitsMax);

        // IsRenamable (offset 41, 1 byte) — always 0 for Epic
        packet[41] = 0;

        // STR/DEX/INT (offsets 42/44/46, 2 bytes each)
        BinaryPrimitives.WriteInt16BigEndian(packet[42..], (short)snap.Strength);
        BinaryPrimitives.WriteInt16BigEndian(packet[44..], (short)snap.Dexterity);
        BinaryPrimitives.WriteInt16BigEndian(packet[46..], (short)snap.Intelligence);

        // Stamina (offset 48/50)
        BinaryPrimitives.WriteInt16BigEndian(packet[48..], (short)snap.Stamina);
        BinaryPrimitives.WriteInt16BigEndian(packet[50..], (short)snap.StaminaMax);

        // Mana (offset 52/54)
        BinaryPrimitives.WriteInt16BigEndian(packet[52..], (short)snap.Mana);
        BinaryPrimitives.WriteInt16BigEndian(packet[54..], (short)snap.ManaMax);

        // Weight (offset 56/58)
        BinaryPrimitives.WriteInt16BigEndian(packet[56..], (short)snap.Weight);
        BinaryPrimitives.WriteInt16BigEndian(packet[58..], (short)snap.WeightMax);

        // Followers (offset 60/61, 1 byte each)
        packet[60] = (byte)snap.Followers;
        packet[61] = (byte)snap.FollowersMax;

        // Intrinsics (offset 62/63/64, 1 byte each)
        packet[62] = (byte)snap.Fortitude;
        packet[63] = (byte)snap.Reflexes;
        packet[64] = (byte)snap.Will;

        // Death Points (offset 65/67, 2 bytes each)
        BinaryPrimitives.WriteInt16BigEndian(packet[65..], (short)snap.DeathPoints);
        BinaryPrimitives.WriteInt16BigEndian(packet[67..], (short)snap.DeathPointsMax);

        // Addiction (offset 69/70/71, 1 byte each)
        packet[69] = (byte)snap.AddictionStr;
        packet[70] = (byte)snap.AddictionDex;
        packet[71] = (byte)snap.AddictionInt;

        // Hunger (offset 72), Level (offset 73)
        packet[72] = (byte)snap.Hunger;
        packet[73] = (byte)snap.Level;

        // Gold (offset 74, 4 bytes)
        BinaryPrimitives.WriteInt32BigEndian(packet[74..], snap.Gold);

        ns.Send(packet);
    }

    private static void WriteAsciiFixed(Span<byte> dest, string text, int maxLen)
    {
        int written = 0;
        foreach (char c in text)
        {
            if (written >= maxLen - 1) break;
            dest[written++] = c < 128 ? (byte)c : (byte)'?';
        }
        // null-terminate
        if (written < maxLen) dest[written] = 0;
    }
}
```

- [ ] **Step 2: Remove the temporary stub from EpicPlayer.cs**

Delete the `namespace Epic.Content.Network.Packets.Stats { internal static class ExtendedStatsPacket ... }` block added in Task 1 Step 3.

- [ ] **Step 3: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 4: E2E — verify stats window**

Start ModernUO. Connect epicuo. Open the paperdoll (click character portrait).

Expected:
- HP, STR/DEX/INT, Stamina, Mana show correct values
- Death Points field visible (should show `3 / 3` for a new character)
- Level shows `1`
- Name shows correctly (not "Desconhecido" since we're viewing our own paperdoll)

**If stats flash between two values:** The vanilla 0x11 and our 0x11 are both arriving. The vanilla one has wrong values at Epic-specific offsets. In epicuo's `PacketHandlers.cs`, check whether `0x11` is handled by the vanilla parser or a custom one. If vanilla, the double-send will corrupt display. Fix: in `EpicPlayer.Delta()`, suppress the base stats send by overriding more selectively — remove the `base.Delta(flag)` call for stats-only flags, handle non-stats flags via base normally. Check `Projects/UOContent/Mobiles/PlayerMobile.cs` line 1515 for Delta override as a reference.

**If Death Points / Level fields are blank or show garbage:** The byte offset is wrong. Compare your `packet[65]` write against `NEW_STATUS_OFFSETS::DEATH_POINTS := 65` in `sendStats.src`. They must match exactly.

- [ ] **Step 5: Commit**

```bash
git add Projects/Epic.Content/Network/Packets/Stats/ Projects/Epic.Content/Mobiles/EpicPlayer.cs
git commit -m "feat: ExtendedStatsPacket — custom 0x11 with Death Points, Level, intrinsics"
```

---

## Task 6: Incoming 0xD7 Router

**Files:**
- Modify: `Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs` — replace stub with real router

**Interfaces:**
- Produces: `HotbarRouter.Handle(NetState, SpanReader)` — routes `0x50`, `0x51`, `0x60` to handlers; stubs defined in Task 7.

- [ ] **Step 1: Replace HotbarRouter stub**

The `0xD7` layout: `[0xD7][length:2][serial:4][subcommand:2][...]`. `SpanReader` is already positioned after the packet ID byte. Read: 2 bytes length, 4 bytes serial, 2 bytes subcommand.

Subcommand values from `pkg/packetHooks/hotbar/include/shared_constants.inc`:
- `0x50` = SUBCOMMAND_REQUEST
- `0x51` = SUBCOMMAND_EXECUTE
- `0x60` = SUBCOMMAND_TOOLTIP_REQUEST

```csharp
// Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs
using Epic.Content.Mobiles;
using Server;
using Server.Network;

namespace Epic.Content.Network.Packets.Hotbar;

public static class HotbarRouter
{
    public static void Handle(NetState state, SpanReader reader)
    {
        if (state.Mobile is not EpicPlayer player) return;

        // 0xD7 layout after packet ID: [length:2][serial:4][subcommand:2][...]
        reader.ReadUInt16();           // length — already known from packet framing
        reader.ReadUInt32();           // serial
        var subcommand = reader.ReadUInt16();

        switch (subcommand)
        {
            case 0x50:
                HotbarStubResponses.SendAll(state, player);
                break;
            case 0x51:
                LogExecute(player, reader);
                break;
            case 0x60:
                TooltipHandler.Handle(state, player, reader);
                break;
            default:
                Utility.WriteLine(ConsoleColor.Yellow,
                    $"[EpicUO] Unknown 0xD7 sub=0x{subcommand:X4} from {player.Name}");
                break;
        }
    }

    private static void LogExecute(EpicPlayer player, SpanReader reader)
    {
        // Stub — full dispatch in the combat/abilities plan
        var actionId   = reader.ReadUInt16();
        var actionType = reader.ReadByte();
        Utility.WriteLine(ConsoleColor.Cyan,
            $"[EpicUO] Hotbar execute type=0x{actionType:X2} id={actionId} from {player.Name}");
    }
}
```

- [ ] **Step 2: Add temporary stubs for Task 7 types (remove in Task 7)**

```csharp
// Append to HotbarRouter.cs — TEMP, removed in Task 7
internal static class HotbarStubResponses
{
    public static void SendAll(Server.Network.NetState ns, EpicPlayer p) { }
}
internal static class TooltipHandler
{
    public static void Handle(Server.Network.NetState ns, EpicPlayer p, SpanReader r) { }
}
```

- [ ] **Step 3: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 4: Commit**

```bash
git add Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs
git commit -m "feat: 0xD7 router — dispatches request/execute/tooltip subcommands"
```

---

## Task 7: Hotbar Stub Responses

**Files:**
- Create: `Projects/Epic.Content/Network/Packets/Hotbar/HotbarStubResponses.cs`
- Create: `Projects/Epic.Content/Network/Packets/Hotbar/TooltipHandler.cs`
- Modify: `Projects/Epic.Content/Network/Packets/Hotbar/HotbarRouter.cs` — remove temp stubs

**Goal:** When client sends `0x50`, server responds with empty-but-valid packets for each category. Hotbar window opens with empty slots — no crash.

**Interfaces:**
- Produces: `HotbarStubResponses.SendAll(NetState, EpicPlayer)` — sends `0x53`–`0x59`
- Produces: `TooltipHandler.Handle(NetState, EpicPlayer, SpanReader)` — sends `0x61`

- [ ] **Step 1: Create HotbarStubResponses.cs**

Minimal valid response: `[0xD7][length:2][subcommand:2][count:2]` = 7 bytes, count=0.

```csharp
// Projects/Epic.Content/Network/Packets/Hotbar/HotbarStubResponses.cs
using Epic.Content.Mobiles;
using Server.Network;
using System;
using System.Buffers.Binary;

namespace Epic.Content.Network.Packets.Hotbar;

public static class HotbarStubResponses
{
    private const byte PacketId = 0xD7;

    // Subcommand IDs from shared_constants.inc
    private const ushort AbilitiesResponse = 0x53;
    private const ushort SpecialsResponse  = 0x54;
    private const ushort SpellsResponse    = 0x55;
    private const ushort SkillsResponse    = 0x56;
    private const ushort CommandsResponse  = 0x59;

    public static void SendAll(NetState ns, EpicPlayer player)
    {
        SendEmpty(ns, AbilitiesResponse);
        SendEmpty(ns, SpecialsResponse);
        SendEmpty(ns, SpellsResponse);
        SendEmpty(ns, SkillsResponse);
        SendEmpty(ns, CommandsResponse);
    }

    private static void SendEmpty(NetState ns, ushort subcommand)
    {
        if (ns.CannotSendPackets()) return;

        Span<byte> packet = stackalloc byte[7];
        packet[0] = PacketId;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], 7);       // length
        BinaryPrimitives.WriteUInt16BigEndian(packet[3..], subcommand);
        BinaryPrimitives.WriteUInt16BigEndian(packet[5..], 0);       // count = 0
        ns.Send(packet);
    }
}
```

- [ ] **Step 2: Create TooltipHandler.cs**

Client sends: `[0xD7][size][0x0060][actionType:1][actionId:2]`
Server responds: `[0xD7][size][0x0061][actionType:1][actionId:2][descLen:2]` (no desc bytes for stub)

```csharp
// Projects/Epic.Content/Network/Packets/Hotbar/TooltipHandler.cs
using Epic.Content.Mobiles;
using Server.Network;
using System;
using System.Buffers.Binary;

namespace Epic.Content.Network.Packets.Hotbar;

public static class TooltipHandler
{
    private const byte   PacketId        = 0xD7;
    private const ushort TooltipResponse = 0x61;

    public static void Handle(NetState ns, EpicPlayer player, SpanReader reader)
    {
        if (ns.CannotSendPackets()) return;

        var actionType = reader.ReadByte();
        var actionId   = reader.ReadUInt16();

        // [0xD7][length:2][0x61][actionType:1][actionId:2][descLen:2] = 10 bytes
        Span<byte> packet = stackalloc byte[10];
        packet[0] = PacketId;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], 10);
        BinaryPrimitives.WriteUInt16BigEndian(packet[3..], TooltipResponse);
        packet[5] = actionType;
        BinaryPrimitives.WriteUInt16BigEndian(packet[6..], actionId);
        BinaryPrimitives.WriteUInt16BigEndian(packet[8..], 0); // descLen = 0
        ns.Send(packet);
    }
}
```

- [ ] **Step 3: Remove temp stubs from HotbarRouter.cs**

Delete the two `internal static class` blocks added at the end of `HotbarRouter.cs` in Task 6.

- [ ] **Step 4: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 5: E2E — hotbar opens without errors**

Start ModernUO. Connect epicuo. Open the hotbar.

Expected:
- Hotbar window appears
- All slots empty (correct — no abilities assigned yet)
- No "connection lost", no crash, no error popup in client
- Server log shows no `Unknown 0xD7` warnings for `0x50` or `0x60`

**If `Unknown 0xD7` appears for `0x50`:** The subcommand read offset is wrong. The `SpanReader` position after the packet ID byte depends on how ModernUO frames the reader. Add a debug log before reading subcommand: `Utility.WriteLine(ConsoleColor.Magenta, $"[Debug] 0xD7 remaining={reader.Remaining}");` and compare against the expected layout to find the correct number of bytes to skip.

- [ ] **Step 6: Commit**

```bash
git add Projects/Epic.Content/Network/Packets/Hotbar/
git commit -m "feat: hotbar stub responses (0x53–0x59) + tooltip stub — hotbar loads without errors"
```

---

## Task 8: Minor Packets (0x62, 0x63, 0x5A, 0x0070)

**Files:**
- Create: `Projects/Epic.Content/Network/Packets/Hotbar/MinorPackets.cs`
- Modify: `Projects/Epic.Content/Mobiles/EpicPlayer.cs` — call `SendEmptyBuffList` and `SendFogDisabled` on login

**Goal:** Prevent remaining 0xD7 subcommands from causing client-side errors on login.

- [ ] **Step 1: Create MinorPackets.cs**

```csharp
// Projects/Epic.Content/Network/Packets/Hotbar/MinorPackets.cs
using Server.Network;
using System;
using System.Buffers.Binary;

namespace Epic.Content.Network.Packets.Hotbar;

public static class MinorPackets
{
    private const byte PacketId = 0xD7;

    // 0x62 — Custom Buff — empty list (no active buffs on login)
    public static void SendEmptyBuffList(NetState ns) => SendEmpty(ns, 0x62);

    // 0x63 — Equipment info — stub (wired in equip system plan)
    public static void SendEquipmentInfoStub(NetState ns) => SendEmpty(ns, 0x63);

    // 0x5A — AoE range preview — range=0 disables the circle preview
    public static void SendAoeRangeZero(NetState ns)
    {
        if (ns.CannotSendPackets()) return;
        Span<byte> packet = stackalloc byte[8];
        packet[0] = PacketId;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], 8);
        BinaryPrimitives.WriteUInt16BigEndian(packet[3..], 0x5A);
        BinaryPrimitives.WriteUInt16BigEndian(packet[5..], 0); // range = 0
        ns.Send(packet);
    }

    // 0x0070 — Atmospheric fog — disabled (Nature system wires real values later)
    public static void SendFogDisabled(NetState ns)
    {
        if (ns.CannotSendPackets()) return;
        Span<byte> packet = stackalloc byte[8];
        packet[0] = PacketId;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], 8);
        BinaryPrimitives.WriteUInt16BigEndian(packet[3..], 0x0070);
        packet[5] = 0; // enabled = false
        ns.Send(packet);
    }

    private static void SendEmpty(NetState ns, ushort subcommand)
    {
        if (ns.CannotSendPackets()) return;
        Span<byte> packet = stackalloc byte[7];
        packet[0] = PacketId;
        BinaryPrimitives.WriteUInt16BigEndian(packet[1..], 7);
        BinaryPrimitives.WriteUInt16BigEndian(packet[3..], subcommand);
        BinaryPrimitives.WriteUInt16BigEndian(packet[5..], 0);
        ns.Send(packet);
    }
}
```

- [ ] **Step 2: Call on login — add to EpicPlayer.OnNetStateChanged()**

```csharp
// In EpicPlayer.cs — update OnNetStateChanged:
public override void OnNetStateChanged()
{
    base.OnNetStateChanged();
    if (NetState != null)
    {
        ExtendedStatsPacket.Send(NetState, this, this);
        MinorPackets.SendEmptyBuffList(NetState);
        MinorPackets.SendFogDisabled(NetState);
    }
}
```

- [ ] **Step 3: Build**

```bash
dotnet build Projects/Epic.Content/Epic.Content.csproj
```

Expected: `Build succeeded.`

- [ ] **Step 4: E2E — clean login**

Connect epicuo. On login, verify in the server log:
- No `Unknown 0xD7` warnings
- No client error dialogs
- Buff bar area shows nothing (correct — no buffs)
- No fog overlay (correct — disabled)

- [ ] **Step 5: Commit**

```bash
git add Projects/Epic.Content/Network/Packets/Hotbar/MinorPackets.cs \
        Projects/Epic.Content/Mobiles/EpicPlayer.cs
git commit -m "feat: minor packets — buff list, fog, AoE stubs sent on login"
```

---

## Self-Review

**Spec coverage:**
- ✅ `Game.Core` project, zero engine deps enforced — Task 0
- ✅ `Epic.Content` project referencing Game.Core + Server — Task 0
- ✅ `EpicPlayer` with all serialized Epic fields — Task 1
- ✅ Razor handshake `0xF0` — Task 2
- ✅ HWID handling (short `0xF0`) — Task 2
- ✅ `AttributeSnapshot` with all fields from `sendStats.src` — Task 3
- ✅ `IdentityRules.DisplayName()` with Desconhecido logic + unit tests — Task 3
- ✅ `MobileAdapter` — Task 4
- ✅ `ExtendedStatsPacket` — 78-byte custom `0x11` — Task 5
- ✅ Incoming `0xD7` router — Task 6
- ✅ Hotbar stubs `0x53`–`0x59` — Task 7
- ✅ Tooltip stub `0x60` → `0x61` — Task 7
- ✅ Custom buff empty `0x62` — Task 8
- ✅ Equipment info stub `0x63` — Task 8 (`SendEquipmentInfoStub` available, wired in equip plan)
- ✅ AoE range `0x5A` stub — Task 8
- ✅ Fog disabled `0x0070` — Task 8
- ⚠️ Experience packet `0xD7:0x57` — NOT in this plan. No progression system yet. The router logs it as unknown (harmless). Wired in the Progression plan.
- ⚠️ Localization / `T.Get()` — NOT in this plan. All strings are currently English constants. Full string table wired in the Localization plan.

**Placeholder scan:** None. All steps have real code.

**Type consistency:**
- `AttributeSnapshot` defined Task 3 → used Task 4 ✅, Task 5 ✅
- `IdentitySnapshot` defined Task 3 → used Task 4 ✅, Task 5 ✅
- `IdentityRules.DisplayName(IdentitySnapshot, bool)` defined Task 3 → called Task 5 ✅
- `EpicPlayer` fields (`Fortitude`, `Reflexes`, `WillStat`, `DeathPointsCurrent`, `DeathPointsMax`, `HungerValue`, `AddictionStr/Dex/Int`, `EpicLevel`, `IsIncognito`, `IncognitoName`) defined Task 1 → read Task 4 ✅
- `HotbarStubResponses` temp stub in Task 6 → replaced by real class Task 7 ✅
- `TooltipHandler` temp stub in Task 6 → replaced by real class Task 7 ✅
- `EpicPacketRegistry` wires `RazorHandshakeHandler.Handle` (Task 2) and `HotbarRouter.Handle` (Task 6) ✅
- `MinorPackets.SendEmptyBuffList` + `SendFogDisabled` defined Task 8 → called in `EpicPlayer.OnNetStateChanged` Task 8 ✅
