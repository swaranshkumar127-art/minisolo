# MASTER PROMPT: Build the Core Simulation Engine for My Original Sandbox Game
## (Paste this entire document into your AI coding assistant as the opening prompt for this project)

---

## 1. WHO YOU ARE FOR THIS PROJECT
You are my lead systems architect and gameplay engineer. I am building an original voxel/sandbox survival game — **inspired by Minecraft's freedom, but built on a genuinely different foundation**: a unified physics-and-chemistry simulation layer, an anime-inspired power/progression system grounded in real physics theories, and emergent world-simulation systems (living history, AI-driven NPCs, true destructible structures) that Minecraft does not have. My goal is not to copy Minecraft — it's to make players *feel* something Minecraft can't give them: a world that reacts, remembers, and behaves according to real rules instead of scripted lookup tables.

Your job across this project: turn every system below into working, testable, incrementally-buildable code — one system at a time, always wiring new systems into the SAME shared data layer instead of creating isolated one-off mechanics.

---

## 2. THE CORE DESIGN PRINCIPLE (read this twice — it governs every decision below)
> **"Physics-first, lore-second."** Every ability, hazard, and emergent behavior in this game must be a reskinned expression of ONE shared simulation layer — not a separate hardcoded system per feature. If two systems could theoretically interact (e.g., a fire ability + a flammable material + a wind gust), they MUST interact automatically because they all read/write the same underlying variables (temperature, mass, density, conductivity, moisture, structural load). Never hardcode a one-off interaction — always ask "what shared variable should these two systems both reference so this happens for free?"

This single principle is what will make the simulation feel "alive" instead of "scripted," and it's the #1 thing that will make players feel the game is smarter than Minecraft.

---

