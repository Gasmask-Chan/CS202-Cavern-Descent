# Enemy Animations Implementation Plan

Trọng tâm của tính năng này là hệ thống Animation (hoạt ảnh) cho kẻ thù (`Snake`, `Bat`, `Spider`), phụ thuộc vào `EnemyState` (hành động của chúng). Các sprite sẽ được lấy từ `assets/sprites/16x16`.

## Proposed Changes

### `src/entities/Entity.h` & `src/entities/Entity.cpp`
- **Thay đổi:** Biến `srcRect` đang ở trạng thái mặc định. Mình sẽ cho phép lật sprite (flip horizontally) khi `width` của `srcRect` mang giá trị âm, rất hữu dụng khi Entity quay trái/phải.

### `src/entities/Enemy.h`
#### [MODIFY] Enemy.h
Thêm các biến phục vụ vòng lặp hoạt ảnh:
```cpp
protected:
    float animTimer;
    float animSpeed;
    int currentFrame;
    int numFrames;
    int baseFrameX; // Bắt đầu ở ô số mấy trên spritesheet
    int baseFrameY; // Nằm ở hàng nào trên spritesheet
```
Thêm hàm `void setAnimation(int numFrames, float speed, int baseX, int baseY)` để dễ dàng thay đổi animation khi chuyển State.

### `src/entities/Enemy.cpp`
#### [MODIFY] Enemy.cpp
Cập nhật Logic Animation bên trong `Enemy::update(dt)`:
```cpp
animTimer += dt;
if (animTimer >= animSpeed) {
    animTimer -= animSpeed;
    currentFrame = (currentFrame + 1) % numFrames;
}

// Tính toán lại Rect của sprite
srcRect.x = (baseFrameX + currentFrame) * std::abs(srcRect.width);
srcRect.y = baseFrameY * std::abs(srcRect.height);

// Lật hình ảnh tùy vào hướng di chuyển (isFacingRight)
if (isFacingRight) {
    srcRect.width = -std::abs(srcRect.width);
} else {
    srcRect.width = std::abs(srcRect.width);
}
```

### Triển khai từng Enemy
#### 1. `Snake`
- Nguồn: `gfx_bat_snake_jetpack.png` (Hàng số 1 - Tọa độ Y: 16)
- **IDLE/PATROL State**: Rắn liên tục bò trườn.
- Animation: Chạy lặp 4 frame đầu tiên (BaseX: 0, BaseY: 1, NumFrames: 4).

#### 2. `Bat`
- Nguồn: `gfx_bat_snake_jetpack.png` (Hàng số 0 - Tọa độ Y: 0)
- **IDLE State**: Treo ngược trần nhà (BaseX: 0, NumFrames: 1).
- **CHASE State**: Bay về phía người chơi (BaseX: 1, NumFrames: 4, lặp lại nhịp nhàng).

#### 3. `Spider`
- Nguồn: `gfx_spider_skeleton.png`
- **IDLE State**: Treo trên trần (BaseX: 0, NumFrames: 1).
- **CHASE/JUMP State**: Chạy các frame di chuyển trên mặt đất và frame dang chân khi nhảy mổ.

## User Review Required
> [!WARNING]
> **Hướng hiển thị mặc định của Sprite (Facing Direction)**
> Thông thường spritesheet sẽ vẽ nhân vật quay mặt sang trái. Đoạn code lật ảnh `srcRect.width = isFacingRight ? -16.0f : 16.0f` được dựa trên giả định này.

Bạn duyệt xem hướng triển khai Animation như thế này đã ổn chưa để mình tiến hành đập đi xây lại (refactor) phần render các con Enemy này nhé!
