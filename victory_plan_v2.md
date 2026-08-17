# Victory Cutscene Implementation Plan (Version 2)

Dựa trên yêu cầu mới nhất của bạn, kịch bản Ending đã được thiết kế lại hoàn toàn thành 5 Scene chính, nối tiếp nhau và kết thúc bằng màn hình nhập tên.

## 1. Cấu trúc dữ liệu và Trạng thái (Data Structures)

Sử dụng `VictoryState` với State Machine nội bộ:

```cpp
enum class EndingScene {
    SCENE1_TUNNEL,
    SCENE2_DESERT_FALL,
    SCENE3_TALLY,
    SCENE4_BLACK_SCREEN,
    SCENE5_SUMMARY // Màn hình nhập tên
};

struct CutsceneActor {
    Vector2 position;
    Vector2 velocity;
    float animTimer = 0.0f;
    int animFrame = 0;
    bool isFacingRight = true;
};
```

**Assets cần load:**
- Textures: `sEnd2BG.png`, `sBGEnd3.png`, `sDesert.png`, `sDesert2.png`, `sDesertTop.png`, `sPalmTree.png`, `sShrub.png`, `sBigTreasure.png`.

## 2. Chi tiết Cập nhật & Hiển thị (Update & Render Logic)

### SCENE 1: TUNNEL (Hầm đền thờ)
- **Hiển thị (Render):** 
  - Vẽ TileMap sử dụng bộ gạch `temple`.
  - Cấu trúc hầm: Nền hầm cách đáy màn hình 2 ô. Hàng dưới cùng là 1 hàng trống, và dưới cùng nữa là Lava (`LiquidSimulator`).
  - Nhân vật vẽ ở giữa hầm.
- **Logic (Update):**
  - Nhân vật tự động chạy sang phải (`velocity.x > 0`).
  - Biến đếm `stepCount` đếm số bước (hoặc có thể dùng tọa độ X để canh 15 bước).
  - Khi đủ 15 bước -> Đột ngột chuyển sang `SCENE2_DESERT_FALL`.

### SCENE 2: DESERT FALL (Rơi xuống sa mạc)
- **Hiển thị (Render):**
  - Bầu trời: `sEnd2BG.png` (Cắt phần núi lửa ở trên bằng `srcRect`).
  - Ở giữa (Background): Núi `sBGEnd3.png`.
  - Cát đáy (Foreground): Khối gạch vẽ bằng cách random `sDesert` và `sDesert2`. Mặt trên cùng phủ `sDesertTop`.
  - Hai bên (Trang trí): Vẽ `sPalmTree` và `sShrub` chốt ở 2 góc trái/phải.
  - Vẽ `Player` (rơi từ trên không xuống).
- **Logic (Update):**
  - `Player` rớt từ `y = -100` xuống mặt cát (áp dụng trọng lực).
  - Khi chạm mặt cát: `Player` chuyển sang frame **Bị Choáng** (Stunned) và `timer` bắt đầu đếm 2 giây.
  - Sau 2 giây: `Player` đứng dậy (chuyển sang frame đứng yên bình thường). Tiếp tục rơi `BigTreasure`.
  - Khối `sBigTreasure` rớt từ đỉnh màn hình xuống. Khi `BigTreasure` chạm đất, `Player` nảy lên một cái (`velocity.y = -400`) rồi rớt lại xuống đất. 
  - Sau khi rớt lại xuống đất, chuyển sang `SCENE3_TALLY`.

### SCENE 3: TALLY (Bảng điểm)
- **Hiển thị (Render):** 
  - Giữ nguyên background và nhân vật của Scene 2.
  - Lần lượt vẽ các UI Text hiển thị ở giữa màn hình:
    - "YOU MADE IT!"
    - "FINAL SCORE:"
    - Điểm đếm dần (Lerp tới số điểm thực).
    - "TIME: ..."
    - "KILLS: ..."
  - Vẽ một hình chữ nhật màu đen có độ mờ (`fadeAlpha`) tăng dần từ `0.0` lên `1.0`.
- **Logic (Update):**
  - Quản lý thứ tự hiện text bằng timer. 
  - Khi text hiển thị xong, `fadeAlpha` bắt đầu tăng dần.
  - Khi `fadeAlpha >= 1.0` (màn hình đen hoàn toàn), chuyển sang `SCENE4_BLACK_SCREEN`.

### SCENE 4: BLACK SCREEN
- **Hiển thị (Render):** Màn hình đen hoàn toàn. Chữ trắng "YOU SHALL BE REMEMBERED AS A HERO".
- **Logic (Update):**
  - Giữ màn hình này trong đúng 5 giây.
  - Hết 5 giây -> Chuyển sang `SCENE5_SUMMARY`.

### SCENE 5: SUMMARY (Tổng kết Victory)
- Đây là màn hình hiện tại đang có (có nhập Initials 3 chữ cái để lưu High Score).
- Mọi logic hiển thị và lưu điểm hiện tại của `VictoryState` sẽ được kích hoạt ở pha này.

## Phê duyệt từ người chơi (User Review Required)

- **Cắt hình sEnd2BG**: Ảnh bầu trời `sEnd2BG.png` có độ phân giải bao nhiêu và phần núi lửa chiếm bao nhiêu pixel để mình cắt cho chuẩn? (Nếu không rõ, mình sẽ cắt 1/3 phía trên).
- **Màn hình TUNNEL**: Để đơn giản và tối ưu, thay vì load nguyên 1 `TileMap`, mình sẽ vẽ cứng (hard-code) một vòng lặp vẽ các block `temple` dọc theo màn hình để làm cái hầm. Lava thì vẽ mô phỏng gợn sóng ở dưới, như thế có ok không?

Nếu bạn đồng ý với kế hoạch mới này, mình sẽ bắt đầu tiến hành viết code cho toàn bộ khối logic trên!
