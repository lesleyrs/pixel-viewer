#include "raylib.h"

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

void SetTitle(FilePathList, Texture2D, float);
void ScaleToFit(Texture2D);

int main(int argc, char *argv[]) {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);
  InitWindow(screenWidth, screenHeight, title);
  // LoadDirectoryFiles uses less memory + combine filetypes?
  // Can't view .ico or .gif files but others should work
  FilePathList files = LoadDirectoryFilesEx(".", ".png", false);
  if (argc > 1) {
    for (int i = 0; i < files.count; i++) {
      // "open with" opens incorrect directory instead of clicked file
      // windows photo viewer doesn't have this problem
      if (TextIsEqual(GetFileName(argv[1]), GetFileName(files.paths[i]))) {
        image = i;
        break;
      }
    }
  }
  Texture2D texture = LoadTexture(files.paths[image]);
  ScaleToFit(texture);
  SetTitle(files, texture, scale);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // fullscreen gives artifacts so need to use maximized
    // could also add resize check for maximized/windowed manually
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_F11) ||
        (IsKeyPressed(KEY_ENTER) &&
         (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))) {
      if (IsWindowMaximized()) {
        ClearWindowState(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_UNDECORATED);
        ScaleToFit(texture);
        SetTitle(files, texture, scale);
      } else {
        SetWindowState(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_UNDECORATED);
        ScaleToFit(texture);
        SetTitle(files, texture, scale);
      }
    }

    if (files.count == 0)
      DrawText(TextFormat("%s", empty),
               GetRenderWidth() / 2 - MeasureText(empty, fontSize) / 2,
               GetRenderHeight() / 2 - fontSize / 2, fontSize, RAYWHITE);

    int key = GetKeyPressed();
    if (key >= KEY_ZERO && key <= KEY_NINE) {
      gapLength = (key - KEY_ZERO) * 10;
      ScaleToFit(texture);
      SetTitle(files, texture, scale);
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
        texture = LoadTexture(files.paths[image]);
        ScaleToFit(texture);
        SetTitle(files, texture, scale);
      }
    }
    if (((IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT) &&
          (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))) ||
         (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT) &&
          GetMouseX() >= GetRenderWidth() / 2)) &&
        image < files.count - 1 && files.count != 0) {
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
        texture = LoadTexture(files.paths[image]);
        ScaleToFit(texture);
        SetTitle(files, texture, scale);
      }
    }

    // add grid + zoom towards the image instead (maybe switch to raylib camera)
    if (IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)) {
      scale += minScale + scale / 100;
      if (scale > maxScale)
        scale = maxScale;
      SetTitle(files, texture, scale);
    }
    if (IsKeyDown(KEY_DOWN) && !IsKeyDown(KEY_UP)) {
      scale -= minScale + scale / 100;
      if (scale < minScale)
        scale = minScale;
      SetTitle(files, texture, scale);
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
      SetTitle(files, texture, scale);
    }

    // can't be sure if non-square image fits so we scale it
    if (IsKeyPressed(KEY_Z)) {
      rotation -= 90;
      if (rotation < 0)
        rotation += 360;
      ScaleToFit(texture);
      SetTitle(files, texture, scale);
    }
    if (IsKeyPressed(KEY_X)) {
      rotation += 90;
      if (rotation >= 360)
        rotation -= 360;
      ScaleToFit(texture);
      SetTitle(files, texture, scale);
    }
    if (IsKeyPressed(KEY_C)) {
      rotation += 180;
      if (rotation >= 360)
        rotation -= 360;
      ScaleToFit(texture);
      SetTitle(files, texture, scale);
    }

    if (IsKeyPressed(KEY_R)) {
      offset = (Vector2){0, 0};
      scale = 100;
      rotation = 0;
      SetTitle(files, texture, scale);
    }
    if (IsKeyPressed(KEY_F)) {
      ScaleToFit(texture);
      SetTitle(files, texture, scale);
    }

    // swap directions?
    if (IsKeyDown(KEY_W))
      offset.y = offset.y - scrollSpeed / scale - 1;
    if (IsKeyDown(KEY_A))
      offset.x = offset.x - scrollSpeed / scale - 1;
    if (IsKeyDown(KEY_S))
      offset.y = offset.y + scrollSpeed / scale + 1;
    if (IsKeyDown(KEY_D))
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
  UnloadDirectoryFiles(files);
  UnloadTexture(texture);
  CloseWindow();
  return 0;
}

// is this correct?
void ScaleToFit(Texture2D texture) {
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

void SetTitle(FilePathList files, Texture2D texture, float scale) {
  if (texture.id > 0) {
    SetWindowTitle(TextFormat("[%i/%i] [%dx%d] %s %.0f%% %i° %u%% - %s",
                              image + 1, files.count, texture.width,
                              texture.height, GetFileName(files.paths[image]),
                              scale, (int)rotation, 100 - gapLength, title));
  } else {
    SetWindowTitle(TextFormat("[%i/%i] - %s", image, files.count, title));
  }
}
