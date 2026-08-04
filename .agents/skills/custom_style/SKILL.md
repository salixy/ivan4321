---
name: code-style
description: C++ code style for this project — naming, formatting, comment tone. Use whenever writing or editing C++ in this repo.
---

Derived from analysis of two reference codebases (`antisocial_leo`, a game-cheat/IL2CPP project, and `skinned_mesh_lod_group.cpp`, a lower-level rendering module) — this skill picks the conventions that fit a desktop ImGui app, not the game-memory-hacking-specific bits (no IL2CPP method-pointer caching, no string obfuscation, no offset-based memory reads unless the task actually calls for it).

## Naming

- Classes/structs: `c_` prefix — `c_ui`, `c_tabs`, `c_framebuffer`.
- Member variables: `m_` prefix — `m_width`, `m_dragging`.
- File-static / translation-unit-local statics: `s_` prefix — `s_active_family`.
- Globals: `g_` prefix — `g_ctx`, `g_ui`.
- Compile-time constants: `k_` prefix, snake_case body — `k_padding_x`, `k_checkbox_size`, `k_min_child_size`.
- POD/value-type structs: `_t` suffix — `family_t`, `free_child_t`.
- Enums: `_e` suffix — `lod_path_e` style (prefer this over bare/mixed enum naming).
- Functions/methods: snake_case, verb-first — `draw_resize_handle`, `resolve_pool`, `set_family`.
- File naming: one primary symbol per file, filename == symbol name minus its prefix, snake_case.

## Formatting

- Allman braces: opening brace on its own line, for functions, classes, and control flow alike.
- Cuddled `else`: closing brace and `else` share a line (`} else {`), but if the `else` itself opens a block, that brace still goes on its own following line if the codebase's own convention is Allman throughout — match whatever the file you're editing already does; new files use the cuddled-brace-only-at-else-keyword form seen in antisocial_leo.
- Pad the interior of parens/brackets/angle-brackets: `if ( x )`, `foo( )` (yes, even empty argument lists), `std::vector< int >`, `array[ i ]`. This is the single most identifying trait of the style — apply it consistently.
- Pointers/references attach to the type, not the name: `c_transform* bone`, `ImVec2 const& pos` — never `c_transform *bone`.
- `const` goes trailing/east: `ImVec2 const wp`, `float const alpha` — not `const ImVec2 wp`. (This already matches how this codebase is written — see `elements.cpp`.)
- 4-space indentation, no tabs.
- No hard line-length cap — long chained boolean/ternary expressions are fine; wrap with the operator leading the continuation line when they do wrap.
- One blank line between logical steps inside a function (guard clauses, then a blank, then the main body); one blank line between groups of `#include`s.

## Comments

- Keep density low overall — most functions should have zero to very few comments; let names and small functions carry meaning.
- When a comment is worth writing, it explains WHY (a non-obvious constraint, a workaround, a subtle invariant) — never WHAT the code does.
- Occasional casual/irreverent phrasing is fine and matches this codebase's voice (e.g. a dry "// yeah this is dumb but it works" style aside) — but sparingly, maybe one or two per file at most, not on every function. Don't force it if there's nothing worth the joke.
- Don't leave commented-out dead code as a matter of routine — delete it, unless there's a specific reason to keep it visible for the next person.

## Structure

