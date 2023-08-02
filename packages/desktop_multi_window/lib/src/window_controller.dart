import 'dart:ui';

import 'window_controller_impl.dart';

/// The [WindowController] instance that is used to control this window.
abstract class WindowController {
  WindowController();

  factory WindowController.fromWindowId(int id) {
    return WindowControllerMainImpl(id);
  }

  factory WindowController.main() {
    return WindowControllerMainImpl(0);
  }

  /// The id of the window.
  /// 0 means the main window.
  int get windowId;

  /// Try to close the window.
  Future<void> close();

  /// Show the window.
  Future<void> show();

  /// Hide the window.
  Future<void> hide();

  /// Set the window frame rect.
  Future<void> setFrame(Rect frame);

  /// Center the window on the screen.
  Future<void> center();

  /// Set the window's title.
  Future<void> setTitle(String title);

  /// Available only on macOS.
  Future<void> setFrameAutosaveName(String name);

  /// Necessary to be called before using any screen related functions.
  Future<void> ensureScreenInitialized();

  /// Wait until ready to show.
  Future<void> waitUntilReadyToShow();

  /// Sets whether the window should be in fullscreen mode.
  Future<void> setFullScreen(bool isFullScreen);

  /// Sets whether the window should show always on top of other windows.
  Future<void> setAlwaysOnTop(bool isAlwaysOnTop);

  /// Makes the window not show in the taskbar / dock.
  Future<void> setSkipTaskbar(bool isSkipTaskbar);

  /// Sets whether the window can be manually resized by the user.
  Future<void> setResizable(bool isResizable);

  /// Sets whether the window can be manually closed by user.
  Future<void> setClosable(bool isClosable);
}
