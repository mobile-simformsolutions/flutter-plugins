import 'dart:ui';

/// Represents a screen, containing information about its size, position, and
/// properties.
class Screen {
  /// Create a new screen.
  Screen(this.frame, this.visibleFrame, this.scaleFactor);

  /// The frame of the screen, in screen coordinates.
  final Rect frame;

  /// The portion of the screen's frame that is available for use by application
  /// windows. E.g., on macOS, this excludes the menu bar.
  final Rect visibleFrame;

  /// The number of pixels per screen coordinate for this screen.
  final double scaleFactor;
}
