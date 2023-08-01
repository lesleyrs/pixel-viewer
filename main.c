#include "raylib.h"
#include <stdlib.h>
#include <string.h>

static const char *title = "Pixel Viewer";
static const char *empty = "No images found in this folder";

Vector2 offset = {0, 0};
float scale = 100;
float rotation = 0;
unsigned int image = 0;
int screenWidth = 1280;
int screenHeight = 720;
int fontSize = 64;
unsigned short scrollSpeed = 300; // random number, only affects keys
unsigned short maxScale = 9000;   // 90.00 * 8 = default screenHeight
unsigned char minScale = 10;
unsigned char timer = 0;
unsigned char delay = 30;
unsigned char pressDelay = 30;
unsigned char heldDelay = 10;
unsigned char gapLength = 10;
bool heldDown = false;
bool fullscreen = false;

FilePathList filteredList;
Texture2D texture;

void SetTitle();
void ScaleToFit();
void UpdateWindow();

int main(int argc, char *argv[]) {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  InitWindow(screenWidth, screenHeight, title);
  FilePathList files = LoadDirectoryFiles(".");
  filteredList.capacity = files.count;
  filteredList.count = 0;
  filteredList.paths = (char **)malloc(filteredList.capacity * sizeof(char *));
  // default raylib supported file types
  // TODO: need to add gif too
  for (int i = 0; i < files.count; i++) {
    if (IsFileExtension(files.paths[i], ".png") ||
        IsFileExtension(files.paths[i], ".qoi") ||
        IsFileExtension(files.paths[i], ".dds") ||
        IsFileExtension(files.paths[i], ".hdr")) {
      filteredList.paths[filteredList.count] = _strdup(files.paths[i]);
      filteredList.count++;
    }
  }
  UnloadDirectoryFiles(files);
  if (argc > 1) {
    for (int i = 0; i < filteredList.count; i++) {
      // TODO: "open with" opens incorrect directory instead of clicked file?
      // windows photo viewer doesn't have this problem
      if (TextIsEqual(GetFileName(argv[1]),
                      GetFileName(filteredList.paths[i]))) {
        texture = LoadTexture(filteredList.paths[image = i]);
        break;
      }
    }
  } else if (filteredList.count) {
    texture = LoadTexture(filteredList.paths[0]);
  }
  ScaleToFit();
  SetTitle();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    UpdateWindow();

    if (filteredList.count == 0)
      DrawText(TextFormat("%s", empty),
               GetRenderWidth() / 2 - MeasureText(empty, fontSize) / 2,
               GetRenderHeight() / 2 - fontSize / 2, fontSize, RAYWHITE);

    int key = GetKeyPressed();
    if (key >= KEY_ZERO && key <= KEY_NINE) {
      gapLength = (key - KEY_ZERO) * 10;
      ScaleToFit();
      SetTitle();
    }

    if ((IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_RIGHT) ||
         IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) &&
        !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT) &&
        !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      delay = pressDelay;
      timer = 0;
      heldDown = false;
    }

    if (((IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT) &&
          (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))) ||
         (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_RIGHT) &&
          GetMouseX() < GetRenderWidth() / 2)) &&
        image > 0) {
      if (timer < delay)
        timer += 1;
      if (!heldDown || timer >= delay) {
        if (timer >= delay) {
          delay = heldDelay;
          timer = 0;
        }
        if (!heldDown)
          heldDown = true;
        image -= 1;
        UnloadTexture(texture);
        texture = LoadTexture(filteredList.paths[image]);
        ScaleToFit();
        SetTitle();
      }
    }
    if (((IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT) &&
          (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))) ||
         (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT) &&
          GetMouseX() >= GetRenderWidth() / 2)) &&
        image < filteredList.count - 1 && filteredList.count != 0) {
      if (timer < delay)
        timer += 1;
      if (!heldDown || timer >= delay) {
        if (timer >= delay) {
          delay = heldDelay;
          timer = 0;
        }
        if (!heldDown)
          heldDown = true;
        image += 1;
        UnloadTexture(texture);
        texture = LoadTexture(filteredList.paths[image]);
        ScaleToFit();
        SetTitle();
      }
    }

    // TODO: add grid + zoom towards the image (maybe switch to raylib camera)
    if (IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)) {
      scale += minScale + scale / 100;
      if (scale > maxScale)
        scale = maxScale;
      SetTitle();
    }
    if (IsKeyDown(KEY_DOWN) && !IsKeyDown(KEY_UP)) {
      scale -= minScale + scale / 100;
      if (scale < minScale)
        scale = minScale;
      SetTitle();
    }

    if (GetMouseWheelMove()) {
      if (scale == minScale)
        scale += (GetMouseWheelMove() * 100 - minScale);
      else
        scale += (GetMouseWheelMove() * 100);
      if (scale > maxScale)
        scale = maxScale;
      if (scale < minScale)
        scale = minScale;
      SetTitle();
    }

    // can't be sure if non-square image fits so we scale it
    if (IsKeyPressed(KEY_Z)) {
      rotation -= 90;
      if (rotation < 0)
        rotation += 360;
      ScaleToFit();
      SetTitle();
    }
    if (IsKeyPressed(KEY_X)) {
      rotation += 90;
      if (rotation >= 360)
        rotation -= 360;
      ScaleToFit();
      SetTitle();
    }
    if (IsKeyPressed(KEY_C)) {
      rotation += 180;
      if (rotation >= 360)
        rotation -= 360;
      ScaleToFit();
      SetTitle();
    }

    if (IsKeyPressed(KEY_R)) {
      offset = (Vector2){0, 0};
      scale = 100;
      rotation = 0;
      SetTitle();
    }
    if (IsKeyPressed(KEY_F)) {
      ScaleToFit();
      SetTitle();
    }

    if (IsKeyDown(KEY_S))
      offset.y = offset.y - scrollSpeed / scale - 1;
    if (IsKeyDown(KEY_D))
      offset.x = offset.x - scrollSpeed / scale - 1;
    if (IsKeyDown(KEY_W))
      offset.y = offset.y + scrollSpeed / scale + 1;
    if (IsKeyDown(KEY_A))
      offset.x = offset.x + scrollSpeed / scale + 1;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
      offset = (Vector2){offset.x + GetMouseDelta().x / scale * 100 / 2,
                         offset.y + GetMouseDelta().y / scale * 100 / 2};

    DrawTexturePro(
        texture,
        (Rectangle){0.0f, 0.0f, (float)texture.width, (float)texture.height},
        (Rectangle){
            GetRenderWidth() / 2. - texture.width + offset.x * scale / 100 +
                texture.width + offset.x * scale / 100,
            GetRenderHeight() / 2. - texture.height + offset.y * scale / 100 +
                texture.height + offset.y * scale / 100,
            (float)texture.width * scale / 100,
            (float)texture.height * scale / 100},
        (Vector2){(float)texture.width * scale / 100 / 2,
                  (float)texture.height * scale / 100 / 2},
        rotation, WHITE);
    EndDrawing();
  }
  RL_FREE(filteredList.paths);
  UnloadTexture(texture);
  CloseWindow();
  return 0;
}

