import 'dart:async';

import 'package:flutter/services.dart';

import 'src/channels.dart';
import 'src/screen.dart';
import 'src/window_controller.dart';
import 'src/window_controller_impl.dart';

export 'src/window_controller.dart';

// Keys for screen and window maps returned by _getScreenListMethod.

/// The frame of a screen or window. The value is a list of four doubles:
///   [left, top, width, height]
const String _frameKey = 'frame';

/// The frame of a screen available for use by applications. The value format
/// is the same as _frameKey's.
///
/// Only used for screens.
const String _visibleFrameKey = 'visibleFrame';

/// The scale factor for a screen or window, as a double.
///
/// This is the number of pixels per screen coordinate, and thus the ratio
/// between sizes as seen by Flutter and sizes in native screen coordinates.
const String _scaleFactorKey = 'scaleFactor';

class DesktopMultiWindow {
  /// Create a new Window.
  ///
  /// The new window instance will call `main` method in your `main.dart` file in
  /// new flutter engine instance with some addiotonal arguments.
  /// the arguments of `main` method is a fixed length list.
  /// ---------------------------------------------------------
  /// | index |   Type   |        description                 |
  /// |-------|----------| -----------------------------------|
  /// | 0     | `String` | the value always is "multi_window".|
  /// | 1     | `int`    | the id of the window.              |
  /// | 2     | `String` | the [arguments] of the window.     |
  /// ---------------------------------------------------------
  ///
  /// You can use [WindowController] to control the window.
  ///
  /// NOTE: [createWindow] will only create a new window, you need to call
  /// [WindowController.show] to show the window.
  static Future<WindowController> createWindow([String? arguments]) async {
    final windowId = await multiWindowChannel.invokeMethod<int>(
      'createWindow',
      arguments,
    );
    assert(windowId != null, 'windowId is null');
    assert(windowId! > 0, 'id must be greater than 0');
    return WindowControllerMainImpl(windowId!);
  }

  static Future<void> quit() async =>
      multiWindowChannel.invokeMethod('destroy');

  /// Invoke method on the isolate of the window.
  ///
  /// Need use [setMethodHandler] in the target window isolate to handle the
  /// method.
  ///
  /// [targetWindowId] which window you want to invoke the method.
  static Future<dynamic> invokeMethod(int targetWindowId, String method,
      [dynamic arguments]) {
    return windowEventChannel.invokeMethod(method, <String, dynamic>{
      'targetWindowId': targetWindowId,
      'arguments': arguments,
    });
  }

  /// Add a method handler to the isolate of the window.
  ///
  /// NOTE: you can only handle this window event in this window engine isoalte.
  /// for example: you can not receive the method call which target window isn't
  /// main window in main window isolate.
  ///
  static void setMethodHandler(
      Future<dynamic> Function(MethodCall call, int fromWindowId)? handler) {
    if (handler == null) {
      windowEventChannel.setMethodCallHandler(null);
      return;
    }
    windowEventChannel.setMethodCallHandler((call) async {
      final fromWindowId = call.arguments['fromWindowId'] as int;
      final arguments = call.arguments['arguments'];
      final result =
      await handler(MethodCall(call.method, arguments), fromWindowId);
      return result;
    });
  }

  /// Get all sub window id.
  static Future<List<int>> getAllSubWindowIds() async {
    final result = await multiWindowChannel
        .invokeMethod<List<dynamic>>('getAllSubWindowIds');
    final ids = result?.cast<int>() ?? const [];
    assert(!ids.contains(0), 'ids must not contains main window id');
    assert(ids.every((id) => id > 0), 'id must be greater than 0');
    return ids;
  }

  /// Returns the list of physical screens attached to the system.
  static Future<List<Screen>> getAttachedScreenList() async {
    final screenList = <Screen>[];
    final response =
    (await multiWindowChannel.invokeMethod('getAttachedScreenList') as List? ??
        [])
        .cast<Map<dynamic, dynamic>>();

    for (final screenInfo in response) {
      screenList.add(_screenFromInfoMap(screenInfo));
    }
    return screenList;
  }

  /// Given a map of information about a screen, return the corresponding
  /// [Screen] object.
  ///
  /// Used for screen deserialization in the platform channel.
  static Screen _screenFromInfoMap(Map<dynamic, dynamic> map) {
    return Screen(
        _rectFromLTWHList((map[_frameKey] as List).cast<double>()),
        _rectFromLTWHList((map[_visibleFrameKey] as List).cast<double>()),
        double.tryParse(map[_scaleFactorKey].toString()) ?? 1);
  }

  /// Given an array of the form [left, top, width, height], return the
  /// corresponding [Rect].
  ///
  /// Used for frame deserialization in the platform channel.
  static Rect _rectFromLTWHList(List<double> ltwh) {
    return Rect.fromLTWH(ltwh[0], ltwh[1], ltwh[2], ltwh[3]);
  }
}
