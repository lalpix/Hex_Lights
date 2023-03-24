enum Topics {
  singleHexColor,
  primaryColor,
  secondaryColor,
  layout,
  fade,
  speed,
  mode,
  power,
}

enum Mode {
  Stationar, // all hex same color
  RotationOuter,
  StationarOuter,
  RandColorRandHex,
  RainbowSwipeVert,
  RainbowSwipeHorz,
  AudioBeatReact, //
  AudioFreqPool, //
  TopBottom,
  LeftRight,
  MeetInMidle,
  DirectionCircle,
}

String modeName(Mode m) {
  switch (m) {
    case Mode.Stationar:
      return 'Bez efektu';
    case Mode.RotationOuter:
      return 'Rotující okraje';
    case Mode.StationarOuter:
      return 'Pouze okraje';
    case Mode.RandColorRandHex:
      return 'Náhodné';
    case Mode.RainbowSwipeVert:
      return 'Duha vertikálně';
    case Mode.RainbowSwipeHorz:
      return 'Duha horizontálně';
    case Mode.AudioBeatReact:
      return 'Reakce na beat';
    case Mode.AudioFreqPool:
      return 'Reakce na frekvenci';
    case Mode.TopBottom:
      return 'Plnění nahoru a dolů';
    case Mode.LeftRight:
      return 'Plnění zleva a zprava';
    case Mode.MeetInMidle:
      return 'Souběh uprostřed';
    case Mode.DirectionCircle:
      return 'Plnění všesměrové';
  }
}