// TODO: is this correct?
void ScaleToFit() {
  offset = (Vector2){0, 0};
  if ((rotation == 90 || rotation == 270) &&
      texture.height > texture.width * GetRenderWidth() / GetRenderHeight())
    scale = (float)GetRenderWidth() / texture.height * 100;
  else if ((rotation == 90 || rotation == 270) &&
           texture.height > texture.width)
    scale = (float)GetRenderHeight() / texture.width * 100;
  else if ((rotation == 90 || rotation == 270) &&
           texture.width > texture.height)
    scale = (float)GetRenderHeight() / texture.width * 100;
  else if (texture.width >
           texture.height * GetRenderWidth() / GetRenderHeight())
    scale = (float)GetRenderWidth() / texture.width * 100;
  else
    scale = (float)GetRenderHeight() / texture.height * 100;

  scale -= gapLength * (scale / 100);
}

void SetTitle() {
  if (texture.id > 0) {
    SetWindowTitle(TextFormat("[%i/%i] [%dx%d] %s %.0f%% %i° %u%% - %s",
                              image + 1, filteredList.count, texture.width,
                              texture.height,
                              GetFileName(filteredList.paths[image]), scale,
                              (int)rotation, 100 - gapLength, title));
  } else {
    SetWindowTitle(
        TextFormat("[%i/%i] - %s", image, filteredList.count, title));
  }
}

void UpdateWindow() {
  if (IsWindowResized()) {
    ScaleToFit();
    SetTitle();
  }

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_F11) ||
      (IsKeyPressed(KEY_ENTER) &&
       (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))) {
    if (!fullscreen) {
      fullscreen = true;
      SetWindowState(FLAG_WINDOW_UNDECORATED);
      SetWindowPosition(0, 0);
      int display = GetCurrentMonitor();
      /* 1 pixel difference stops exclusive fullscreen but:
        +1 pixel pop-ups might not show?
        -1 pixel causes task bar to show up early on alt tab */
      SetWindowSize(GetMonitorWidth(display) + 1, GetMonitorHeight(display));
      ScaleToFit();
      SetTitle();
    } else {
      fullscreen = false;
      if (IsWindowMaximized())
        ClearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST |
                         FLAG_WINDOW_MAXIMIZED);
      else
        ClearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);
      int display = GetCurrentMonitor();
      SetWindowPosition(GetMonitorWidth(display) / 2 - screenWidth / 2,
                        GetMonitorHeight(display) / 2 - screenHeight / 2);
      SetWindowSize(screenWidth, screenHeight);
      ScaleToFit();
      SetTitle();
    }
  }

  if (!IsWindowFocused() && fullscreen)
    ClearWindowState(FLAG_WINDOW_TOPMOST);
  else if (IsWindowFocused() && fullscreen)
    SetWindowState(FLAG_WINDOW_TOPMOST);
}