- Prefer local lambdas over private helper methods for one-off decomposition inside a function.
- Prefer designated initializers for aggregate structs where it reads cleanly: `{ .id = x, .value = y }`.
- Guard-clause / early-return style over deep nesting.
- Raw pointers for non-owning views; reach for RAII/smart pointers only where this code actually owns a resource (GL handles, heap allocations) — mirrors `skinned_mesh_lod_group.cpp`'s explicit create/destroy symmetry, not antisocial_leo's raw-memory-view style (which doesn't apply here since we don't reach into another process's memory).

---

## Full reference analysis (verbatim from the source agent)

The above rules are the distilled, applicable subset. Below is the complete original analysis for both source codebases, kept for exact quotes/examples if a judgment call comes up that the summary above doesn't cover.

### 1. `antisocial_leo`

#### Naming conventions

- **Classes/structs**: `c_` prefix almost universally — `c_hooks`, `c_ragebot`, `c_features_manager`, `c_esp`, `c_config`, `c_player_controller`, `c_widgets`, `c_base_widget`, `c_animator`, `c_photon_player`, `c_biped_map`. Non-gameplay ObjC classes break this: `antisocial`, `WebSocketManager`, `SoundManager`, `AppBadgeView`, `DeviceMetrics` (PascalCase, standard ObjC convention).
- **Structs used as POD/value types**: `_t` suffix — `vec3_t`, `quater_t`, `euler_angles_t`, `antiaim_config_t`, `chams_config_t`, `spoof_config_t`, `variable_object_t`, `esp_fonts_t`.
- **Enums**: inconsistent suffixing — `antiaim_t` (enum class), `view_mode_t`, `sound_t`, but also bare `prank_type`, `surface_type`. No single convention was applied consistently.
- **Members**: `m_` prefix everywhere — `m_target`, `m_fire`, `m_pos`, `m_configs`, `m_allowed_edit`, `m_name_hash`. Statics inside translation units are plain snake_case with no `s_`/`g_` prefix (`selected_cfg`, `delete_confirmation`).
- **Globals**: `g_` prefix for the one big global — `g_ctx` (a `globals_t*` service locator holding `features`, `interface`, `il2cpp`, `hooks`, `cfg`, `audio_player`, `widgets`, `user_data`, etc). Non-`g_` globals also exist inconsistently (`wsManager`, `selector`, `self_device` — lowercase, no prefix).
- **Functions/methods**: snake_case throughout — `process_local_player`, `select_first_available_point`, `get_original_view_angles`, `bind_bone_transform`.
- **Hook plumbing idiom** (very consistent, load-bearing for "sounding like this codebase"):
  - Original function pointer: `og_<name>` (`og_filter`, `og_servers`, `og_task_info`, `og_dlsym`).
  - Hook replacement function: `hk_<name>` (`hk_filter`, `hk_servers`, `hk_task_info`, `hk_dlsym`, `hk_exit`, `hk_abort`).
  - Hook namespaces mirror the directory tree 1:1: `hk::player_inputs::create_move::update()`, `hk::hit_caster::cast_hit::hook()`, `hk::chat_manager::send_to_all::hook()` — each hook lives in its own folder `game/hooks/<system>/<hook_name>/<hook_name>.{hpp,cpp}` exposing exactly `void hook()` (and sometimes `void update()`).
- **Constants/macros**: SCREAMING_SNAKE for C macros (`SMALL_FLOAT`, `PI`, `Deg2Rad` — note `Deg2Rad`/`Rad2Deg` break the SCREAMING_SNAKE rule, PascalCase leftover from a ported Unity/C# source). `declare_variable(type, name, default)` macro is the load-bearing config-reflection idiom (see below).
- **File naming**: all lowercase snake_case, and the convention is **one class per folder**, folder name == file basename == primary symbol name minus `c_`: `game/features/misc/aspect_ratio/aspect_ratio.{hpp,cpp}` → `c_aspect_ratio`. This is followed with zero exceptions across ~250 files.
- **Two different author "signatures" show up in file header comments** (evidence of multiple contributors, useful if you want to imitate provenance-style headers):
  ```
  //
  // Created by rei on 12/18/23.
  //
  ```
  ```
  //
  // Created by Kai Tears on 16/08/2025.
  //
  ```
  Files by "Kai Tears" (widgets, elements, alive_players, featurespages, etc.) also switch header-guard style to `#ifndef X_HPP / #define X_HPP / #endif`, whereas the older core (`singleton.hpp`, `vec3.hpp`, `memory_manager.hpp`, `config.hpp`) uses `#pragma once`. Both conventions coexist in the same project.

#### Formatting

- **Extremely consistent, heavily-spaced "BSD-ish/Allman" style** across the entire `cheat/` tree (this is the dominant house style):
  - Braces: **Allman** (own line) for functions, classes, and control flow alike:
    ```cpp
    void c_ragebot::execute( c_player_controller* local )
    {
        if ( !c::get< bool >( g_ctx->cfg.ragebot_enabled ) )
            return;
    ```
  - **Cuddled `else`**: closing brace and `else` share a line, but the `else`'s own opening brace goes to the next line:
    ```cpp
    } else if ( !strcmp( model, "iPhone14,4" ) || ... )
    {
        return 2.88f;
    } else
    {
        return ( float ) UIScreen.mainScreen.scale;
    }
    ```
  - **Pervasive interior spacing** — space just inside every paren/bracket/angle-bracket, even for zero-arg calls:
    - `if ( x )`, `for ( int i = 0; i < 4; i++ )`, `foo( )` (space before closing paren of empty arg list), `response[ crypt_string( "data" ) ]`, `std::vector< unsigned char >`, `std::optional< std::string >`.
    - This is the single most recognizable fingerprint of this codebase — nearly every `(`, `[`, `<...>` gets padding.
  - **Pointer/reference placement**: left-attached to type, `c_transform* bone`, `const vec3_t& view`, `uintptr_t* modified` — never `c_transform *bone` or `c_transform * bone`.
  - **Indentation**: 4 spaces, no tabs.
  - **`switch`**: `case` labels flush with `switch`, not indented an extra level; braces sometimes wrap individual cases (`case 1: { ... break; }`), sometimes not.
  - **Ternaries and long boolean chains** wrap with the operator leading the continuation line, heavily parenthesized:
    ```cpp
    aa.yaw = flip
                ? c::get< antiaim_config_t >( g_ctx->cfg.ragebot_antiaim_config ).jitter_first
                : c::get< antiaim_config_t >( g_ctx->cfg.ragebot_antiaim_config ).jitter_second;
    ```
  - **Line length**: not capped — many lines run 150–250+ columns, especially chained `c::get<T>(g_ctx->cfg.x)` conditionals and reinterpret_cast chains.
  - **Blank lines**: one blank line between logical steps inside a function (guard clauses, then blank, then main body); one blank line between includes-groups and between functions; no blank line inside short getter-style one-liners.
- **`bypass.xm1` is a stylistic outlier** — compact K&R style, no interior-paren spacing (`if (containsFanta) {`), opening braces cuddled on the same line as `if`/`function`, tabs/looser indentation, effectively looks like it was pasted in from a community jailbreak-detection-bypass snippet rather than written in-house. If you need "in-house cheat logic" style, model off `ragebot.cpp`/`vec3.hpp`/`config.cpp`, not `bypass.xm1`.
- **Objective-C++ files** (`antisocial.mm`, `WebSocketManager.mm`, `SoundManager.mm`) keep the same interior-spacing convention even in ObjC method signatures: `- ( void )touchesBegan:( NSSet< UITouch* >* )touches withEvent:( UIEvent* )event`, `+ ( instancetype )sharedManager`.

#### Comments

- **Low density overall** (roughly 1 comment per 20–40 lines in feature code; near-zero in SDK offset-accessor `.cpp` files like `player_controller.cpp`).
- What gets commented: almost never "what" the code does; occasionally a one-word contextual tag next to a config field (`// jitter`, `// pitch cfg`, `// yaw cfg`, `// only pixel`, `// only vischeck`), or a numeric-enum legend (`// 0 - default, 1 - down, 2 - up`).
- **Dead/commented-out code is left in place routinely** rather than deleted (`// if ( c::get< bool >( g_ctx->cfg.ragebot_silent ) )`, whole blocks of ObjC dispatch calls commented out with `//`).
- **Casual, self-aware, sometimes profane developer voice** in comments — distinctive and worth imitating if the goal is "sound like this exact codebase":
  - `// TODO, or does someone give a fuck actually?`
  - `// shit ass code`
  - `// yep, now this should be done locally`
  - `void log_shit( const std::string& text )`
  - A Russian poem literally printed to stdout in `viewDidLoad` (`я потерял себя / от безысходности я хотел убить себя...`).
  - Occasional Russian-language comments (`// предзагрузка`, `// Готово к воспроизведению`) indicating the author is a Russian speaker working in English identifiers.
- No Doxygen/banner-style comment blocks anywhere. No file-level "purpose" comments except the two author-attribution headers noted above.
- Rare credit comment: `/* @credits: underscorediscovery */` above the FNV-1a implementation.

#### Code structure / logic patterns

- **Function length**: short-to-medium for SDK accessors (1–3 lines, pure offset reads), long for feature `execute()`/`on_gui()` methods (50–150+ lines), driven by guard-clause style rather than decomposition.
- **Error handling**: almost entirely **bool return codes + early-return guard clauses** (`if ( !local || !local->alive( ) ) return;`). Exceptions exist only at the ObjC++/JSON boundary (`try { nlohmann::json... } catch ( const nlohmann::json::parse_error& )`), never in core gameplay/SDK code (Makefile actually sets `-fno-exceptions` then re-enables `-fexceptions` later on the same line — contradictory flags, exceptions end up enabled, but the *convention* is still "don't throw in game logic, only at JSON/network boundaries"). `player_controller.cpp::alive()` wraps a `try { ... } catch ( ... ) { return false; }` around raw pointer dereferences as a crash-guard — a defensive-programming idiom, not real exception use.
- **Singleton pattern is the backbone of the whole architecture**: a shared CRTP base (`template<typename T> class singleton` in `singleton.hpp`, using `std::call_once`) that every manager class inherits from (`c_hooks`, `c_ragebot`, `c_features_manager`, `c_config`, `c_widgets`, `c_esp`, `c_il2cpp_api`). Access is always `ClassName::get()`. A single god-object `g_ctx` (a `globals_t*`) is threaded through nearly every file as the composition root.
- **Manual memory/offset access is pervasive and load-bearing** — this is a live-memory Unity/IL2CPP game cheat. SDK wrapper classes (`c_player_controller`, `c_transform`, etc.) have **empty header bodies** (just method declarations, zero data members) — `this` literally *is* the remote object's address, and every accessor does `*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + 0xNN)`. Example idiom repeated dozens of times:
  ```cpp
  c_biped_map* c_player_controller::biped_map( )
  {
      return *reinterpret_cast< c_biped_map** >( reinterpret_cast< uintptr_t >( this ) + 0x30 );
  }
  ```
- **IL2CPP method-calling idiom**: cache a `static auto fn = reinterpret_cast<Ret(*)(Args...)>(g_ctx->il2cpp->get_method_pointer(ay_obfuscate("Namespace"), ay_obfuscate("Class"), ay_obfuscate("Method"), argc));` then call `fn(this, ...)` — appears near-identically in every SDK method that needs to call back into managed code.
- **Two distinct string-obfuscation helpers used for different purposes**: `crypt_string("...")` wraps general/UI/network string literals; `ay_obfuscate("...")` wraps IL2CPP/reflection/ObjC selector strings specifically. Both feed the Hikari-LLVM obfuscator (see Makefile: `-mllvm -hikari -mllvm -enable-bcfobf -mllvm -bcf_junkasm ...`, plus custom `__attribute__((annotate("strenc")))` / `__attribute__((annotate("fla")))` markers on individual functions to selectively opt them into string-encryption and control-flow-flattening passes). This is a heavily anti-RE-hardened commercial cheat, not a hobby project — a defining domain trait.
- **Config system is reflection-via-macro**: `declare_variable(type, name, default)` expands to `const std::uint32_t name = c::add_variable<type>(fnv1a::hash_const(#name), fnv1a::hash_const(#type), default);`, registering into a flat `std::vector<variable_object_t>`; all reads/writes go through `c::get<T>(handle)`. Serialization (`c_config::to_string`/`load_from_string`) is a big `switch` on `fnv1a::hash_const("typename")` dispatching to per-type `pack_*`/`get_*` free functions, base64-encoded, sent to a cloud endpoint.
- **UI is immediate-mode ImGui, mixed directly into feature/business logic** (not cleanly separated) — `render()` methods on feature classes call `ImGui::GetBackgroundDrawList()->AddCircleFilled(...)` etc. directly. A recurring idiom for animated widget state: a local `struct _state { c_animator field{...}; };` plus `static std::unordered_map<ImGuiID, _state> __state;` keyed by `ImGui::GetID(...)`, giving each widget instance persistent per-frame lerp state without a formal widget class hierarchy. `c_animator` (custom tween helper, `.update<T>(target)` / `.get<T>()`) is used everywhere color/alpha/width needs to animate.
- **Hand-rolled math library** (`vec3_t`, `quater_t`, `euler_angles_t` in `vec3.hpp`/`euler.hpp`) rather than GLM or similar — full operator overload sets (`+=`, `-=`, `*=`, `/=`, free `operator+/-/*//`), Unity-mirroring static helper names (`vec3_t::lerp`, `::move_towards`, `::rotate_towards`, `quater_t::look_rotation`) that map 1:1 onto `UnityEngine.Vector3`/`Quaternion` API surface — because this is reverse-engineered from Unity's own math.
- **Includes**: grouped by origin, blank line between groups, no strict alphabetization within a group — own header first, then closely related headers, then a blank line, then more distant subsystem headers, ObjC framework imports (`#import`) always precede C/C++ stdlib includes in `.mm` files.
- **Raw pointers only** in gameplay/SDK code (never smart pointers — these are non-owning views into another process's/engine's memory); `std::unique_ptr`/`std::shared_ptr` appear only in infrastructure (`singleton<T>`, `c_variable<T>`).
- **Templates**: used narrowly and functionally — `singleton<T>`, `c::get<T>()`, `c_variable<T>`, `il2cpp_hooker::hook<hk, orig_t>` — not heavy generic-metaprogramming, just enough to build the reflection/config/singleton scaffolding.
- **Lambdas are idiomatic and heavy**: almost every non-trivial function defines 1–4 local lambdas (`auto const dmg_ok = [gun](gun_damage_t dmg){...};`, `auto const try_aim = [this, local](...) -> bool {...};`) instead of private helper methods — lambdas are the preferred unit of local decomposition.
- **No RAII wrappers around OS/game resources** beyond what STL containers give for free; GL/native resource cleanup (in the second file, contrastingly, this *is* done — see below) is largely absent here since this file doesn't touch raw GPU handles directly.
- **Duplicated logic across the codebase**: two independent base64 implementations exist (a slow bitset-based one in `config/base64.cpp` that `throw`s a bare `const char*` on bad input, and a fast lookup-table one as an inline lambda inside `antisocial.mm`) — evidence of copy-paste-from-different-sources rather than a single shared utility, a genuine quirk of this codebase you'd reproduce if trying to match it exactly (i.e., don't over-consolidate; a little duplication/inconsistency is "in style" here).
- **Genuine, uncorrected bugs present in the style** (worth knowing, not necessarily worth imitating): `c_player_controller::in_air()` and `::fps_camera()` have no `return` on the fallthrough path when their outer `if` is false (UB); `alive()` has three redundant equivalent null checks in a row (`this == 0 || this == NULL || !this`). This kind of redundant defensive check and missing-return is a recurring authentic quirk, not a one-off.

### 2. `skinned_mesh_lod_group.cpp`

#### Naming conventions

- **Class**: `c_skinned_mesh_lod_group` — same `c_` prefix convention as `antisocial_leo`, in a `sonar::sdk` namespace (`using sonar::c_memory; using sonar::sdk::c_skinned_mesh_lod_group;` at top).
- **Members**: `m_` prefix, exhaustively — `m_valid`, `m_gl_uploaded`, `m_source_controller`, `m_bind_verts`, `m_bone_hashes`, `m_skin_buffers`, `m_slot_refs`, `m_pool_resolved`. Same convention as `antisocial_leo`.
- **Compile-time constants**: **`k_` prefix**, snake_case body — `k_native_component_go`, `k_native_go_name`, `k_interleaved_stride`, `k_max_pool_block`, `k_no_skin_slot`, `k_stream0_stride`. This is a Google-style-derived convention (`kConstantName` → `k_constant_name`) that does **not** appear in `antisocial_leo` (which uses bare SCREAMING_SNAKE macros or no prefix at all for constants). This is the clearest *naming* divergence between the two sources.
- **Types**: `_t` suffix for structs (`chams_mesh_handle_t`, `transform_matrix_t`, `transform_data_t`, `transform_access_t`, `skin_view_t`, `mesh_source_t`, `read_request_t`), `_e` suffix for enums (`lod_path_e`) — a cleaner, more consistently-applied Hungarian-ish suffix scheme than `antisocial_leo`'s mixed `_t`/no-suffix enum naming.
- **Functions**: snake_case, verb-first (`unregister_gl_mesh`, `register_gl_mesh`, `read_native_transform_name`, `find_native_transform_of_component`, `walk_transform_root`, `resolve_mesh_source`, `bind_bone_transform`, `cache_cpu`, `ensure_gl_upload`, `resolve_pool`, `acquire_skin`, `release_skin`).
- **File/class naming**: file name matches the class name exactly minus prefix, same one-symbol-per-file discipline as `antisocial_leo`.

#### Formatting

- **Same heavily-spaced Allman family style** as the `cheat/` core of `antisocial_leo`: `if ( !valid( ) )`, `mem->read< uintptr_t >( ... )`, `std::vector< std::byte >`, space-padded empty-arg calls `valid( )`, `get( )`. Braces on their own line for functions and control flow; cuddled `} else {` for else-chains (e.g. `set_native_visible`).
- **Indentation**: 4 spaces.
- **Pointer/reference placement**: left-attached, identical to `antisocial_leo` (`uintptr_t* modified`, `std::byte const* interleaved`).
- **`const`/qualifier placement is more "east const" / trailing-qualifier than `antisocial_leo`**: `uintptr_t const go = ...`, `std::byte const* interleaved`, `size_t const bp_count = ...` — consistently `Type const name` rather than `const Type name`. `antisocial_leo` mixes both (`const std::string& name`, `float const forward`) but leans slightly more toward leading `const` in signatures; this file is stricter/more consistent about trailing `const`.
- **Line length**: also uncapped, but on average shorter/denser than `antisocial_leo` — this file favors many small statements over long chained boolean expressions.
- **Designated initializers used freely** (C++20 aggregate init), not seen anywhere in `antisocial_leo`:
  ```cpp
  return {
      .id = arms_lod,
      .smr_managed = mem->read< uintptr_t >( arms_lod + 0x28 ),
  };
  ```
- **`__restrict`-qualified raw pointers** for a hot loop (`sonar::sdk::transform_matrix_t const* __restrict mats_ptr = ...`) — a low-level optimization idiom absent from `antisocial_leo`.

#### Comments

- **Essentially zero comments in the entire 1069-line file.** Not a single `//` or `/* */` explanatory comment was found in the whole file — pure self-documenting code via naming (`k_native_transform_father`, `resolve_pool`, `acquire_skin`) and structure. This is a stark, load-bearing difference from `antisocial_leo`'s casual/jokey commenting style — if asked to write in *this* file's style specifically, the instruction is: **do not comment**, let names and small functions carry meaning.
- No author-header comments, no TODOs, no commented-out dead code — the file reads as "finished/production," not "work in progress with notes to self" the way `antisocial_leo` does.

#### Code structure / logic patterns

- **File organization**: constants block → tiny static helper free-functions (`unregister_gl_mesh`, `register_gl_mesh`, `read_native_transform_name`, `find_native_transform_of_component`, `walk_transform_root`) → destructor → move constructor/assignment → public API methods in roughly lifecycle order (`matches_source` → `invalidate_cpu_cache` → `release_gl_resources` → `bind_bone_transform` → `cache_cpu` [the large ~320-line core routine] → `ensure_gl_upload` → `resolve_pool` → `skin` (two overloads) → `snapshot_skin`/`snapshot_into` → `acquire_skin`/`release_skin` → `set_native_visible`). This ordering — build up mirrors the object's actual runtime lifecycle — is more disciplined than `antisocial_leo`'s files, which tend to just list feature methods in whatever order they were written.
- **Error handling**: pure return-code/bool style with guard clauses, same philosophy as `antisocial_leo`'s gameplay code — but **zero exceptions anywhere**, not even at boundaries (no try/catch in the whole file; this is a lower-level, hotter-path module than `antisocial_leo`'s JSON/network glue).
- **Concurrency is a first-class concern here, and much more rigorous than in `antisocial_leo`**: explicit `std::atomic<bool>`/`std::atomic<uint8_t>` fields (`m_valid`, `m_gl_uploaded`, `m_published_skin`, `m_slot_refs[]`) with hand-chosen memory orders (`memory_order_acquire`/`_release`/`_seq_cst`) implementing a **lock-free triple-buffered "publish skin slot" pattern** (`acquire_skin`/`release_skin` with a retry loop and per-slot reference counts, `write_slot` selection avoiding the currently-published/currently-read slot). `antisocial_leo` has essentially no equivalent — its one `std::thread(...).detach()` usage is fire-and-forget, not synchronized with atomics.
- **Manual/native memory reading is the same architectural bedrock as `antisocial_leo`** (confirms both are the same *genre*): a `c_memory` singleton-like accessor (`c_memory::get()`) with `read<T>()`, `read_bytes()`, and a **batched scatter-read API** (`std::vector<c_memory::read_request_t> reqs; reqs.push_back({addr, {dst,len}}); mem->read_scatter(reqs);`) — a more sophisticated/efficient version of the same "reach into another process and pull fields by hex offset" idiom `antisocial_leo` does one field at a time via raw `reinterpret_cast`.
- **No IL2CPP method invocation here** — this file reads data structures directly (mesh/skeleton/bone hierarchy) rather than calling back into managed code, so the "`static auto fn = reinterpret_cast<...>(get_method_pointer(...))`" idiom from `antisocial_leo` does not appear; instead it's pure structured binary reads (`mem->read<uint32_t>(vd + 0x78)`, batched via `read_scatter`) and `std::memcpy`-based reinterpretation of raw byte buffers into typed structs (`std::memcpy(&p, raw_vbuf.data() + off, 12);`) — favoring `memcpy` over `reinterpret_cast<T*>` type punning for the actual vertex/bone payload, which is more strict-aliasing-safe than `antisocial_leo`'s freer use of `reinterpret_cast<T*>`.
- **Uses GLM** (`glm::vec3`, `glm::vec4`, `glm::mat4`, `glm::quat`, `glm::mat4_cast`, `glm::normalize`) for all vector/quaternion/matrix math, instead of hand-rolling a math library the way `antisocial_leo` does. Skeletal bone-chain composition (parent-relative position/rotation accumulation up to 64-deep chains, quaternion composition for world rotation) is implemented by hand with raw quaternion-rotate-vector formulas inlined (`mat.position.x = p.position.x + sx + sx*(...) + ...`) rather than via `glm`'s operator overloads — a deliberate hot-path optimization choice (avoids per-step temporary quaternion multiplies) that contrasts with `antisocial_leo`'s more "readable/operator-overload-heavy" numerical style.
- **Raw OpenGL ES resource management with explicit RAII-adjacent cleanup**: `register_gl_mesh`/`unregister_gl_mesh` free functions own the create/destroy symmetry for VAO/VBO/EBO handles, called from the destructor and from move-assignment (`unregister_gl_mesh(m_gl_mesh)` before overwriting) — more careful resource-lifetime discipline than anywhere in `antisocial_leo`.
- **Defensive numeric sanity-checking is much heavier here**: `std::isfinite()` checks on every position/rotation component before trusting memory-read data (`if ( !std::isfinite( m0.position.x ) || ... || std::abs( m0.position.x ) > 1.0e6f )`), distance-based outlier rejection between bones (`d.x*d.x + d.y*d.y + d.z*d.z > 25.f`), and a "distance from expected origin" sanity check (`best_dist_sq > 144.f`) before accepting a freshly-read skeleton pose as valid — this file assumes the remote memory can be transiently garbage/torn (mid-write) and defends against it numerically; `antisocial_leo` mostly just null/pointer-validity-checks (`c_memory::invalid(...)`) without float sanity checks.
- **`crc32_str` is a self-contained constexpr-invoked-at-runtime function that rebuilds its 256-entry lookup table on every call** rather than caching it in a `static` — a real (mild) inefficiency, worth reproducing faithfully if imitating this exact file's quirks rather than "fixing" it.
- **No obfuscation, no string encryption, no `crypt_string`/`ay_obfuscate` equivalents anywhere.** This file is not hardened against reverse-engineering the way `antisocial_leo` is — a genuine domain/hardening-posture difference, not just style.

