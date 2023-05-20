enum Topics {
  singleHexColor,
  primaryColor,
  secondaryColor,
  layout,
  fade,
  brightness,
  speed,
  mode,
  power,
  rainbow,
}

enum Mode {
  Stationar, // all hex same color
  RotationOuter,
  StationarOuter,
  RandColorRandHex,
  RandColorRandHexFade,
  RainbowSwipeVert,
  RainbowSwipeHorz,
  AudioBeatReact, //
  AudioFreqPool, //
  TwoColorFading,
  TopBottom,
  LeftRight,
  MeetInMidle,
  DirectionCircle,
}

String modeName(Mode m) {
  switch (m) {
    case Mode.Stationar:
      return 'No efect';
    case Mode.RotationOuter:
      return 'Rotating border';
    case Mode.StationarOuter:
      return 'Border only';
    case Mode.RandColorRandHex:
      return 'Random';
    case Mode.RandColorRandHexFade:
      return 'Random fading';
    case Mode.RainbowSwipeVert:
      return 'Rainbow vertical';
    case Mode.RainbowSwipeHorz:
      return 'Rainbow horizontal';
    case Mode.AudioBeatReact:
      return 'Beat reaction';
    case Mode.AudioFreqPool:
      return 'Frequency vizualizer';
    case Mode.TwoColorFading:
      return 'Two color fade';
    case Mode.TopBottom:
      return 'Top down fill';
    case Mode.LeftRight:
      return 'Left right fill';
    case Mode.MeetInMidle:
      return 'Meet in middle fill';
    case Mode.DirectionCircle:
      return 'All direction fill';
  }
}