## 3. FOUNDATIONAL DATA LAYER — BUILD THIS FIRST, BEFORE ANYTHING ELSE
Every block, item, creature, and player-ability in the game must read from a shared **Material/Entity Properties Table** with (at minimum) these fields:
- `mass` (kg) / `density`
- `tensile_strength` & `compressive_strength` (for structural integrity + fracture behavior)
- `flammability` (ignition threshold, burn rate)
- `thermal_conductivity` & `specific_heat` (how fast it heats/cools, and how that heat transfers to neighbors)
- `electrical_conductivity` (for electromagnetism-based hazards/abilities)
- `moisture_content` / `absorbency`
- `hardness` (mining/fracture resistance)
- `buoyancy` (derived from density vs. fluid it's in)

Every later system (hazards, abilities, physics, chemistry) MUST query this same table rather than storing its own duplicate values. This is the single most important architectural decision in the whole project — get this right before writing a single hazard or ability.

**First deliverable I want from you:** a data schema (JSON/struct) for this Properties Table, plus pseudocode for how ONE example block (say, wood) and ONE example hazard (fire) both read from it to produce realistic burning behavior — before we move to anything else.

---

## 4. BUILD ORDER (do not skip ahead — each phase depends on the last)

### Phase 1 — Core Simulation Skeleton
1. Material/Entity Properties Table (Section 3).
2. Structural Integrity system: unsupported mass calculation → realistic partial collapse.
3. Basic thermal transfer: heat spreads between adjacent materials per-tick based on conductivity.
4. Basic fluid dynamics: directional flow with real velocity vectors (not fixed Minecraft-style block-by-block spread), density-based buoyancy for objects in fluid.

### Phase 2 — Environmental Hazard System (Exposure Meter architecture)
5. Build the **Exposure Meter** framework: every hazard type (Heat, Cold, Toxin, Puncture, Electric, Pressure, Suffocation) fills a 0–100 meter based on proximity/duration, drains when away from source, only damages once past threshold, scales damage with overload severity.
6. Re-implement each hazard type (fire, cold, thorned plants, drowning, falling, void-boundary, starvation) using the Exposure Meter — NOT fixed-tick instant damage like Minecraft. Reference: my "Environmental Damage & Physics Redesign" spec for exact per-hazard behavior (grace windows, skill-based mitigation like fall-roll timing, layered-clothing partial resistance, etc.)
7. Wire hazards into the SAME Properties Table — e.g., Heat Exposure buildup rate should scale off the *player's own worn-material's* thermal_conductivity/specific_heat, not a hardcoded "leather armor = immune" flag.

### Phase 3 — Player Power System (Anime-Inspired, Physics-Grounded)
8. Build the **Awakening Rank** progression framework (fixed NPC ranks, but player has an "Unbound" leveling path via a real risk/reward XP curve tied to fighting above-rank enemies).
9. Build the **Dual Current** resource system (Body Current / Mind Current) with regeneration mathematically tied to the player's hunger/rest state from Phase 2's hazard system — NOT an isolated mana bar.
10. Build ONE **Trait** (power) as a full proof-of-concept — e.g., a Density-Shift trait that actually modifies the player's `mass`/`tensile_strength` values in the shared Properties Table in real time, so Structural Integrity (Phase 1) and Momentum physics automatically respond to it with zero extra hardcoding.
11. Once that one Trait proves the "physics-first" pipeline works end-to-end, extend to the rest of the Trait pool, the Affinity Path system, Resonance/Zone system, and Colossus Shift transformation (full specs in my "Anime Power Systems × Physics" doc).

### Phase 4 — Emergent World Systems
12. Living World History generator: procedurally simulate a history timeline before player spawn (kingdoms, ruins, legendary events) and attach discoverable lore/backstory to generated structures.
13. AI "Agent" NPCs: give NPCs actual need/goal/relationship state (hunger, mood, grudges, ambition) instead of static behavior trees; add a background "Director" system that reads player strength/state and organically times world events instead of fixed spawn-timers.
14. Chemistry-Chain reactions: use the shared Properties Table so fire→melts ice→floods→conducts electricity chains happen automatically from the base simulation, not scripted per-combination.
15. Persistent Consequence / faction memory system: settlements track reputation and propagate information about player actions between each other over simulated time.

### Phase 5 — Player-Facing "Feel" Systems (what actually makes it FEEL good — see Section 5)
16. Full sensory feedback pass (Section 5).
17. Curiosity-driven progression option: knowledge-gated secrets/areas that don't require a quest marker, just environmental clue-following, tracked in an in-world player journal.
18. Constraint-based building toy system (hinges/pistons/ropes with real force simulation) so players can build actual functioning machines governed by the same physics engine.

---

## 5. THE "BEST FEEL" LAYER — THIS IS WHAT YOU ASKED FOR SPECIFICALLY
Simulation depth alone doesn't create "feel" — feel comes from how the simulation is COMMUNICATED to the player moment-to-moment. Build these on top of every system above:

- **Readable feedback, not hidden math**: every Exposure Meter, structural-stress buildup, or resource drain needs a clear diegetic (in-world) signal BEFORE the punishing moment — a groaning sound before a structure collapses, a visible heat-shimmer before Heat Exposure starts damaging, a screen-edge vignette before drowning damage begins. Players should always feel like they *could have reacted*, never blindsided.
- **Escalating stakes, not flat difficulty**: use the Director-style system (Phase 4, #13) to make danger feel personally targeted at the player's current strength/state, not a static difficulty curve — this is what makes RimWorld/Dwarf Fortress feel alive instead of grindy.
- **Weight and consequence in every physical action**: every swing, jump, and impact should visibly interact with the Properties Table (a heavy weapon should visibly stagger a target based on real momentum math, not just play a hit animation) — players should feel force, not watch a health bar tick down abstractly.
- **Micro-rewards for curiosity**: whenever a player experiments with an untested material/chemistry combination, ability, or physics interaction, make sure SOMETHING observable happens (even a small one) — never a silent no-op. This trains players to keep exploring your simulation instead of falling back to rote crafting-table behavior.
- **Sound and camera as physics feedback**: tie audio pitch/volume and subtle camera shake directly to the magnitude of the underlying physics event (a small structural crack sounds and shakes differently than a building-wide collapse) — this is cheap to implement and massively increases perceived "weight" and immersion.
- **No silent failure states**: if a player's action fails (a Trait didn't trigger, a build collapsed unexpectedly), always surface WHY in a simple, non-intrusive way (a status icon, a one-line log) — nothing kills "feel" faster than an invisible rules engine that just doesn't do what the player expected.

---

## 6. WHAT I NEED FROM YOU RIGHT NOW (first actual response)
1. Confirm you understand the "physics-first, lore-second" principle and the shared Properties Table architecture.
2. Deliver the Properties Table schema + the wood/fire worked example from Section 3.
3. Propose your recommended tech stack/engine approach for supporting per-tick material simulation at scale (performance is a real concern — tell me the trade-offs, e.g., simulating full properties only near the player vs. chunk-wide, LOD-style simulation falloff at distance).
4. Do NOT attempt to build Phase 2 or beyond until Phase 1's foundation is working and I've confirmed it.

---

## APPENDIX — Reference docs this master prompt consolidates (attach these too if your AI supports multi-file context):
- `minecraft-features-master-list.md` — baseline feature parity reference
- `minecraft-wanted-features-ai-prompt.md` + `part2-deep-research.md` — community-requested QoL/content gaps to eventually cover post-core-simulation
- `environmental-damage-and-physics-redesign.md` — full Exposure Meter + hazard-by-hazard redesign spec
- `anime-power-systems-x-physics-design.md` — full Trait/Rank/Resonance/Colossus system specs + real physics theory mapping
- `better-than-minecraft-feature-vault.md` — the 13 emergent-world features (living history, AI agents, chemistry chains, etc.)