### 3. Where the two sources agree vs. diverge (quick reference)

| Aspect | antisocial_leo | skinned_mesh_lod_group.cpp |
|---|---|---|
| Class prefix | `c_` | `c_` (same) |
| Member prefix | `m_` | `m_` (same) |
| Global prefix | `g_ctx` (ad hoc elsewhere) | none in this file (no globals) |
| Constant prefix | none consistent / SCREAMING_SNAKE macros | `k_` prefix, consistently |
| Enum suffix | mixed `_t` / none | `_e` (clean) |
| Struct suffix | `_t` | `_t` (same) |
| Brace style | Allman, cuddled `else` | Allman, cuddled `else` (same) |
| Interior paren/angle spacing | yes, pervasive | yes, pervasive (same) |
| `const` placement | mixed leading/trailing | consistently trailing (`Type const x`) |
| Comment density | low but present, casual/jokey/profane | essentially zero |
| Exceptions | only at JSON/network boundary | none at all |
| Concurrency | none (fire-and-forget thread) | lock-free atomics, explicit memory orders, triple-buffering |
| Math library | hand-rolled `vec3_t`/`quater_t` | GLM |
| Memory access | per-field `reinterpret_cast` reads, `this`-is-address SDK wrappers | batched scatter-reads + per-field reads, `memcpy`-based struct packing |
| IL2CPP method calls | yes (`static auto fn = reinterpret_cast<...>(get_method_pointer(...))`) | no (pure data reads) |
| String/IL2CPP-name obfuscation | yes (`crypt_string`, `ay_obfuscate`, Hikari attributes) | none |
| Resource cleanup discipline | minimal | explicit symmetric create/destroy (GL handles) |
| Numeric defensive checks | pointer/null validity only | `std::isfinite` + magnitude/outlier sanity checks throughout |
| Designated initializers | not used | used (`{ .id = ..., .smr_managed = ... }`) |

**Takeaway for matching style going forward**: both share the same surface-level "cheat scene" formatting DNA (`c_`/`m_` prefixes, Allman braces, padded parens, snake_case, raw-offset memory reads, singleton/`_t`-suffixed value types). If the target is "match `antisocial_leo`," lean into casual/profane comments, hand-rolled math, IL2CPP method-pointer caching, and heavy string obfuscation macros. If the target is "match `skinned_mesh_lod_group.cpp`," drop comments almost entirely, add a `k_`-prefixed constants block up top, prefer GLM over hand-rolled math, use trailing `const`, add `std::isfinite`/magnitude sanity checks around any remotely-read floating data, and use atomics with explicit memory orders for anything shared across render/logic threads.
