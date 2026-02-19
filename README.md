<<<<<<< HEAD
# Runtime Transform Gizmo System (Unreal Engine 5)

## 📌 Overview
This project implements a **runtime transform gizmo system** in Unreal Engine using C++.  
It allows users to select and move actors during gameplay, similar to the Unreal Editor transform tool.

The system supports:
- Single and multi-actor selection
- Axis and plane constrained movement
- Group pivot calculation
- Visual highlighting
- Multiplayer (server-authoritative) movement

---

## 🚀 Features

### 🎯 Actor Selection
- Mouse-based selection using `GetHitResultUnderCursor`
- Ctrl + Click for multi-selection
- Toggle selection support
- Dynamic material highlighting using `UMaterialInstanceDynamic`

### 🔧 Runtime Gizmo
- Custom transform gizmo spawned at runtime
- Axis movement (X, Y, Z)
- Plane movement (XY, YZ, XZ)
- Component tags for axis/plane detection

### 🧮 Constrained Drag System
- Mouse deprojection to world space
- Drag plane cached on mouse press
- Line-plane intersection for stable movement
- Vector projection for axis locking
- Maintains relative offsets for multi-actor movement

### 🌐 Multiplayer Support
- Server-authoritative actor movement
- RPC-based transform synchronization
- Reduced jitter using movement tolerance checks

---

## 🎮 Controls

| Action | Input |
|--------|--------|
| Select Actor | Left Click |
| Multi-Select | Ctrl + Click |
| Move Actors | Click + Drag Gizmo |
| Axis Movement | Drag X / Y / Z |
| Plane Movement | Drag XY / YZ / XZ |

---

## 🛠 Implementation Notes
- Built inside a custom `PlayerController`
- Drag plane and initial hit location cached on mouse press
- Movement logic handled during Tick only when dragging
- Group pivot calculated by averaging selected actor locations
- Actor offsets stored to preserve spacing during multi-move

---


=======
# Gizmo
>>>>>>> 6ee4e75daec11993fa0c457ac8241aed1870ee1e
