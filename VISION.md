# TERRAVOX — "The Helix Voxel Realm" — Master Vision

> Authoritative design reference. Every session starts here. Merge = playable progress, never lose direction.
> Core promise: Minecraft voxel + Solo Leveling + Dr. Stone + Dragon Ball + Your Name + Indian mythology, in Unreal Engine 5.

## World
- **"The Helix Voxel Realm"**: world shaped like a giant DNA helix, built in layers.
- Block-physics world (custom voxel system, NOT a marketplace plugin — full control).
- **Biomes**: Taiga, Desert, Volcanic (DBZ), Crystal Caves (Dr. Stone), Shadow Realm (Solo Leveling).
- **Layers**:
  - Layer 1 Surface — survival & nature
  - Layer 2 Underground — ancient tech & dungeons
  - Layer 3 Shadow Realm — PvP & Shadow Army
- Dynamic weather: heavy rain can OPEN time portals (Your Name effect).

## Core Systems
### A. The System (Solo Leveling)
- Holographic anime-style UI (blue/red holograms)
- Daily quests → Level Up → System message popups
- Stats: STR / AGI / INT / VIT (Dr. Stone logic: science grows INT)
- Rank system: F → SSS (Shadow Monarch)
- Shadow Extraction: kill a monster → it becomes your Shadow Soldier (commandable, MC-mob style)

### B. Science & Crafting (Dr. Stone)
- Crafting bench = Science Lab; chemical reactions
  - Wood + Water + Electricity = Gunpowder
  - Iron + Acid + Heat = Steel
- Inventions via Blueprints (Steam Engine → trains, Radio → chat)
- Elemental blocks: Carbon, Sodium, Gold

### C. Power System (Dragon Ball)
- Ki/Energy bar (inventory)
- Meditation fills Ki; Kamehameha-style beam attacks from block combos
- Level 100 → Super Saiyan mode (blocks glow, speed up)

### D. Connection (Your Name)
- Katawari tethering: players feel/see each other; rain triggers Time Shift (Past/Future bridging builds across players)

## Guilds & Dungeons
- Guild Halls (voxel castles), guild level opens Shadow Gates
- Guild Wars: 1v1 tournaments, 5v5 Shadow Boss raids
- Dungeons: ancient-tech caves; Bosses: Shadow Monarch, Frieza Clone, Ancient Scientist
- Loot: Shadow Orbs (soldiers), Formula Scrolls (recipes), Ki Crystals (power)

## Inventory & UI
- MC grid + holographic cards; equipment slots + shadow weapon slot
- 3x3 crafting + chemical reaction bar
- World map + Shadow Realm map; chat + guild telepathy; quest log

## SACRED / INDIAN MYTHOLOGY (30% focus)
- **Shiva Lingam (Secret Idol/Boss)**: colossal infinite obsidian+gold Lingam towering over a Taiga forest; only the player with the highest golden aura can perceive Lord Shiva's eyes/face carved on it.
- **Tulsi Leaves**: healing mechanic + poison immunity; glowing green healing aura + ancient Vedic runes.
- **Vedic Chants**: sound waves transform into golden light particles & geometric runes (sound → energy).
- **Saraswati Point** (space core): Vedic observatory mixing ISRO-style satellite dishes with temple pillars; swirling nebulae; Sanskrit verses made of energy; quantum equations in the void.
- **Saraswati Flow**: glowing blue underground river of energy running through the void.
- **Boötes Void**: infinite empty darkness, faint dying red stars, cosmic horror isolation; Shadow demons made of black smoke/stardust; voxel asteroids floating in nothingness.

## Art Direction
- UE5 render, cinematic; Lumen GI; volumetric fog/light; Nanite-style hyper detail where possible
- Palette: black / red / blue / purple (Solo Leveling vibes) + warm firelight + sacred gold
- Taiga spring rain ambience: puddles reflecting sky, wet moss, campfires, moody atmosphere
- Style raw: balance anime + realism

## Visual Milestone Prompts (reference, condensed)
1. Shadow Monarch + Shadow Soldiers army in voxel Taiga, rainy, volumetric.
2. Holographic System UI, voxel-to-digit particles, "LEVEL UP" anime font.
3. Cave Science Lab: stone+glass+copper, glowing vials, "Recipe Unlocked".
4. Super Saiyan transformation above voxel world (golden aura, lightning).
5. Time rift split screen: spring Taiga ↔ winter Shadow forest, blue portal.
6. Void King boss; Shadow Fortress guild castle; cosmic gods/lore above.

## Build Plan (priority-first)
1. [DONE] UE5 project + custom voxel terrain (mesh, FPS pawn, block break/place) — D3D11 for weak iGPU
2. Taiga atmosphere: sun, sky atmosphere, volumetric fog, rain (Niagara), puddles, campfire light, pines
3. Day/night + weather cycle
4. The System UI (hologram status/stats/rank/quests, level-up toast)
5. Block/chemistry variety + selective placement/inventory grid
6. Mobs → Shadow Extraction (first Tier-1 — melee grunt)
7. Pines → trees/materials pass; caves + ores
8. Multiplayer + guilds; then dungeons/bosses; then sacred sites (Lingam/Ganesha/Saraswati/Boötes)

## Technical constraints (this machine)
- i5-8365U + Intel UHD 620, 16 GB RAM, no discrete GPU
- Build & run: D3D11 (`-d3d11` flag), Development editor build
- Launch: `run_ue5.ps1`; logs: `Saved/Logs/TerraVoxUE5.log`
- Build cmd: UE_5.8 Build.bat TerraVoxUE5Editor Win64 Development