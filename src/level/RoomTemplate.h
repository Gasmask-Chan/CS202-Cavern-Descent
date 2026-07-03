#pragma once

namespace Platformer {

// Room roles assigned by the macro-grid path algorithm.
// Template geometry is resolved in LevelGenerator via hardcoded int arrays.
enum class RoomRole {
    UNKNOWN,
    TYPE_0,             // Side room (closed, unused)
    TYPE_1,             // Left/Right pass
    TYPE_2,             // Left/Right/Bottom pass (Drop room)
    TYPE_3,             // Left/Right/Top pass (Landing room)
    TYPE_2_DROP_THROUGH,// Left/Right/Top/Bottom pass
    TYPE_SHOP,          // Closed room repurposed as a shop
    TYPE_ALTAR,         // Closed room repurposed as a kali altar
};

} // namespace Platformer
